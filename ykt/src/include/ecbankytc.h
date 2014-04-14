#ifndef _ECBANKYTC_H_
#define _ECBANKYTC_H_

#include <string>
#include "tinyxml.h"
#include "pubdb.h"
#include "ecbankfunc.h"

class ec_bank_any_ytc : public ec_bank_trans
{
public:
    ec_bank_any_ytc();
    virtual ~ec_bank_any_ytc();
    int transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para);
    int post_transfer(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para, int write_card);
protected:
    virtual int send_to_anybank(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para);
    int send_to_ytc(T_t_ecloaddtl& ecloaddtl, const std::string& req, std::string& resp);
    int pack_body(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para, std::string& body);
    int pack_reverse_body(T_t_ecloaddtl& ecloaddtl,const ec_bank_trans_param& para,
                          std::string& body, int write_card);
    int reverse_to_ytc(T_t_ecloaddtl& ecloaddtl, const std::string& req, std::string& resp);
    int pack_post_trans_body(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para,
                             int write_card,  std::string& body);
    //int retry_to_ytc(T_t_ecloaddtl& ecloaddtl, std::string& resp);
    //int pack_query_body(T_t_ecloaddtl& ecloaddtl, std::string& body);
    //std::string parse_ytc_errmsg(const std::string& errmsg);
public:
    static int calc_trans_mac(TiXmlNode* root);
    static int check_trans_mac(TiXmlNode* root);
private:
    static std::string escape_value(TiXmlHandle& element);
    static std::string get_mac_data(TiXmlNode* root);
};

class ec_bank_cash_ytc : public ec_bank_any_ytc
{
public:
    ec_bank_cash_ytc();
    virtual ~ec_bank_cash_ytc();
protected:
    virtual int send_to_anybank(T_t_ecloaddtl& ecloaddtl, const ec_bank_trans_param& para);
};

#endif // _ECBANKYTC_H_
