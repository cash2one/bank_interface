/* --------------------------------------------
 * ��������: 2013-06-4
 * ��������: hd
 * �汾��Ϣ: 1.0.0.0
 * ������: IC������
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
	//ȡtag����
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

	//ȡtag��ֵ����
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

	//ȡtag��ֵ
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

	//���ɽ�����ˮ�Ų���¼
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

 	//�汾��
	ec_add_xml_node_value(root,"VERSION", "010101" );

	//���״���
	ec_add_xml_node_value(root,"TRANS_TYPE", "120321" );

	//��������
	ec_add_xml_node_value(root,"ACTION_TYPE", "1");

	/*//��Ӧ����
	ec_add_xml_node_value(root,"RESP_CODE", );*/

	//��¼�������
	ec_add_xml_node_value(root,"LOGIN_TYPE", "1");

	//��������
	ec_add_xml_node_value(root,"TRANS_SOURCE", "BP");

	//��¼��������
	ec_add_xml_node_value(root,"LOGIN_INST_CODE", "48080000");

	//��������
	{
		string str;
		str.assign(trans.accdate,8);
		ec_add_xml_node_value(root,"TXN_DATE", str);
	}

	//����ʱ��
	{
		string str;
		str.assign(trans.acctime,6);
		ec_add_xml_node_value(root,"TXN_TIME", str);
	}

	//���ٺ�/��ˮ��
	{
		string str;
		str.assign(trans.refno,14);
		//str[6]='0';
		//str[7]='0';
		str=str.substr(6);
		//str.append(1,'1');
		ec_add_xml_node_value(root,"TRACE_NO", str);
	}

	//��¼�������
	{
		string str;

		ret = get_ec_para(YTCEC_LOGIN_MERCH, str);
		if (ret)
		{
			LOG(ERROR, "����ͨ��¼�������δ����");
			return E_COMMON_ERR;
		}
		ec_add_xml_node_value(root,"LOGIN_MERCH_CODE", str);
	}
	//ec_add_xml_node_value(root,"LOGIN_MERCH_CODE", "J001");

	//��¼����Ա����
	{
		string str;

		ret = get_ec_para(YTCEC_LOGIN_USER, str);
		if (ret)
		{
			LOG(ERROR, "����ͨ��¼����Ա����δ����");
			return E_COMMON_ERR;
		}
		ec_add_xml_node_value(root,"LOGIN_USER_CODE", str);
	}
	//ec_add_xml_node_value(root,"LOGIN_USER_CODE", "A5");

	std::string	ocard_no	;	//	�ɿ�����
	std::string	validtid	;	//	�ֿ��������֤
	std::string	password	;	//	����
	std::string	idcode	;	//	�ͻ�֤������
	std::string	idtype	;	//	�ͻ�֤������
	std::string	name	;	//	�ͻ���
	std::string	familyname	;	//	�ͻ���
	std::string	name_spell	;	//	ƴ����
	std::string	familyname_spell	;	//	ƴ����
	std::string	birthday	;	//	����
	std::string	telphone	;	//	�绰����
	std::string	mobile	;	//	�ֻ�
	std::string	corpname	;	//	������λ
	std::string	address	;	//	��ַ
	std::string	postcode	;	//	�ʱ�
	std::string	email	;	//	����
	std::string	roomno	;	//	����
	std::string	floorno	;	//	¥��
	std::string	homeaddr	;	//	סַ
	std::string	estate	;	//	ESTATE
	std::string	street	;	//	�ֵ�
	std::string	district	;	//	DISTRICT
	std::string	area	;	//	����
	std::string	newcardno	;	//	�¿���
	std::string	selectpin	;	//	������ѡ��
	std::string	activeswitch	;	//	�Ƿ񼤻�
	std::string	feeswitch	;	//	�շѱ�־
	std::string	fee	;	//	�����ѽ��
	std::string	paytype	;	//	���ʽ
	std::string	creditcardno	;	//	���ÿ���
	std::string	memocode	;	//	��ִ����
	std::string	reason	;	//	����ԭ��
	std::string newcardfield55; //55��
    string  reserved;       //����

    //�ɿ�����
    {
            ss.str("");
            COL2VAR(ocard_no);
            ss << ocard_no;
            ec_add_xml_node_value(root,"PRIMARY_ACCT_NUM",ss.str());
    }

    //�ֿ��������֤
    {
            ss.str("");
            COL2VAR(validtid);
            ss << validtid;
            ec_add_xml_node_value(root,"VALIDTID",ss.str());
    }

    //����
    {
            ss.str("");
            COL2VAR(password);
            ss << password;
            ec_add_xml_node_value(root,"CUSTPASSWORD",ss.str());
    }

    //�ͻ�֤������
    {
            ss.str("");
            COL2VAR(idcode);
            ss << idcode;
            ec_add_xml_node_value(root,"IDTYPE",ss.str());
    }

    //�ͻ�֤������
    {
            ss.str("");
            COL2VAR(idtype);
            ss << idtype;
            ec_add_xml_node_value(root,"IDCODE",ss.str());
    }

    //�ͻ���
    {
            ss.str("");
            COL2VAR(name);
            ss << name;
            ec_add_xml_node_value(root,"NAME",ss.str());
    }

    //�ͻ���
    {
            ss.str("");
            COL2VAR(familyname);
            ss << familyname;
            ec_add_xml_node_value(root,"SNAME",ss.str());
    }

    //ƴ����
    {
            ss.str("");
            COL2VAR(name_spell);
            ss << name_spell;
            ec_add_xml_node_value(root,"SPELL_NAME",ss.str());
    }

    //ƴ����
    {
            ss.str("");
            COL2VAR(familyname_spell);
            ss << familyname_spell;
            ec_add_xml_node_value(root,"SPELL_SNAME",ss.str());
    }

    //����
    {
            ss.str("");
            COL2VAR(birthday);
            ss << birthday;
            ec_add_xml_node_value(root,"BIRTHDAY",ss.str());
    }

    //�绰����
    {
            ss.str("");
            COL2VAR(telphone);
            ss << telphone;
            ec_add_xml_node_value(root,"COPHONE",ss.str());
    }

    //�ֻ�
    {
            ss.str("");
            COL2VAR(mobile);
            ss << mobile;
            ec_add_xml_node_value(root,"MOBILE",ss.str());
    }

    //������λ
    {
            ss.str("");
            COL2VAR(corpname);
            ss << corpname;
            ec_add_xml_node_value(root,"BUS_NAME",ss.str());
    }

    //��ַ
    {
            ss.str("");
            COL2VAR(address);
            ss << address;
            ec_add_xml_node_value(root,"ADDR",ss.str());
    }

    //�ʱ�
    {
            ss.str("");
            COL2VAR(postcode);
            ss << postcode;
            ec_add_xml_node_value(root,"POST",ss.str());
    }

    //����
    {
            ss.str("");
            COL2VAR(email);
            ss << email;
            ec_add_xml_node_value(root,"EMAIL",ss.str());
    }

    //����
    {
            ss.str("");
            COL2VAR(roomno);
            ss << roomno;
            ec_add_xml_node_value(root,"ROOM",ss.str());
    }

    //¥��
    {
            ss.str("");
            COL2VAR(floorno);
            ss << floorno;
            ec_add_xml_node_value(root,"FLOOR",ss.str());
    }

    //סַ
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

    //�ֵ�
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

    //����
    {
            ss.str("");
            COL2VAR(area);
            ss << area;
            ec_add_xml_node_value(root,"REGION",ss.str());
    }

    //�¿���
    {
            ss.str("");
            COL2VAR(newcardno);
            ss << newcardno;
            ec_add_xml_node_value(root,"NCARD_NO",ss.str());
    }

    //������ѡ��
    {
            ss.str("");
            COL2VAR(selectpin);
            ss << selectpin;
            ec_add_xml_node_value(root,"SELECTPIN",ss.str());
    }

    //�Ƿ񼤻�
    {
            ss.str("");
            COL2VAR(activeswitch);
            ss << activeswitch;
            ec_add_xml_node_value(root,"ACTIVE",ss.str());
    }

    //�շѱ�־
    {
            ss.str("");
            COL2VAR(feeswitch);
            ss << feeswitch;
            ec_add_xml_node_value(root,"FEEFLG",ss.str());
    }

    //�����ѽ��
    {
            ss.str("");
            COL2VAR(fee);
            ss << fee;
            ec_add_xml_node_value(root,"FEE",ss.str());
    }

    //���ʽ
    {
            ss.str("");
            COL2VAR(paytype);
            ss << paytype;
            ec_add_xml_node_value(root,"PAYMENTWAY",ss.str());
    }

    //���ÿ���
    {
            ss.str("");
            COL2VAR(creditcardno);
            ss << creditcardno;
            ec_add_xml_node_value(root,"CREDITNO",ss.str());
    }

    //��ִ����
    {
            ss.str("");
            COL2VAR(memocode);
            ss << memocode;
            ec_add_xml_node_value(root,"MEMOCODE",ss.str());
    }

    //����ԭ��
    {
            ss.str("");
            COL2VAR(reason);
            ss << reason;
            ec_add_xml_node_value(root,"REAS",ss.str());
    }

	//IC�������򣨶�Ӧ������׼��55�����ж���������

	ec_add_xml_node_value(root,"ICC_DATA", "");
	TiXmlNode* icc_data;

	ec_xml_get_xml_childnode(root,"ICC_DATA",&icc_data);

	//����55���ֶΣ�����map

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

	//���׼�����
    ec_add_xml_node_value(*icc_data, "TAG_9F36", tag_map["9F36"]);


    //Ӧ������
    ec_add_xml_node_value(*icc_data, "TAG_9F26", tag_map["9F26"]);


    //�������Զ�������
    ec_add_xml_node_value(*icc_data, "TAG_9F10", tag_map["9F10"]);


    //��Ȩ���
    ec_add_xml_node_value(*icc_data, "TAG_9F02", tag_map["9F02"]);


    //�������
    ec_add_xml_node_value(*icc_data, "TAG_9F03", tag_map["9F03"]);


    //�ն˹��Ҵ���
    ec_add_xml_node_value(*icc_data, "TAG_9F1A", tag_map["9F1A"]);


    //�ն���֤���
    ec_add_xml_node_value(*icc_data, "TAG_95", tag_map["95"]);


    //���׻��Ҵ���
    ec_add_xml_node_value(*icc_data, "TAG_5F2A", tag_map["5F2A"]);


    //��������
    ec_add_xml_node_value(*icc_data, "TAG_9A", tag_map["9A"]);


    //��������
    ec_add_xml_node_value(*icc_data, "TAG_9C", tag_map["9C"]);


    //����Ԥ֪��
    ec_add_xml_node_value(*icc_data, "TAG_9F37", tag_map["9F37"]);


    //Ӧ�ý�������
    ec_add_xml_node_value(*icc_data, "TAG_82", tag_map["82"]);


    //��һ���ֻ��Ҵ���
    ec_add_xml_node_value(*icc_data, "TAG_9F71", tag_map["9F71"]);


    //��һ�������
    ec_add_xml_node_value(*icc_data, "TAG_9F79", tag_map["9F79"]);


	//������
	{
		ss.str("");
		COL2VAR(reserved);
		ss<<reserved;
		ec_add_xml_node_value(root,"RESERVED",ss.str());
	}

	//���ļ�����
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
        LOG(ERROR, "����ͨ��ʱʱ��δ����");
        return E_COMMON_ERR;
    }

	timeout*=1000;

    ret = get_ec_para(YTCEC_SVRIP, ytc_ip);
    if (ret)
    {
        LOG(ERROR, "����ͨǰ�û�δ����");
        return E_COMMON_ERR;
    }
    ret = get_ec_para(YTCEC_SVRPORT, ytc_port);
    if (ret)
    {
        LOG(ERROR, "����ͨǰ�û�δ����");
        return E_COMMON_ERR;
    }
    LOG(DEBUG, "����ͨ������[" << ytc_ip << ":" << ytc_port << "]");

    CTcpSocket sock;
    alarm(timeout / 1000);
    if (!sock.ConnectTcp((char*)ytc_ip.c_str(), ytc_port))
    {
        alarm(0);
        LOG(ERROR, "connect to ytc error");
        return E_COMMON_ERR;
    }
    alarm(0);

    // ����ͨ����
    stringstream ss;
    ss << setw(4) << setfill('0') <<  req.length() << req;

    LOG(DEBUG, "����ͨ����[" << ss.str() << "]");
    if (sock.Send((char*)ss.str().c_str(), ss.str().length()) < (int)ss.str().length())
    {
        LOG(ERROR, "Send to ytc error");
        return E_COMMON_ERR;
    }
    // ��ͷ
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
    // ����
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
        ERRTIP("local������ͨ��ʧ��");
		LOG(DEBUG, "[connect failure][" << resp << "]");
        return E_COMMON_ERR;
    }

	LOG(DEBUG, "[connect success, resq:][" << resp << "]");

	TiXmlDocument doc;
    doc.Parse(resp.c_str());
    TiXmlElement* root = doc.FirstChildElement();

    if (!root)
    {
        ERRTIP( "local����ͨ�������ݽ�������");
        LOG(ERROR, "����ͨ�������ݽ�������");
        LOG(ERROR, "[" << resp << "]");
        return E_COMMON_ERR;
    }
	string value;
	//TiXmlNode** return_code
	TiXmlNode* Return_Field;
	if (!ec_xml_get_xml_childnode(*root,"Return",&Return_Field))
	{
		ERRTIP( "local����ͨ�쳣���أ������쳣���ش���");
		LOG(ERROR, "����ͨ�쳣���أ������쳣���ش���");
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
	//����mac
	/*string mac_info;
	ec_xml_get_xml_value(*Return_Field, "MESG_AUTHN_CODE", mac_info)
	ret=check_mac(mac_info,resp);
	if (ret)
		return ret;
	*/
	//��������
    int retcode = 0;
	std::string resp_info;

    {
        string value;
        if (!ec_xml_get_xml_value(*Return_Field, "RESP_CODE", value) || value.empty())
        {
            ERRTIP( "local����ͨ���ش�����Ϣ��������");
            LOG(ERROR, "����ͨ���ش�����Ϣ��������");
            LOG(ERROR, "[" << resp << "]");
            return E_COMMON_ERR;
        }
        istringstream is(value);
        is >> retcode;
    }

	if ( retcode!=0 && ec_xml_get_xml_value(*Return_Field, "RESP_INFO", resp_info))
	{
		ERRTIP("bank resp_info:[%s]", resp_info.c_str());
		//��Ӧ��Ϣ
		VAR2COL(resp_info);
		return E_COMMON_ERR;
	}

	string str_value;

    //��Ӧ��Ϣ
    //std::string     resp_info;
    ec_xml_get_xml_value(*Return_Field, "RESP_INFO", resp_info);
    VAR2COL(resp_info);

    //�¿���Ʒ����
    std::string     newproductcode;
    ec_xml_get_xml_value(*Return_Field, "PRODUCT", newproductcode);
    VAR2COL(newproductcode);

    //�¿���Ʒ����
    std::string     newproductname;
    ec_xml_get_xml_value(*Return_Field, "CDTYNAME", newproductname);
    VAR2COL(newproductname);

    //�¿���Ƭ״̬
    std::string     newcardstatus;
    ec_xml_get_xml_value(*Return_Field, "STATUS", newcardstatus);
    VAR2COL(newcardstatus);

    //�˻����
    int     accbala;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "BALAMT", str_value);
            istringstream ss(str_value);
            ss>>accbala;
    }
    VAR2COL(accbala);

    //���ý��
    int     freebala;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "INITAMT", str_value);
            istringstream ss(str_value);
            ss>>freebala;
    }
    VAR2COL(freebala);

    //�¿��Ƿ������ֵ
    std::string     depositflag;
    ec_xml_get_xml_value(*Return_Field, "DEPOSITYN", depositflag);
    VAR2COL(depositflag);

    //�¿��Ƿ�����ATMȡ��
    std::string     atmflag;
    ec_xml_get_xml_value(*Return_Field, "ATM", atmflag);
    VAR2COL(atmflag);

    //��Ч��
    std::string     expiredate;
    ec_xml_get_xml_value(*Return_Field, "EXPDATE", expiredate);
    VAR2COL(expiredate);

    //�¿�����
    std::string     newpin;
    ec_xml_get_xml_value(*Return_Field, "NEWPIN", newpin);
    VAR2COL(newpin);

    //������ˮ��
    std::string     transno;
    ec_xml_get_xml_value(*Return_Field, "TRANNO", transno);
    VAR2COL(transno);

    //��һ���ִ���
    std::string     primarycurr;
    ec_xml_get_xml_value(*Return_Field, "CURR", primarycurr);
    VAR2COL(primarycurr);

    //��һ���ֻ������
    int     primaryexchamt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "EXCHAMT", str_value);
            istringstream ss(str_value);
            ss>>primaryexchamt;
    }
    VAR2COL(primaryexchamt);

    //�ڶ����ִ���
    std::string     secondcurr;
    ec_xml_get_xml_value(*Return_Field, "S_CURR", secondcurr);
    VAR2COL(secondcurr);

    //�ڶ����ִ�Ȧ����
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
