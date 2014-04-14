#ifndef _ECBANKFUNC_H_
#define _ECBANKFUNC_H_

#include <sstream>
#include <string>
#include "tinyxml.h"
#include "pubdb.h"


#define ICBC_CODE "00"  // ��������
#define BOC_CODE "01"   // �й�����
#define BANKCOMM_CODE "02" // ��ͨ����
#define CCB_CODE "03"      // ��������
#define YTC_CODE "04"       // ����ͨ
#define CASH_CODE "98"    // �ֽ���
#define BANK_ANY_CODE "99" // ƥ���κ�����


#define EC_PAYSTATUS_INIT 0
#define EC_PAYSTATUS_OK 1
#define EC_PAYSTATUS_FAILED 2
#define EC_PAYSTATUS_NOTSUPPORT 3
// �ֽ�����ˮδ���
#define EC_PAYSTATUS_BROKEN 4

//��֤��ʹ�ñ�־
#define MARGIN_INIT 0 //��ʼ״̬
#define MARGIN_NOUSE 1  //δʹ��
#define MARGIN_USED 2   //��ʹ��
#define MARGIN_UNCONFIRMED 3    //�޷�ȷ��


#define HEX_FMT(x) std::hex<<std::uppercase<<std::setw(x)<<std::setfill('0')
#define HEX_BYTE_FMT HEX_FMT(2)


std::string encode_hex(unsigned char* data, size_t datalen);
std::string encode_hex(const std::string& data);
std::size_t decode_hex(const std::string& data, unsigned char* hex);
std::string decode_hex(const std::string& data);

int get_bankcode_from_bankcardno(const char* bankcardno, T_t_bankcardheader* bankcode);

int get_ec_para(int paraid, std::string& paraval);
int get_ec_para(int paraid, int& paraval);

bool ec_add_xml_node_value(TiXmlNode& root, const std::string& node_name,
                           int node_value);
bool ec_add_xml_node_value(TiXmlNode& root, const std::string& node_name,
                           const std::string& node_value);
bool ec_xml_get_xml_value(TiXmlNode& root, const std::string& node_name,
                          std::string& node_value);
bool ec_xml_get_xml_childnode(TiXmlNode& root, const std::string& node_name,
                              TiXmlNode** child);

void ec_x9_19_mac_hex(const std::string& key_hex, const std::string& data_hex,
                      std::string& mac_hex);
void ec_x9_19_mac(const std::string& key, const std::string& data,
                  std::string& mac);

////////////////////////////////////////////////////////////////////////////////////
// ��¼�����ֽ��˻���Ϣ
int ec_update_account(T_t_ecloaddtl* dtl, T_t_ecaccount* account);
// ���±�֤����
int ec_update_marginacc(T_t_ecloaddtl* dtl);

#define JHEC_SVRIP 1             // ����ǰ�û�IP
#define JHEC_SVRPORT 2           // ����ǰ�û��˿�
#define JHEC_SETTLE_ACCNO 3      // ���жԹ������˺�
#define JHEC_MECH_ID  4          // ���е����ֽ�����̱��
#define JHEC_TIMEOUT  5			// ���г�ʱʱ��

#define GHEC_SVRIP 10             //����ǰ�û�IP
#define GHEC_SVRPORT 11           // ����ǰ�û��˿�
#define GHEC_MECH_ID  12          // ���н����̳���
#define GHEC_CUSTNO   13          // ���н��뵥λ���
#define GHEC_MARGIN_ACCOUNT 14   // ���п���ת�˱�֤���˻�
#define GHEC_TIMEOUT	15		// ���г�ʱʱ��

#define YTCEC_SVRIP   20          // ����ͨǰ�û� IP
#define YTCEC_SVRPORT 21          // ����ͨǰ�û� PORT
#define YTCEC_LOGIN_STATUS  22    // ����ͨǩ��״̬
#define YTCEC_MACKEY  23          // ����ͨ MAC key
#define YTCEC_PINKEY  24          // ����ͨ PIN key
#define YTCEC_ORANIZATION_KEY 25  // ����ͨ��������Կ
#define YTCEC_ORANIZATION_CODE 26 // ����ͨ��������
#define YTCEC_TIMEOUT 27          // ����ͨ��ʱʱ��
#define YTCEC_LOGIN_USER 28         // ����ͨ����Ա����
#define YTCEC_LOGIN_MERCH 30         // ����ͨ����Ա����

#define ECARD_QC_SVR_BRANCH  40   // һ��ͨȦ��ǰ��ͨѶƽ̨�ڵ��
#define ECARD_QC_SVR_MAINFUNC 41  // һ��ͨȦ��ǰ�û������ܺ�
#define ECARD_CORE_SVR_BRANCH 42  // һ��ͨ���ĺ�̨ͨѶƽ̨�ڵ��
#define ECARD_CORE_SVR_MAINFUNC 43 // һ��ͨ���ĺ�̨�����ܺ�
#define ECARD_SHORT_OF_MARGINACC 44 // ��֤��������ʾ������Ϣ

class ec_bank_trans_param
{
public:
    typedef std::map<std::string, std::string> value_type;
    ec_bank_trans_param();
    ~ec_bank_trans_param();

