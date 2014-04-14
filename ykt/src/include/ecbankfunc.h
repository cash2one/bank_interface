#ifndef _ECBANKFUNC_H_
#define _ECBANKFUNC_H_

#include <sstream>
#include <string>
#include "tinyxml.h"
#include "pubdb.h"


#define ICBC_CODE "00"  // 工商银行
#define BOC_CODE "01"   // 中国银行
#define BANKCOMM_CODE "02" // 交通银行
#define CCB_CODE "03"      // 建设银行
#define YTC_CODE "04"       // 中银通
#define CASH_CODE "98"    // 现金交易
#define BANK_ANY_CODE "99" // 匹配任何银行


#define EC_PAYSTATUS_INIT 0
#define EC_PAYSTATUS_OK 1
#define EC_PAYSTATUS_FAILED 2
#define EC_PAYSTATUS_NOTSUPPORT 3
// 现金交易流水未完成
#define EC_PAYSTATUS_BROKEN 4

//保证金使用标志
#define MARGIN_INIT 0 //初始状态
#define MARGIN_NOUSE 1  //未使用
#define MARGIN_USED 2   //已使用
#define MARGIN_UNCONFIRMED 3    //无法确认


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
// 记录电子现金账户信息
int ec_update_account(T_t_ecloaddtl* dtl, T_t_ecaccount* account);
// 更新保证金金额
int ec_update_marginacc(T_t_ecloaddtl* dtl);

#define JHEC_SVRIP 1             // 交行前置机IP
#define JHEC_SVRPORT 2           // 交行前置机端口
#define JHEC_SETTLE_ACCNO 3      // 交行对公结算账号
#define JHEC_MECH_ID  4          // 交行电子现金代理商编号
#define JHEC_TIMEOUT  5			// 交行超时时间

#define GHEC_SVRIP 10             //工行前置机IP
#define GHEC_SVRPORT 11           // 工行前置机端口
#define GHEC_MECH_ID  12          // 工行接入商场号
#define GHEC_CUSTNO   13          // 工行接入单位编号
#define GHEC_MARGIN_ACCOUNT 14   // 工行跨行转账保证金账户
#define GHEC_TIMEOUT	15		// 工行超时时间

#define YTCEC_SVRIP   20          // 中银通前置机 IP
#define YTCEC_SVRPORT 21          // 中银通前置机 PORT
#define YTCEC_LOGIN_STATUS  22    // 中银通签到状态
#define YTCEC_MACKEY  23          // 中银通 MAC key
#define YTCEC_PINKEY  24          // 中银通 PIN key
#define YTCEC_ORANIZATION_KEY 25  // 中银通机构主密钥
#define YTCEC_ORANIZATION_CODE 26 // 中银通机构代码
#define YTCEC_TIMEOUT 27          // 中银通超时时间
#define YTCEC_LOGIN_USER 28         // 中银通操作员代码
#define YTCEC_LOGIN_MERCH 30         // 中银通操作员代码

#define ECARD_QC_SVR_BRANCH  40   // 一卡通圈存前置通讯平台节点号
#define ECARD_QC_SVR_MAINFUNC 41  // 一卡通圈存前置机主功能号
#define ECARD_CORE_SVR_BRANCH 42  // 一卡通核心后台通讯平台节点号
#define ECARD_CORE_SVR_MAINFUNC 43 // 一卡通核心后台主功能号
#define ECARD_SHORT_OF_MARGINACC 44 // 保证金余额不足提示错误信息

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
    enum { r_ok = 0, // 成功
           r_send, // 发送错误
           r_recv, // 接收错误
           r_conn, // 连接错误
           r_error // 银行交易错误
         };
    enum { wc_success = 0, // 写卡成功
           wc_failed , // 写卡失败
           wc_unconfirm, // 中途拔卡
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
