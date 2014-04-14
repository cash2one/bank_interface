/* --------------------------------------------
 * 创建日期: 2013-6-4
 * 程序作者: hd
 * 版本信息: 1.0.0.0
 * 程序功能: IC卡费用查询
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
	ec_add_xml_node_value(root,"TRANS_TYPE", "120440" );

	//动作代码
	ec_add_xml_node_value(root,"ACTION_TYPE", "1");

	/*//响应代码
	ec_add_xml_node_value(root,"RESP_CODE", );*/

	//登录身份类型
	ec_add_xml_node_value(root,"LOGIN_TYPE", "1");

	//交易渠道
	ec_add_xml_node_value(root,"TRANS_SOURCE", "BP");

	//登录机构代码
	ec_add_xml_node_value(root,"LOGIN_INST_CODE", "10006");

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

	//费用类型
	std::string feetype;
	COL2VAR(feetype);
	ec_add_xml_node_value(root,"FEEGROUP_CODE", feetype);

	//卡号
	std::string bankcardno;
    COL2VAR(bankcardno);
	ec_add_xml_node_value(root,"PRIMARY_ACCT_NUM", bankcardno);

	//卡数量
	int cardcnt=0;
	COL2VAR(cardcnt);
	{
		stringstream ss;
        ss << cardcnt;
        ec_add_xml_node_value(root, "CARD_COUNT", ss.str());
	}

	//保留域
	{
		stringstream ss;
		std::string reserved("");
        ss << reserved;
        ec_add_xml_node_value(root, "RESERVED", ss.str());
	}

	//报文鉴别码
	// ec_add_xml_node_value(root, "MESG_AUTHN_CODE", "87AE");
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

FUNCTION(3401)
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
        ERRTIP("与银行通信失败");
		LOG(DEBUG, "[connect failure][" << resp << "]");
        return E_COMMON_ERR;
    }

	LOG(DEBUG, "[connect success resq:][" << resp << "]");

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
			ERRTIP( "bank return_message:[%s]", return_message.c_str());
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

	int in_fee_amt;
	int out_fee_amt;
	std::string feetype;
	std::string product_group_code;
	std::string product_code;
	int face_value;
	int guarantee_deposit;
	std::string fee_flag;
	int handing_charge;

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
		return E_COMMON_ERR;
	}

	value=string("");

	//内收手续费
	{
		value=string("");
		ec_xml_get_xml_value(*Return_Field, "TOTAL_IN_FEE_AMT", value);

		istringstream ss(value);
		ss>>in_fee_amt;
	}

	//外收手续费
	{
		value=string("");
		ec_xml_get_xml_value(*Return_Field, "TOTAL_OUT_FEE_AMT", value);

		istringstream ss(value);
		ss>>out_fee_amt;
	}

	//收费类型
		ec_xml_get_xml_value(*Return_Field, "FEE_RCV_TYPE", feetype);

	//产品组代码
		ec_xml_get_xml_value(*Return_Field, "PRD_GRP_CODE", product_group_code);

	//产品代码
		ec_xml_get_xml_value(*Return_Field, "PRD_CODE", product_code);

	//卡面值
	{
		value=string("");
		ec_xml_get_xml_value(*Return_Field, "CARD_UNIT_PRICE", value);

		istringstream ss(value);
		ss>>face_value;
	}

	//卡押金
	{
		value=string("");
		ec_xml_get_xml_value(*Return_Field, "PLEDGE", value);

		istringstream ss(value);
		ss>>guarantee_deposit;
	}

	//费用标志
		ec_xml_get_xml_value(*Return_Field, "FEE_RCV_FLAG", fee_flag);

	//单张卡片手续费
	{
		value=string("");
		ec_xml_get_xml_value(*Return_Field, "SIG_FEE_AMT", value);

		istringstream ss(value);
		ss>>handing_charge;
	}


	VAR2COL(in_fee_amt);
	VAR2COL(out_fee_amt);
	VAR2COL(feetype);
	VAR2COL(product_group_code);
	VAR2COL(product_code);
	VAR2COL(face_value);
	VAR2COL(guarantee_deposit);
	VAR2COL(fee_flag);
	VAR2COL(handing_charge);

	gWriter.addRow();

	LOG(DEBUG, "function completed");


    return 0;
}
