/* --------------------------------------------
 * 创建日期: 2013-06-4
 * 程序作者: hd
 * 版本信息: 1.0.0.0
 * 程序功能: IC卡基本信息查询
 * --------------------------------------------*/

#define _IN_SQC_
#include <stdio.h>
#include <string.h>
#include <iomanip>
#include "pubdef.h"
#include "errdef.h"
#include "pubfunc.h"
#include "pubdb.h"
#include "dbfunc.h"
#include "transcode.h"
#include "dbfunc_foo.h"
#include "acctrans.h"
#include "unitfunc.h"
#include "ecbankfunc.h"
#include "cpack.h"
#include "bupub.h"
#include "bufunc.h"
#include "ecbankytc.h"

static int pack_body(TRANS& trans,std::string& body)
{
    int ret = 0;

	gReader.fetchRow();

	//生成交易流水号并记录
    ret = GetNewRefno(trans.refno);
    if (ret)
    {
        return ret;
    }

	   TiXmlDocument doc;
    {
        TiXmlElement root("ROOT");
        doc.InsertEndChild(root);
    }
    char dt[15] = {0};
    getsysdatetime(dt);
    string now(dt);

	TiXmlNode& root = *(doc.FirstChild());

 	//版本号
	ec_add_xml_node_value(root,"VERSION", "010101" );

	//交易代码
	ec_add_xml_node_value(root,"TRANS_TYPE", "120521" );

	//动作代码
	ec_add_xml_node_value(root,"ACTION_TYPE", "1");

	/*//响应代码
	ec_add_xml_node_value(root,"RESP_CODE", );*/

	//登录身份类型
	ec_add_xml_node_value(root,"LOGIN_TYPE", "1");

	//交易渠道
	ec_add_xml_node_value(root,"TRANS_SOURCE", "BP");

	//登录机构代码
	ec_add_xml_node_value(root,"LOGIN_INST_CODE", "48080000");

	//交易日期
	{
		string str;
		str.assign(trans.accdate,8);
		ec_add_xml_node_value(root,"TXN_DATE", str);
	}

	//交易时间
	{
		string str;
		str.assign(trans.acctime,6);
		ec_add_xml_node_value(root,"TXN_TIME", str);
	}

	//跟踪号/流水号
	{
		string str;
		str.assign(trans.refno,14);
		//str[6]='0';
		//str[7]='0';
		str=str.substr(6);
		//str.append(1,'1');
		ec_add_xml_node_value(root,"TRACE_NO", str);
	}

	//登录网点代码
	{
		string str;

		ret = get_ec_para(YTCEC_LOGIN_MERCH, str);
		if (ret)
		{
			LOG(ERROR, "中银通登录网点代码未配置");
			return E_COMMON_ERR;
		}
		ec_add_xml_node_value(root,"LOGIN_MERCH_CODE", str);
	}
	//ec_add_xml_node_value(root,"LOGIN_MERCH_CODE", "J001");

	//登录操作员代码
	{
		string str;

		ret = get_ec_para(YTCEC_LOGIN_USER, str);
		if (ret)
		{
			LOG(ERROR, "中银通登录操作员代码未配置");
			return E_COMMON_ERR;
		}
		ec_add_xml_node_value(root,"LOGIN_USER_CODE", str);
	}
	//ec_add_xml_node_value(root,"LOGIN_USER_CODE", "A5");

	string  bankcardno;   //卡号

	//卡号
	{
		stringstream ss;
		COL2VAR(bankcardno);
		ss << bankcardno;
		ec_add_xml_node_value(root,"CARD_NO",ss.str());
	}

	//密码检查选项
	ec_add_xml_node_value(root,"CHK_OPTION", "0");

	//密码
	ec_add_xml_node_value(root,"PASSWORD", "12345678");

	//报文鉴别码
	//ec_add_xml_node_value(root, "MESG_AUTHN_CODE", "87AE");
	 ec_bank_any_ytc::calc_trans_mac(&root);

	TiXmlPrinter printer;
    doc.Accept(&printer);

	char xml_header[100]="<?xml version='1.0' encoding='GBK'?>\n";
	body.append(xml_header);
    body += printer.CStr();
	return 0;
}

