/* --------------------------------------------
 * 创建日期: 2011-11-21
 * 程序作者: 汤成
 * 版本信息: 3.0.0.0
 * 程序功能: 圈存交易初始化
 * --------------------------------------------*/

#define _IN_SQC_
#include <stdio.h>
#include <string.h>
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

extern CSvrLink* g_pSvrLink;  // 与业务调度中心的连接

static int doCard(TRANS& trans, T_t_card& tCard)
{
    int ret = 0;
    ret = DB_t_card_read_by_cardno(trans.cardno, &tCard);
    if(ret)
    {
        LOG(ERROR, "cardno[" << trans.cardno << "]");
        if(DB_NOTFOUND == ret)
            return E_NOTEXIST_CARDNO;
        else
            return E_DB_CARD_R;
    }
    if(tCard.status[0] != STATUS_NORMAL)
    {
        return ERRINFO(E_CARD_LOGOUT, trans.cardno);
    }
    if('1' == tCard.lockflag[0])
    {
        ERRTIP("请将该卡注销");
        return ERRINFO(E_CARDNO_LOCKED, trans.cardno);
    }
    if('1' == tCard.frozeflag[0])
    {
        return ERRINFO(E_CARD_FREEZE, trans.cardno);
    }
    if('1' == tCard.lossflag[0])
    {
        return ERRINFO(E_CARD_LOST, trans.cardno);
    }
    if('1' == tCard.badflag[0])
    {
        return ERRINFO(E_CARD_BADRECORD, trans.cardno);
    }
    //比较卡物理ID是否相同
    if(strcmp(tCard.cardphyid, trans.cardphyid) != 0)
    {
        LOG(ERROR, "db cardphyid[" << tCard.cardphyid << "],input cardphyid[" << trans.cardphyid
            << "]");
        return E_CARD_PHYNO_DIFFER;
    }
    //检查卡的有效期
    if(strcmp(tCard.expiredate, trans.transdate) <= 0)
        return E_CARD_EXPIRED;
    return 0;
}

static int do_check_card_pwd(int custid, const char* pwd)
{
    // 发送请求到圈存前置机进行圈存交易
    ST_CPACK rCPack, aCPack;
    ST_PACK* rPack = &(rCPack.pack);

    ResetNormalCPack(&rCPack, 0, 1);
    rCPack.head.RequestType = 700000;
    SetHeadCol(&rCPack, F_SCUST_NO, F_SSTATUS0, F_SBANK_PWD, 0);

    sprintf(rPack->scust_no, "%d", custid);
    des2src(rPack->sbank_pwd, pwd);
    strcpy(rPack->sstatus0, "1");

    // 获取配置参数
    int ret;
    int ecard_branch, ecard_mainfunc;
    ret = get_ec_para(ECARD_CORE_SVR_BRANCH, ecard_branch);
    if(ret)
    {
        LOG(ERROR, "未配置一卡通后台节点号[" << ECARD_CORE_SVR_BRANCH << "]");
        return -1;
    }
    ret = get_ec_para(ECARD_CORE_SVR_MAINFUNC, ecard_mainfunc);
    if(ret)
    {
        LOG(ERROR, "未配置一卡通后台主功能号[" << ECARD_CORE_SVR_MAINFUNC << "]");
        return -1;
    }

    ST_PACK ArrayPack;
    memset(&ArrayPack, 0, sizeof ArrayPack);
    // 超时 45s
    LOG(DEBUG, "向一卡通系统请求branch[" << ecard_branch << "]basefunc[" << ecard_mainfunc
        << "]custid=" << rPack->scust_no << ";");
    ret = ExtCall(0, ecard_branch, ecard_mainfunc, 0, 45, &rCPack, &aCPack, &ArrayPack);
    // 重置 ParmBits
    memset(&rCPack, 0, sizeof rCPack);
    g_pSvrLink->SetCol(0, rCPack.head.ParmBits);

    if(ret < 0)
    {
        LOG(ERROR, "向一卡通请求，ret=" << ret << ",retcode=" << aCPack.head.retCode
            << ",msg=" << aCPack.pack.vsmess);
        if(-2 == ret)
        {
            LOG(DEBUG, "向一卡通系统请求超时");
            return -1; // 请求超时
        }
        else
        {
            LOG(DEBUG, "向一卡通系统请求失败");
            return -1;
        }
    }
    if(aCPack.head.retCode !=  0)
    {
        LOG(ERROR, "向一卡通系统请求错误，ret=" << aCPack.head.retCode << ",msg=" <<
            aCPack.pack.vsmess);
        ERRTIP("卡密码错误");
        return -1;
    }
    LOG(DEBUG, "向一卡通系统请求成功");
    return 0;
}

