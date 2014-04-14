/* --------------------------------------------
 * 创建日期: 2013-06-4
 * 程序作者: hd
 * 版本信息: 1.0.0.0
 * 程序功能: IC卡批量售卡
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
	ec_add_xml_node_value(root,"TRANS_TYPE", "120226" );

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

	//受卡机终端标识码

	ec_add_xml_node_value(root,"CARD_ACCPTR_TERMNL_ID", "12345676");

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

	    string  cardno_begin;   //开始卡号
        string  cardno_end;     //结束卡号
        int     cardcnt;        //卡数量
        string  product_group_code;     //产品组代码
        string  product_code;   //产品代码
        int     in_fee_amt;     //内收手续费
        int     out_fee_amt;    //外收手续费
        int     receivable_amt; //应收金额
        int     is_guarantee_deposit;   //是否收取押金
        int     guarantee_deposit;      //押金
        int     is_get_fee;     //是否收费
        int     fee_amt;        //发卡费
        string  idcode; //持卡人证件号码
        string  idtype; //持卡人证件类型
        string  name;   //持卡人名
        string  familyname;     //持卡人姓
        string  name_spell;     //持卡人名-拼音
        string  familyname_spell;       //持卡人姓-拼音
        string  birthday;       //持卡人生日
        string  telphone;       //持卡人电话号码
        string  mobile; //持卡人手机
        string  address;        //持卡人地址
        string  postcode;       //持卡人邮编
        string  email;  //持卡人电邮
        string  corpname;       //持卡人单位名称
        string  comments;       //持卡人备注
        string  reserved;       //保留

	//卡片数量
	{
		stringstream ss;
		COL2VAR(cardcnt);
		ss << cardcnt;
		ec_add_xml_node_value(root,"CARDCOUNT",ss.str());
	}

	//起始卡号
	{
		stringstream ss;
		COL2VAR(cardno_begin);
		ss <<cardno_begin;
		ec_add_xml_node_value(root,"STARTCARD",ss.str());
	}

	//结束卡号
	{
		stringstream ss;
		COL2VAR(cardno_end);
		ss <<cardno_end;
		ec_add_xml_node_value(root,"ENDCARD",ss.str());
	}

	//产品组
	{
		stringstream ss;
		COL2VAR(product_group_code);
		ss <<product_group_code;
		ec_add_xml_node_value(root,"PRD_GRP_CODE",ss.str());
	}

	//产品
	{
		stringstream ss;
		COL2VAR(product_code);
		ss <<product_code;
		ec_add_xml_node_value(root,"PRD_CODE",ss.str());
	}

	//内收手续费
	{
		stringstream ss;
		COL2VAR(in_fee_amt);
		ss << setiosflags(ios::right)<<setw(12) <<setfill('0') <<in_fee_amt;
		ec_add_xml_node_value(root,"TOTAL_IN_FEE_AMT",ss.str());
	}

	//外收手续费
	{
		stringstream ss;
		COL2VAR(out_fee_amt);
		ss << setiosflags(ios::right)<<setw(12) <<setfill('0') <<out_fee_amt;
		ec_add_xml_node_value(root,"TOTAL_OUT_FEE_AMT",ss.str());
	}

	//应收金额
	{
		stringstream ss;
		COL2VAR(receivable_amt);
		ss << setiosflags(ios::right)<<setw(12) <<setfill('0') <<receivable_amt;
		ec_add_xml_node_value(root,"BALAMT",ss.str());
	}

	//是否收取押金
	{
		stringstream ss;
		COL2VAR(is_guarantee_deposit);
		ss <<is_guarantee_deposit;
		ec_add_xml_node_value(root,"PLEDGEFLG",ss.str());
	}

	//押金
	{
		stringstream ss;
		COL2VAR(guarantee_deposit);
		ss << setiosflags(ios::right)<<setw(12) <<setfill('0') <<guarantee_deposit;
		ec_add_xml_node_value(root,"PLEDGE",ss.str());
	}

	//是否收费
	{
		stringstream ss;
		COL2VAR(is_get_fee);
		ss <<is_get_fee;
		ec_add_xml_node_value(root,"FEEFLG",ss.str());
	}

	//发卡费
	{
		stringstream ss;
		COL2VAR(fee_amt);
		ss << setiosflags(ios::right)<<setw(12) <<setfill('0') <<fee_amt;
		ec_add_xml_node_value(root,"FEE",ss.str());
	}

	//持卡人证件号码
	{
		stringstream ss;
		COL2VAR(idcode);
		ss <<idcode;
		ec_add_xml_node_value(root,"IDCODE",ss.str());
	}

	//持卡人证件类型
	{
		stringstream ss;
		COL2VAR(idtype);
		ss <<idtype;
		ec_add_xml_node_value(root,"IDTYPE",ss.str());
	}

	//持卡人名
	{
		stringstream ss;
		COL2VAR(name);
		ss <<name;
		ec_add_xml_node_value(root,"NAME",ss.str());
	}

	//持卡人姓
	{
		stringstream ss;
		COL2VAR(familyname);
		ss <<familyname;
		ec_add_xml_node_value(root,"SURNAME",ss.str());
	}

	//持卡人拼音名
	{
		stringstream ss;
		COL2VAR(name_spell);
		ss <<name_spell;
		ec_add_xml_node_value(root,"SPELL_NAME",ss.str());
	}

	//持卡人拼音姓
	{
		stringstream ss;
		COL2VAR(familyname_spell);
		ss <<familyname_spell;
		ec_add_xml_node_value(root,"SPELL_SURNAME",ss.str());
	}

	//持卡人生日
	{
		stringstream ss;
		COL2VAR(birthday);
		ss <<birthday;
		ec_add_xml_node_value(root,"BIRTHDAY",ss.str());
	}

	//持卡人电话号码
	{
		stringstream ss;
		COL2VAR(telphone);
		ss<<telphone;
		ec_add_xml_node_value(root,"COPHONE",ss.str());
	}

	//持卡人手机
	{
		stringstream ss;
		COL2VAR(mobile);
		ss<<mobile;
		ec_add_xml_node_value(root,"MOBILE",ss.str());
	}

	//持卡人地址
	{
		stringstream ss;
		COL2VAR(address);
		ss<<address;
		ec_add_xml_node_value(root,"ADDR",ss.str());
	}

	//持卡人邮编
	{
		stringstream ss;
		COL2VAR(postcode);
		ss<<postcode;
		ec_add_xml_node_value(root,"POST",ss.str());
	}

	//持卡人电邮
	{
		stringstream ss;
		COL2VAR(email);
		ss<<email;
		ec_add_xml_node_value(root,"EMAIL",ss.str());
	}

	//持卡人单位名称
	{
		stringstream ss;
		COL2VAR(corpname);
		ss<<corpname;
		ec_add_xml_node_value(root,"COMNAME",ss.str());
	}

	//持卡人备注
	{
		stringstream ss;
		COL2VAR(comments);
		ss<<comments;
		ec_add_xml_node_value(root,"CUSTRES",ss.str());
	}

	//保留域
	{
		stringstream ss;
		COL2VAR(reserved);
		ss<<reserved;
		ec_add_xml_node_value(root,"RESERVED",ss.str());
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


FUNCTION(3402)
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

	value=string("");


	//卡内可用金额
	int available_amt;
	{
		ec_xml_get_xml_value(*Return_Field, "INITAMT", value);
		istringstream ss(value);
		ss>>available_amt;
	}

	//开卡日期
	std::string open_date;
	ec_xml_get_xml_value(*Return_Field, "OPENDATE", open_date);

	//卡片状态
	std::string card_status;
	ec_xml_get_xml_value(*Return_Field, "STATUS", card_status);

	VAR2COL(available_amt);
	VAR2COL(open_date);
	VAR2COL(card_status);

	gWriter.addRow();

	LOG(DEBUG, "function completed");

    return 0;
}