static int send_to_bank(const std::string& req, std::string& resp)
{
    int timeout = 0;
    string ytc_ip;
    int ytc_port;
    int ret;

	ret = get_ec_para(YTCEC_TIMEOUT, timeout);
	if (ret)
    {
        LOG(ERROR, "中银通超时时限未配置");
        return E_COMMON_ERR;
    }

	timeout*=1000;

    ret = get_ec_para(YTCEC_SVRIP, ytc_ip);
    if (ret)
    {
        LOG(ERROR, "中银通前置机未配置");
        return E_COMMON_ERR;
    }
    ret = get_ec_para(YTCEC_SVRPORT, ytc_port);
    if (ret)
    {
        LOG(ERROR, "中银通前置机未配置");
        return E_COMMON_ERR;
    }
    LOG(DEBUG, "中银通服务器[" << ytc_ip << ":" << ytc_port << "]");

    CTcpSocket sock;
    alarm(timeout / 1000);
    if (!sock.ConnectTcp((char*)ytc_ip.c_str(), ytc_port))
    {
        alarm(0);
        LOG(ERROR, "connect to ytc error");
        return E_COMMON_ERR;
    }
    alarm(0);

    // 中银通报文
    stringstream ss;
    ss << setw(4) << setfill('0') <<  req.length() << req;

    LOG(DEBUG, "中银通报文[" << ss.str() << "]");
    if (sock.Send((char*)ss.str().c_str(), ss.str().length()) < (int)ss.str().length())
    {
        LOG(ERROR, "Send to ytc error");
        return E_COMMON_ERR;
    }
    // 包头
    size_t body_len = 0;
    {
        char buffer[5] = {0};
        if (sock.Recv(buffer, 4, timeout) <= 0)
        {
            LOG(ERROR, "Recv body head from ykc error");
            return E_COMMON_ERR;
        }
        istringstream is(buffer);
        is >> body_len;
    }
    // 包体
    string body;
    {
        char* buffer = new char[body_len + 1];
        memset(buffer, 0, body_len + 1);
        if (sock.Recv(buffer, body_len, timeout) <= 0)
        {
            LOG(ERROR, "Recv body from ykc error");
            delete [] buffer;
            return E_COMMON_ERR;
        }
        body = buffer;
        delete [] buffer;

        resp = body;
    }
    return 0;
}