FUNCTION(3001)
{
    int ret = 0;
    CAccTrans& ats = CAccTrans::GetInst();


    TRANS& trans = ats.trans;
    ATTR2ST(trans, termid);
    char accdate[9] = {0}, refno[9] = {0};
    ATTR2VAR(accdate);
    ATTR2VAR(refno);

    gReader.fetchRow();

    COL2ST(trans, amount);
    COL2ST(trans, cardno);
    COL2ST(trans, cardphyid);

    std::string crbankcardno, drbankcardno, cardpwd;
    COL2VAR(crbankcardno);
    COL2VAR(drbankcardno);
    COL2VAR(cardpwd);

    //检查金额的合法性
    // if (trans.amount < 100 || trans.amount % 100 != 0)
    // 圈存金额不能为0
    if(trans.amount <= 0)
    {
        ERRTIP("输入金额不合法");
        return E_COMMON_ERR;
    }

    //检查终端合法性

    //检查卡片合法性
    T_t_card card;
    memset(&card, 0, sizeof card);
    ret = doCard(trans, card);
    if(ret)
    {
        return ret;
    }
    LOG(DEBUG, "电子现金初始化，custid=" << card.custid << ",cardno=" << card.cardno);
    T_t_customer customer;
    memset(&customer, 0, sizeof customer);
    ret = DB_t_customer_read_by_custid(card.custid, &customer);
    if(ret)
    {
        if(ret)
        {
            return E_DB_CUSTOMER_N;
        }
        return E_DB_CUSTOMER_R;
    }

    ret = do_check_card_pwd(card.custid , cardpwd.c_str());
    if(ret)
    {
        LOG(ERROR, "do_check_card_pwd error,ret=" << ret);
        return ret;
    }
    trans.custid = card.custid;

    //检查交易时间段
    T_t_bankcard crbankcard, drbankcard;
    memset(&crbankcard, 0, sizeof crbankcard);
    memset(&drbankcard, 0, sizeof drbankcard);

    //检查借方卡号
    ret = DB_t_bankcard_read_by_custid_and_bankcardno(card.custid, drbankcardno.c_str(),
            &drbankcard);
    if(ret)
    {
        LOG(ERROR, "DB_t_bankcard_read_by_custid_and_bankcardno error,ret=" << ret);
        if(DB_NOTFOUND == ret)
        {
            ERRTIP("借方银行账号未绑定");
            return E_COMMON_ERR;
        }
        ERRTIP("读取银行绑定关系表失败");
        return E_COMMON_ERR;
    }
    T_t_bankcardheader drbankcardheader;
    memset(&drbankcardheader, 0, sizeof drbankcardheader);
    ret = get_bankcode_from_bankcardno(drbankcardno.c_str(), &drbankcardheader);
    if(ret)
    {
        LOG(ERROR, "get_bankcode_from_bankcardno error,ret=" << ret);
        ERRTIP("借记卡不支持");
        return ret;
    }
    //检查贷方卡号
    T_t_bankcardheader crbankcardheader;
    memset(&crbankcardheader, 0, sizeof crbankcardheader);
    ret = get_bankcode_from_bankcardno(crbankcardno.c_str(), &crbankcardheader);
    if(ret)
    {
        LOG(ERROR, "get_bankcode_from_bankcardno error,ret=" << ret);
        ERRTIP("电子现金账号不支持");
        return ret;
    }
    ec_bank_trans* comm = ec_bank_trans_load::instance()->get_transfer(
                              drbankcardheader.bankcode, crbankcardheader.bankcode);
    if(!comm)
    {
        ERRTIP("不支持转账模式");
        return E_COMMON_ERR;
    }

    if(comm->is_use_margin_acc())
    {
        string margin_msg;
        ret = get_ec_para(ECARD_SHORT_OF_MARGINACC, margin_msg);
        if(ret)
        {
            margin_msg = "银行圈存接口异常，请通知网点工作人员或拨打服务热线400-830-0082";
        }

        T_t_ecmarginaccount marginacc;
        memset(&marginacc, 0, sizeof marginacc);
        ret = DB_t_ecmarginaccount_read_lock_by_cur_and_bankcode(
                  crbankcardheader.bankcode, &marginacc);
        if(ret)
        {
            if(DB_NOTFOUND == ret)
            {
                ERRTIP(margin_msg.c_str());
                return E_COMMON_ERR;
            }
            return E_DB_ECMARGINACCOUNT_R;
        }
        if(marginacc.balance - trans.amount < 0)
        {
            ERRTIP(margin_msg.c_str());
            return E_COMMON_ERR;
        }
    }

    //生成交易流水号并记录
    T_t_ecloaddtl ecloaddtl;
    memset(&ecloaddtl, 0, sizeof ecloaddtl);
    des2src(ecloaddtl.accdate, ats.trans.accdate);
    des2src(ecloaddtl.acctime, ats.trans.acctime);
    ecloaddtl.amount = trans.amount;
    ret = GetNewRefno(trans.refno);
    if(ret)
    {
        return ret;
    }
    des2src(ecloaddtl.refno, trans.refno);
    ecloaddtl.cardno = trans.cardno;
    ecloaddtl.custid = trans.custid;
    COL2ST(ecloaddtl, ecbalance);
    des2src(ecloaddtl.stuempno, customer.stuempno);
    des2src(ecloaddtl.cardphyid, trans.cardphyid);
    des2src(ecloaddtl.crbankcardno, crbankcardno.c_str());
    des2src(ecloaddtl.crbankcode, crbankcardheader.bankcode);
    des2src(ecloaddtl.drbankcardno, drbankcardno.c_str());
    des2src(ecloaddtl.drbankcode, drbankcardheader.bankcode);
    ecloaddtl.status = TRANSTATUS_INIT;
    ecloaddtl.termid = trans.termid;
    ret = ats.GetTermSeqno();
    if(ret)
        return ret;

    ecloaddtl.termseqno = trans.termseqno;
    des2src(ecloaddtl.transdate, trans.accdate);
    des2src(ecloaddtl.transtime, trans.acctime);

    ret = DB_t_ecloaddtl_add(&ecloaddtl);
    if(ret)
    {
        LOG(ERROR, "保存流水失败");
        LOG(DEBUG, "保存流水错误，accdate=" << ecloaddtl.accdate << ",refno=" << ecloaddtl.refno);
        return ret;
    }
    ST2ATTR(ecloaddtl, accdate);
    ST2ATTR(ecloaddtl, refno);
    return 0;

}