    bool has_value(const std::string& key) const;
    std::string get(const std::string& key) const;
    bool set(const std::string& key, const std::string& value);
    bool set(const std::string& key, int value);
    std::string& operator[](const std::string& key);
    const std::string operator[](const std::string& key) const;
    bool remove(const std::string& key);
    void clear();
private:
    ec_bank_trans_param(const ec_bank_trans_param&);
    ec_bank_trans_param& operator=(const ec_bank_trans_param&);

    value_type values_;
};

class ec_bank_trans;
class ec_bank_trans_load
{
public:
    ~ec_bank_trans_load();
    ec_bank_trans* get_transfer(const std::string& from_bank, const std::string& to_bank);
    bool register_transfer(const std::string& from_bank, const std::string& to_bank, ec_bank_trans* trans);
    static ec_bank_trans_load* instance();
    static void free();
private:
    static ec_bank_trans_load* s_instance_;
private:
    typedef std::map<std::string, ec_bank_trans*> trans_define_map;
    ec_bank_trans_load()
    {}
    ec_bank_trans_load(const ec_bank_trans_load&);
    ec_bank_trans_load& operator=(const ec_bank_trans_load&);

    trans_define_map define_map_;
};

#define EC_REGISTER_TRANS(fb,tb,trans) bool _b_##fb##tb = \
    ec_bank_trans_load::instance()->register_transfer(fb,tb,new trans())

#pragma pack(1)
typedef struct
{
    char bank_ip[32];
    int bank_port;
    int delay_time;
} ST_BANK_CONFIG;

#pragma pack()

class ec_bank_trans
{
public:
    enum { r_ok = 0, // �ɹ�
           r_send, // ���ʹ���
           r_recv, // ���մ���
           r_conn, // ���Ӵ���
           r_error // ���н��״���
         };
    enum { wc_success = 0, // д���ɹ�
           wc_failed , // д��ʧ��
           wc_unconfirm, // ��;�ο�
         };
    ec_bank_trans();
    virtual ~ec_bank_trans();
    virtual int transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para) = 0;
    virtual int post_transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para, int write_card) = 0;
    size_t get_buffer_as_int(const char* buffer, std::size_t len);
    void get_buffer_as_str(const char* buffer, std::size_t len, char* out);
    inline int debug() const
    {
        return debug_;
    }
    inline void set_debug(int level = 1)
    {
        debug_ = level;
    }
    int send_to_and_recv_from_bank(const ST_BANK_CONFIG* config, const char* pSend,
                                   size_t send_len, char* pRecv, size_t recv_len);
    inline bool is_use_margin_acc() const
    {
        return use_margin_acc_;
    }
    inline bool is_need_payment() const
    {
        return need_payment_;
    }
protected:
    virtual int send_to_anybank_ykt(T_t_ecloaddtl& ecloaddtl,
                                    const ec_bank_trans_param& para);
    int debug_;
    bool use_margin_acc_;
    bool need_payment_;
};

////////////////////////////////////////////////////////////////////////////////

class ec_bank_any_bocomm : public ec_bank_trans
{
public:
    ec_bank_any_bocomm();
    virtual ~ec_bank_any_bocomm();
    int transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para);
    int post_transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para, int write_card);
protected:
    // virtual int send_to_anybank(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para);
    virtual int send_to_bocomm(T_t_ecloaddtl& ecloaddtl, const char* transcode);
    std::size_t pack_body(T_t_ecloaddtl& ecloaddtl, char* body, const char* transcode);
    std::size_t pack_header(T_t_ecloaddtl& ecloaddtl, char* header, const char* transcode,
                            const char* body, std::size_t body_len);
};

class ec_bank_cash_bocomm : public ec_bank_any_bocomm
{
public:
    ec_bank_cash_bocomm();
    virtual ~ec_bank_cash_bocomm();
protected:
    virtual int send_to_anybank_ykt(T_t_ecloaddtl& ecloaddtl,
                                    const ec_bank_trans_param& para);
};

class ec_bank_icbc : public ec_bank_trans
{
public:
    ec_bank_icbc()
    {}
    virtual ~ec_bank_icbc()
    {}
    virtual int transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para);
    virtual int post_transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para, int write_card);
protected:
    void setDRBankcardNo(const std::string& drbankno);
    int do_transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para);
    int do_post_transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para, int write_card);
private:
    int pack_body(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para, std::string& body);
    int pack_post_trans_body(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para,
                             int write_card, std::string& body);
    int send_to_icbc(T_t_ecloaddtl& ecloaddtl, const std::string& req, std::string& resp);
    int retry_to_icbc(T_t_ecloaddtl& ecloaddtl, std::string& resp);
    int pack_query_body(T_t_ecloaddtl& ecloaddtl, std::string& body);
    std::string parse_icbc_errmsg(const std::string& errmsg);
    std::string dr_bankcardno_;
};

class ec_bank_any_to_icbc: public ec_bank_icbc
{
public:
    ec_bank_any_to_icbc()
    {}
    virtual ~ec_bank_any_to_icbc()
    {}
    virtual int transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para);
    virtual int post_transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para, int write_card);
};

class ec_bank_cash_to_icbc: public ec_bank_icbc
{
public:
    ec_bank_cash_to_icbc()
    {}
    virtual ~ec_bank_cash_to_icbc()
    {}
    virtual int transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para);
    virtual int post_transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para, int write_card);
};

#endif // _ECBANKFUNC_H_