FUNCTION(3403)
{
    int ret = 0;
    CAccTrans& ats = CAccTrans::GetInst();
	gWriter.clear();


    TRANS& trans = ats.trans;
    ATTR2ST(trans, termid);

	std::string body;
	pack_body(trans,body);

	LOG(DEBUG, "[pack success, body:][" << body << "]");

	std::string resp;
	ret=send_to_bank(body,resp);
    if (ret)
    {
        ERRTIP("local与银行通信失败");
		LOG(DEBUG, "[connect failure][" << resp << "]");
        return E_COMMON_ERR;
    }

	LOG(DEBUG, "[connect success, resq:][" << resp << "]");

	TiXmlDocument doc;
    doc.Parse(resp.c_str());
    TiXmlElement* root = doc.FirstChildElement();

    if (!root)
    {
        ERRTIP( "local中银通返回数据解析错误");
        LOG(ERROR, "中银通返回数据解析错误");
        LOG(ERROR, "[" << resp << "]");
        return E_COMMON_ERR;
    }
	string value;
	//TiXmlNode** return_code
	TiXmlNode* Return_Field;
	if (!ec_xml_get_xml_childnode(*root,"Return",&Return_Field))
	{
		ERRTIP( "local中银通异常返回，解析异常返回错误");
		LOG(ERROR, "中银通异常返回，解析异常返回错误");
		LOG(ERROR, "[" << resp << "]");
		return E_COMMON_ERR;
	}
	{
		string return_message;
		if (ec_xml_get_xml_value(*Return_Field, "Return_Message", return_message))
		{
			ERRTIP( "bank return_message:[%s]",return_message.c_str());
            LOG(ERROR, return_message.c_str());
			LOG(ERROR, "[" << resp << "]");
			return E_COMMON_ERR;
		}
	}
	//检验mac
	/*string mac_info;
	ec_xml_get_xml_value(*Return_Field, "MESG_AUTHN_CODE", mac_info)
	ret=check_mac(mac_info,resp);
	if (ret)
		return ret;
	*/
	//解析报文
    int retcode = 0;
	std::string resp_info;

    {
        string value;
        if (!ec_xml_get_xml_value(*Return_Field, "RESP_CODE", value) || value.empty())
        {
            ERRTIP( "local中银通返回错误信息解析错误");
            LOG(ERROR, "中银通返回错误信息解析错误");
            LOG(ERROR, "[" << resp << "]");
            return E_COMMON_ERR;
        }
        istringstream is(value);
        is >> retcode;
    }

	if ( retcode!=0 && ec_xml_get_xml_value(*Return_Field, "RESP_INFO", resp_info))
	{
		ERRTIP("bank resp_info:[%s]", resp_info.c_str());
		//响应信息
		VAR2COL(resp_info);
		return E_COMMON_ERR;
	}

	string str_value;
	//stringstream ss;

    //持卡人名
    std::string     name;
    ec_xml_get_xml_value(*Return_Field, "CUST_NAME", name);
    VAR2COL(name);

    //持卡人姓
    std::string     familyname;
    ec_xml_get_xml_value(*Return_Field, "CUST_SURNAME", familyname);
    VAR2COL(familyname);

    //客户证件类型
    std::string     idtype;
    ec_xml_get_xml_value(*Return_Field, "CERT_TYPE", idtype);
    VAR2COL(idtype);

    //客户证件号码
    std::string     idcode;
    ec_xml_get_xml_value(*Return_Field, "CERT_ID", idcode);
    VAR2COL(idcode);

    //卡销售日期
    std::string     opendate;
    ec_xml_get_xml_value(*Return_Field, "OPEN_DATE", opendate);
    VAR2COL(opendate);

    //卡作废日期
    std::string     closedate;
    ec_xml_get_xml_value(*Return_Field, "CLOSE_DATE", closedate);
    VAR2COL(closedate);

    //HOLD日期
    std::string     holddate;
    ec_xml_get_xml_value(*Return_Field, "HOLD_DATE", holddate);
    VAR2COL(holddate);

    //Release日期
    std::string     releasedate;
    ec_xml_get_xml_value(*Return_Field, "RELEASE_DATE", releasedate);
    VAR2COL(releasedate);

    //卡状态
    std::string     cardstatus;
    ec_xml_get_xml_value(*Return_Field, "CARD_STATE", cardstatus);
    VAR2COL(cardstatus);

    //卡余额
    std::string     onlinebal;
    ec_xml_get_xml_value(*Return_Field, "CARD_BAL", onlinebal);
    VAR2COL(onlinebal);

    //产品名称
    std::string     productname;
    ec_xml_get_xml_value(*Return_Field, "PRD_NAME", productname);
    VAR2COL(productname);

    //初始金额
    int     initamt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "INIT_PRICE", str_value);
            istringstream ss(str_value);
            ss>>initamt;
    }
    VAR2COL(initamt);

    //昨日余额
    int     yesterdayamt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "LAST_BAL", str_value);
            istringstream ss(str_value);
            ss>>yesterdayamt;
    }
    VAR2COL(yesterdayamt);

    //卡片激活日期
    std::string     activedate;
    ec_xml_get_xml_value(*Return_Field, "ACTIVE_DATE", activedate);
    VAR2COL(activedate);

    //卡片销售网点代码
    std::string     saleinstcode;
    ec_xml_get_xml_value(*Return_Field, "SALE_INST_CODE", saleinstcode);
    VAR2COL(saleinstcode);

    //卡片激活网点代码
    std::string     activeinstcode;
    ec_xml_get_xml_value(*Return_Field, "ACTIVE_INST_CODE", activeinstcode);
    VAR2COL(activeinstcode);

    //有效日期
    std::string     expiredate;
    ec_xml_get_xml_value(*Return_Field, "CARD_VAL_PERIOD", expiredate);
    VAR2COL(expiredate);

    //预制卡日期
    std::string     precrtdate;
    ec_xml_get_xml_value(*Return_Field, "PRE_CRT_DATE", precrtdate);
    VAR2COL(precrtdate);

    //换卡日期
    std::string     exchangedate;
    ec_xml_get_xml_value(*Return_Field, "EXH_DATE", exchangedate);
    VAR2COL(exchangedate);

    //新卡号
    std::string     newcardno;
    ec_xml_get_xml_value(*Return_Field, "NEW_CARDNO", newcardno);
    VAR2COL(newcardno);

    //原卡号
    std::string     oldcardno;
    ec_xml_get_xml_value(*Return_Field, "OLD_CARDNO", oldcardno);
    VAR2COL(oldcardno);

    //最近交易日期
    std::string     lasttransdate;
    ec_xml_get_xml_value(*Return_Field, "LAST_TRAN_DATE", lasttransdate);
    VAR2COL(lasttransdate);

    //Hold/release
    std::string     holdorrelease;
    ec_xml_get_xml_value(*Return_Field, "HOLD_RELEASE", holdorrelease);
    VAR2COL(holdorrelease);

    //可作废日期
    std::string     canceldate;
    ec_xml_get_xml_value(*Return_Field, "CANCEL_DATE", canceldate);
    VAR2COL(canceldate);

    //CURR
    std::string     currcoe;
    ec_xml_get_xml_value(*Return_Field, "CURR_CODE", currcoe);
    VAR2COL(currcoe);

    //当日取现累计金额
    int     withdrawdaytotal;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "CASH_DAY_AMT", str_value);
            istringstream ss(str_value);
            ss>>withdrawdaytotal;
    }
    VAR2COL(withdrawdaytotal);

    //当日取现累计次数
    int     withdrawdaytimes;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "CASH_DAY_NUM", str_value);
            istringstream ss(str_value);
            ss>>withdrawdaytimes;
    }
    VAR2COL(withdrawdaytimes);

    //末次取现日期
    std::string     lastwithdrawdate;
    ec_xml_get_xml_value(*Return_Field, "LAST_CASH_DATE", lastwithdrawdate);
    VAR2COL(lastwithdrawdate);

    //密码当日错误次数
    int     pinerrcnt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "PINERRTDY", str_value);
            istringstream ss(str_value);
            ss>>pinerrcnt;
    }
    VAR2COL(pinerrcnt);

    //密码错误总数
    int     pinerrtimes;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "PINERRSUM", str_value);
            istringstream ss(str_value);
            ss>>pinerrtimes;
    }
    VAR2COL(pinerrtimes);

    //密码错误日期
    std::string     pinerrday;
    ec_xml_get_xml_value(*Return_Field, "PINERRDAT", pinerrday);
    VAR2COL(pinerrday);

    //CVN错误总数
    int     cvnerrtimes;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "CVNERRSUM", str_value);
            istringstream ss(str_value);
            ss>>cvnerrtimes;
    }
    VAR2COL(cvnerrtimes);

    //脱机账户余额
    int     offlineamt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "ICACCTBAL", str_value);
            istringstream ss(str_value);
            ss>>offlineamt;
    }
    VAR2COL(offlineamt);

    //ec_bank_any_ytc::calc_trans_mac(&root);


	gWriter.addRow();

	LOG(DEBUG, "function completed");

    return 0;
}
