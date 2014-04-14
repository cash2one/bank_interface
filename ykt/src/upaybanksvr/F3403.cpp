/* --------------------------------------------
 * ��������: 2013-06-4
 * ��������: hd
 * �汾��Ϣ: 1.0.0.0
 * ������: IC��������Ϣ��ѯ
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

	TiXmlNode& root = *(doc.FirstChild());

 	//�汾��
	ec_add_xml_node_value(root,"VERSION", "010101" );

	//���״���
	ec_add_xml_node_value(root,"TRANS_TYPE", "120521" );

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

	string  bankcardno;   //����

	//����
	{
		stringstream ss;
		COL2VAR(bankcardno);
		ss << bankcardno;
		ec_add_xml_node_value(root,"CARD_NO",ss.str());
	}

	//������ѡ��
	ec_add_xml_node_value(root,"CHK_OPTION", "0");

	//����
	ec_add_xml_node_value(root,"PASSWORD", "12345678");

	//���ļ�����
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
	//stringstream ss;

    //�ֿ�����
    std::string     name;
    ec_xml_get_xml_value(*Return_Field, "CUST_NAME", name);
    VAR2COL(name);

    //�ֿ�����
    std::string     familyname;
    ec_xml_get_xml_value(*Return_Field, "CUST_SURNAME", familyname);
    VAR2COL(familyname);

    //�ͻ�֤������
    std::string     idtype;
    ec_xml_get_xml_value(*Return_Field, "CERT_TYPE", idtype);
    VAR2COL(idtype);

    //�ͻ�֤������
    std::string     idcode;
    ec_xml_get_xml_value(*Return_Field, "CERT_ID", idcode);
    VAR2COL(idcode);

    //����������
    std::string     opendate;
    ec_xml_get_xml_value(*Return_Field, "OPEN_DATE", opendate);
    VAR2COL(opendate);

    //����������
    std::string     closedate;
    ec_xml_get_xml_value(*Return_Field, "CLOSE_DATE", closedate);
    VAR2COL(closedate);

    //HOLD����
    std::string     holddate;
    ec_xml_get_xml_value(*Return_Field, "HOLD_DATE", holddate);
    VAR2COL(holddate);

    //Release����
    std::string     releasedate;
    ec_xml_get_xml_value(*Return_Field, "RELEASE_DATE", releasedate);
    VAR2COL(releasedate);

    //��״̬
    std::string     cardstatus;
    ec_xml_get_xml_value(*Return_Field, "CARD_STATE", cardstatus);
    VAR2COL(cardstatus);

    //�����
    std::string     onlinebal;
    ec_xml_get_xml_value(*Return_Field, "CARD_BAL", onlinebal);
    VAR2COL(onlinebal);

    //��Ʒ����
    std::string     productname;
    ec_xml_get_xml_value(*Return_Field, "PRD_NAME", productname);
    VAR2COL(productname);

    //��ʼ���
    int     initamt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "INIT_PRICE", str_value);
            istringstream ss(str_value);
            ss>>initamt;
    }
    VAR2COL(initamt);

    //�������
    int     yesterdayamt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "LAST_BAL", str_value);
            istringstream ss(str_value);
            ss>>yesterdayamt;
    }
    VAR2COL(yesterdayamt);

    //��Ƭ��������
    std::string     activedate;
    ec_xml_get_xml_value(*Return_Field, "ACTIVE_DATE", activedate);
    VAR2COL(activedate);

    //��Ƭ�����������
    std::string     saleinstcode;
    ec_xml_get_xml_value(*Return_Field, "SALE_INST_CODE", saleinstcode);
    VAR2COL(saleinstcode);

    //��Ƭ�����������
    std::string     activeinstcode;
    ec_xml_get_xml_value(*Return_Field, "ACTIVE_INST_CODE", activeinstcode);
    VAR2COL(activeinstcode);

    //��Ч����
    std::string     expiredate;
    ec_xml_get_xml_value(*Return_Field, "CARD_VAL_PERIOD", expiredate);
    VAR2COL(expiredate);

    //Ԥ�ƿ�����
    std::string     precrtdate;
    ec_xml_get_xml_value(*Return_Field, "PRE_CRT_DATE", precrtdate);
    VAR2COL(precrtdate);

    //��������
    std::string     exchangedate;
    ec_xml_get_xml_value(*Return_Field, "EXH_DATE", exchangedate);
    VAR2COL(exchangedate);

    //�¿���
    std::string     newcardno;
    ec_xml_get_xml_value(*Return_Field, "NEW_CARDNO", newcardno);
    VAR2COL(newcardno);

    //ԭ����
    std::string     oldcardno;
    ec_xml_get_xml_value(*Return_Field, "OLD_CARDNO", oldcardno);
    VAR2COL(oldcardno);

    //�����������
    std::string     lasttransdate;
    ec_xml_get_xml_value(*Return_Field, "LAST_TRAN_DATE", lasttransdate);
    VAR2COL(lasttransdate);

    //Hold/release
    std::string     holdorrelease;
    ec_xml_get_xml_value(*Return_Field, "HOLD_RELEASE", holdorrelease);
    VAR2COL(holdorrelease);

    //����������
    std::string     canceldate;
    ec_xml_get_xml_value(*Return_Field, "CANCEL_DATE", canceldate);
    VAR2COL(canceldate);

    //CURR
    std::string     currcoe;
    ec_xml_get_xml_value(*Return_Field, "CURR_CODE", currcoe);
    VAR2COL(currcoe);

    //����ȡ���ۼƽ��
    int     withdrawdaytotal;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "CASH_DAY_AMT", str_value);
            istringstream ss(str_value);
            ss>>withdrawdaytotal;
    }
    VAR2COL(withdrawdaytotal);

    //����ȡ���ۼƴ���
    int     withdrawdaytimes;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "CASH_DAY_NUM", str_value);
            istringstream ss(str_value);
            ss>>withdrawdaytimes;
    }
    VAR2COL(withdrawdaytimes);

    //ĩ��ȡ������
    std::string     lastwithdrawdate;
    ec_xml_get_xml_value(*Return_Field, "LAST_CASH_DATE", lastwithdrawdate);
    VAR2COL(lastwithdrawdate);

    //���뵱�մ������
    int     pinerrcnt;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "PINERRTDY", str_value);
            istringstream ss(str_value);
            ss>>pinerrcnt;
    }
    VAR2COL(pinerrcnt);

    //�����������
    int     pinerrtimes;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "PINERRSUM", str_value);
            istringstream ss(str_value);
            ss>>pinerrtimes;
    }
    VAR2COL(pinerrtimes);

    //�����������
    std::string     pinerrday;
    ec_xml_get_xml_value(*Return_Field, "PINERRDAT", pinerrday);
    VAR2COL(pinerrday);

    //CVN��������
    int     cvnerrtimes;
    {
			str_value=string("");
            ec_xml_get_xml_value(*Return_Field, "CVNERRSUM", str_value);
            istringstream ss(str_value);
            ss>>cvnerrtimes;
    }
    VAR2COL(cvnerrtimes);

    //�ѻ��˻����
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
