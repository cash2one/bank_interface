/* --------------------------------------------
 * 创建日期: 2013-06-4
 * 程序作者: hd
 * 版本信息: 1.0.0.0
 * 程序功能: IC卡换卡
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

static void get_first_field(std::string& icc_data, std::string& tag_field_name, std::string& tag_field_value)
{
	//取tag名字
	//LOG(DEBUG, "in get_first_field");
	size_t offset = 0;
	{
		if (icc_data[1]=='F')
			offset=4;
		else
			offset=2;
		tag_field_name=icc_data.substr(0,offset);
		icc_data=icc_data.substr(offset);
	}

	//取tag域值长度
	int field_len=0;

	{
		//stringstream ss;

		if( (icc_data[0] < '8') && (icc_data[0] >= '0'))
		{
			sscanf(icc_data.substr(0,2).c_str(),"%x",&field_len);
			icc_data=icc_data.substr(2);
		}
		else
		{
			//char tmp;
			int field_len_len;

			sscanf(icc_data.substr(0,2).c_str(),"%x",&field_len_len);
			field_len_len-=0x80;
			sscanf(icc_data.substr(2,field_len_len*2).c_str(),"%x",&field_len);
			icc_data=icc_data.substr((field_len_len+1)*2);
		}
	}

	//取tag域值
	{
		tag_field_value=icc_data.substr(0,field_len*2);
		icc_data=icc_data.substr(field_len*2);
	}
	//LOG(DEBUG, "out get_first_field");
}

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
	stringstream ss;

	TiXmlNode& root = *(doc.FirstChild());

 	//版本号
	ec_add_xml_node_value(root,"VERSION", "010101" );

	//交易代码
	ec_add_xml_node_value(root,"TRANS_TYPE", "120321" );

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

	std::string	ocard_no	;	//	旧卡卡号
	std::string	validtid	;	//	持卡人身份验证
	std::string	password	;	//	密码
	std::string	idcode	;	//	客户证件类型
	std::string	idtype	;	//	客户证件号码
	std::string	name	;	//	客户名
	std::string	familyname	;	//	客户姓
	std::string	name_spell	;	//	拼音名
	std::string	familyname_spell	;	//	拼音姓
	std::string	birthday	;	//	生日
	std::string	telphone	;	//	电话号码
	std::string	mobile	;	//	手机
	std::string	corpname	;	//	工作单位
	std::string	address	;	//	地址
	std::string	postcode	;	//	邮编
	std::string	email	;	//	电邮
	std::string	roomno	;	//	房间
	std::string	floorno	;	//	楼层
	std::string	homeaddr	;	//	住址
	std::string	estate	;	//	ESTATE
	std::string	street	;	//	街道
	std::string	district	;	//	DISTRICT
	std::string	area	;	//	地区
	std::string	newcardno	;	//	新卡号
	std::string	selectpin	;	//	卡密码选择
	std::string	activeswitch	;	//	是否激活
	std::string	feeswitch	;	//	收费标志
	std::string	fee	;	//	换卡费金额
	std::string	paytype	;	//	付款方式
	std::string	creditcardno	;	//	信用卡号
	std::string	memocode	;	//	回执号码
	std::string	reason	;	//	换卡原因
	std::string newcardfield55; //55域
    string  reserved;       //保留

    //旧卡卡号
    {
            ss.str("");
            COL2VAR(ocard_no);
            ss << ocard_no;
            ec_add_xml_node_value(root,"PRIMARY_ACCT_NUM",ss.str());
    }

    //持卡人身份验证
    {
            ss.str("");
            COL2VAR(validtid);
            ss << validtid;
            ec_add_xml_node_value(root,"VALIDTID",ss.str());
    }

    //密码
    {
            ss.str("");
            COL2VAR(password);
            ss << password;
            ec_add_xml_node_value(root,"CUSTPASSWORD",ss.str());
    }

    //客户证件类型
    {
            ss.str("");
            COL2VAR(idcode);
            ss << idcode;
            ec_add_xml_node_value(root,"IDTYPE",ss.str());
    }

    //客户证件号码
    {
            ss.str("");
            COL2VAR(idtype);
            ss << idtype;
            ec_add_xml_node_value(root,"IDCODE",ss.str());
    }

    //客户名
    {
            ss.str("");
            COL2VAR(name);
            ss << name;
            ec_add_xml_node_value(root,"NAME",ss.str());
    }

    //客户姓
    {
            ss.str("");
            COL2VAR(familyname);
            ss << familyname;
            ec_add_xml_node_value(root,"SNAME",ss.str());
    }

    //拼音名
    {
            ss.str("");
            COL2VAR(name_spell);
            ss << name_spell;
            ec_add_xml_node_value(root,"SPELL_NAME",ss.str());
    }

    //拼音姓
    {
            ss.str("");
            COL2VAR(familyname_spell);
            ss << familyname_spell;
            ec_add_xml_node_value(root,"SPELL_SNAME",ss.str());
    }

    //生日
    {
            ss.str("");
            COL2VAR(birthday);
            ss << birthday;
            ec_add_xml_node_value(root,"BIRTHDAY",ss.str());
    }

    //电话号码
    {
            ss.str("");
            COL2VAR(telphone);
            ss << telphone;
            ec_add_xml_node_value(root,"COPHONE",ss.str());
    }

    //手机
    {
            ss.str("");
            COL2VAR(mobile);
            ss << mobile;
            ec_add_xml_node_value(root,"MOBILE",ss.str());
    }

    //工作单位
    {
            ss.str("");
            COL2VAR(corpname);
            ss << corpname;
            ec_add_xml_node_value(root,"BUS_NAME",ss.str());
    }

    //地址
    {
            ss.str("");
            COL2VAR(address);
            ss << address;
            ec_add_xml_node_value(root,"ADDR",ss.str());
    }

    //邮编
    {
            ss.str("");
            COL2VAR(postcode);
            ss << postcode;
            ec_add_xml_node_value(root,"POST",ss.str());
    }

    //电邮
    {
            ss.str("");
            COL2VAR(email);
            ss << email;
            ec_add_xml_node_value(root,"EMAIL",ss.str());
    }

    //房间
    {
            ss.str("");
            COL2VAR(roomno);
            ss << roomno;
            ec_add_xml_node_value(root,"ROOM",ss.str());
    }

    //楼层
    {
            ss.str("");
            COL2VAR(floorno);
            ss << floorno;
            ec_add_xml_node_value(root,"FLOOR",ss.str());
    }

    //住址
    {
            ss.str("");
            COL2VAR(homeaddr);
            ss << homeaddr;
            ec_add_xml_node_value(root,"BUILDING",ss.str());
    }

    //ESTATE
    {
            ss.str("");
            COL2VAR(estate);
            ss << estate;
            ec_add_xml_node_value(root,"ESTATE",ss.str());
    }

    //街道
    {
            ss.str("");
            COL2VAR(street);
            ss << street;
            ec_add_xml_node_value(root,"STREET",ss.str());
    }

    //DISTRICT
    {
            ss.str("");
            COL2VAR(district);
            ss << district;
            ec_add_xml_node_value(root,"DISTRICT",ss.str());
    }

    //地区
    {
            ss.str("");
            COL2VAR(area);
            ss << area;
            ec_add_xml_node_value(root,"REGION",ss.str());
    }

    //新卡号
    {
            ss.str("");
            COL2VAR(newcardno);
            ss << newcardno;
            ec_add_xml_node_value(root,"NCARD_NO",ss.str());
    }

    //卡密码选择
    {
            ss.str("");
            COL2VAR(selectpin);
            ss << selectpin;
            ec_add_xml_node_value(root,"SELECTPIN",ss.str());
    }

    //是否激活
    {
            ss.str("");
            COL2VAR(activeswitch);
            ss << activeswitch;
            ec_add_xml_node_value(root,"ACTIVE",ss.str());
    }

    //收费标志
    {
            ss.str("");
            COL2VAR(feeswitch);
            ss << feeswitch;
            ec_add_xml_node_value(root,"FEEFLG",ss.str());
    }

    //换卡费金额
    {
            ss.str("");
            COL2VAR(fee);
            ss << fee;
            ec_add_xml_node_value(root,"FEE",ss.str());
    }

    //付款方式
    {
            ss.str("");
            COL2VAR(paytype);
            ss << paytype;
            ec_add_xml_node_value(root,"PAYMENTWAY",ss.str());
    }

    //信用卡号
    {
            ss.str("");
            COL2VAR(creditcardno);
            ss << creditcardno;
            ec_add_xml_node_value(root,"CREDITNO",ss.str());
    }

    //回执号码
    {
            ss.str("");
            COL2VAR(memocode);
            ss << memocode;
            ec_add_xml_node_value(root,"MEMOCODE",ss.str());
    }

    //换卡原因
    {
            ss.str("");
            COL2VAR(reason);
            ss << reason;
            ec_add_xml_node_value(root,"REAS",ss.str());
    }

	//IC卡数据域（对应银联标准中55域，下列都是其子域）

	ec_add_xml_node_value(root,"ICC_DATA", "");
	TiXmlNode* icc_data;

	ec_xml_get_xml_childnode(root,"ICC_DATA",&icc_data);

	//解析55域字段，存入map

	map<std::string,std::string> tag_map;
	COL2VAR(newcardfield55);
	string field55(newcardfield55);
	LOG(DEBUG, "55"<<newcardfield55);
	string tag_field_name(""),tag_field_value("");

    while (field55.length()>0)
    {
        get_first_field(field55,tag_field_name,tag_field_value);
		LOG(DEBUG, "tag_field_name"<<tag_field_name<<"tag_field_value"<<tag_field_value);
        tag_map[tag_field_name]=tag_field_value;

    }

	//交易计数器
    ec_add_xml_node_value(*icc_data, "TAG_9F36", tag_map["9F36"]);


    //应用密文
    ec_add_xml_node_value(*icc_data, "TAG_9F26", tag_map["9F26"]);


    //发卡行自定义数据
    ec_add_xml_node_value(*icc_data, "TAG_9F10", tag_map["9F10"]);


    //授权金额
    ec_add_xml_node_value(*icc_data, "TAG_9F02", tag_map["9F02"]);


    //其他金额
    ec_add_xml_node_value(*icc_data, "TAG_9F03", tag_map["9F03"]);


    //终端国家代码
    ec_add_xml_node_value(*icc_data, "TAG_9F1A", tag_map["9F1A"]);


    //终端验证结果
    ec_add_xml_node_value(*icc_data, "TAG_95", tag_map["95"]);


    //交易货币代码
    ec_add_xml_node_value(*icc_data, "TAG_5F2A", tag_map["5F2A"]);


    //交易日期
    ec_add_xml_node_value(*icc_data, "TAG_9A", tag_map["9A"]);


    //交易类型
    ec_add_xml_node_value(*icc_data, "TAG_9C", tag_map["9C"]);


    //不可预知数
    ec_add_xml_node_value(*icc_data, "TAG_9F37", tag_map["9F37"]);


    //应用交互特征
    ec_add_xml_node_value(*icc_data, "TAG_82", tag_map["82"]);


    //第一币种货币代码
    ec_add_xml_node_value(*icc_data, "TAG_9F71", tag_map["9F71"]);


    //第一币种余额
    ec_add_xml_node_value(*icc_data, "TAG_9F79", tag_map["9F79"]);


	//保留域
	{
		ss.str("");
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


FUNCTION(3404)
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

    //响应信息
    //std::string     resp_info;
    ec_xml_get_xml_value(*Return_Field, "RESP_INFO", resp_info);
    VAR2COL(resp_info);

    //新卡产品代号
    std::string     newproductcode;
    ec_xml_get_xml_value(*Return_Field, "PRODUCT", newproductcode);
    VAR2COL(newproductcode);

    //新卡产品名称
    std::string     newproductname;
    ec_xml_get_xml_value(*Return_Field, "CDTYNAME", newproductname);
    VAR2COL(newproductname);

    //新卡卡片状态
    std::string     newcardstatus;
    ec_xml_get_xml_value(*Return_Field, "STATUS", newcardstatus);
    VAR2COL(newcardstatus);

    //账户余额
    int     accbala;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "BALAMT", str_value);
            istringstream ss(str_value);
            ss>>accbala;
    }
    VAR2COL(accbala);

    //可用金额
    int     freebala;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "INITAMT", str_value);
            istringstream ss(str_value);
            ss>>freebala;
    }
    VAR2COL(freebala);

    //新卡是否允许充值
    std::string     depositflag;
    ec_xml_get_xml_value(*Return_Field, "DEPOSITYN", depositflag);
    VAR2COL(depositflag);

    //新卡是否允许ATM取现
    std::string     atmflag;
    ec_xml_get_xml_value(*Return_Field, "ATM", atmflag);
    VAR2COL(atmflag);

    //有效期
    std::string     expiredate;
    ec_xml_get_xml_value(*Return_Field, "EXPDATE", expiredate);
    VAR2COL(expiredate);

    //新卡密码
    std::string     newpin;
    ec_xml_get_xml_value(*Return_Field, "NEWPIN", newpin);
    VAR2COL(newpin);

    //交易流水号
    std::string     transno;
    ec_xml_get_xml_value(*Return_Field, "TRANNO", transno);
    VAR2COL(transno);

    //第一币种代码
    std::string     primarycurr;
    ec_xml_get_xml_value(*Return_Field, "CURR", primarycurr);
    VAR2COL(primarycurr);

    //第一币种换卡金额
    int     primaryexchamt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "EXCHAMT", str_value);
            istringstream ss(str_value);
            ss>>primaryexchamt;
    }
    VAR2COL(primaryexchamt);

    //第二币种代码
    std::string     secondcurr;
    ec_xml_get_xml_value(*Return_Field, "S_CURR", secondcurr);
    VAR2COL(secondcurr);

    //第二币种待圈存金额
    int     secondexchamt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "S_EXCHAMT", str_value);
            istringstream ss(str_value);
            ss>>secondexchamt;
    }
    VAR2COL(secondexchamt);




	gWriter.addRow();

	LOG(DEBUG, "function completed");

    return 0;
}
