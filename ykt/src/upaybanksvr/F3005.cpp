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
        LOG(ERROR, "db cardphyid[" << tCard.cardphyid
            << "],input cardphyid[" << trans.cardphyid << "]");
        return E_CARD_PHYNO_DIFFER;
    }
    //检查卡的有效期
    if(strcmp(tCard.expiredate, trans.transdate) <= 0)
        return E_CARD_EXPIRED;
    return 0;
}

FUNCTION(3005)
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

    std::string crbankcardno, drbankcardno;
    COL2VAR(crbankcardno);
    COL2VAR(drbankcardno);

    //检查金额的合法性
    // if (trans.amount < 100 || trans.amount % 100 != 0)
    // 中银通的圈存补登业务需要发起金额为 0 的交易
    if(trans.amount < 0)
    {
        ERRTIP("输入金额不合法");
        return E_COMMON_ERR;
    }

    //检查终端合法性

    //检查卡片合法性
    T_t_card card;
    memset(&card, 0, sizeof card);
    T_t_customer customer;
    memset(&customer, 0, sizeof customer);
    card.cardno = trans.cardno;
    // 判断交易卡号的合法性
    if(card.cardno > 0)
    {
        ret = doCard(trans, card);
        if(ret)
        {
            return ret;
        }
        LOG(DEBUG, "电子现金初始化，custid=" << card.custid << ",cardno=" << card.cardno);

        ret = DB_t_customer_read_by_custid(card.custid, &customer);
        if(ret)
        {
            if(ret)
            {
                return E_DB_CUSTOMER_N;
            }
            return E_DB_CUSTOMER_R;
        }
        trans.custid = card.custid;
    }
    //检查交易时间段
    T_t_bankcard crbankcard;
    memset(&crbankcard, 0, sizeof crbankcard);

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
                              CASH_CODE, crbankcardheader.bankcode);
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
    des2src(ecloaddtl.drbankcode, CASH_CODE);
    // des2src(ecloaddtl.drbankcardno, drbankcardno.c_str());
    // des2src(ecloaddtl.drbankcode, drbankcardheader.bankcode);
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
        LOG(DEBUG, "保存流水错误，accdate=" << ecloaddtl.accdate
            << ",refno=" << ecloaddtl.refno);
        return ret;
    }
    ST2ATTR(ecloaddtl, accdate);
    ST2ATTR(ecloaddtl, refno);
    return 0;

}
