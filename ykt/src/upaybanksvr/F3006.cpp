/* --------------------------------------------
 * 创建日期: 2011-11-21
 * 程序作者: 汤成
 * 版本信息: 3.0.0.0
 * 程序功能: 现金-电子现金确认
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

static int do_process(T_t_ecloaddtl& ecloaddtl)
{
    ec_bank_trans* comm = ec_bank_trans_load::instance()->get_transfer(
                              ecloaddtl.drbankcode, ecloaddtl.crbankcode);
    if (!comm)
    {
        ERRTIP("不支持转账模式");
        return E_COMMON_ERR;
    }
    ec_bank_trans_param param;
    string balalimit, limitperload, balacycle, expiredate;
    COL2VAR(balalimit);
    COL2VAR(limitperload);
    COL2VAR(balacycle);
    COL2VAR(expiredate);
    param["balalimit"] = balalimit;
    param["limitperload"] = limitperload;
    param["balacycle"] = balacycle;
    param["expiredate"] = expiredate;
    // 检查限额判断

    int ret = comm->transfer(ecloaddtl , param);
    switch (ret)
    {
    case ec_bank_trans::r_recv:
    case ec_bank_trans::r_send:
        ecloaddtl.status = 9;
        break;
    case ec_bank_trans::r_ok:
        ecloaddtl.status = TRANSTATUS_SUCC;
        break;
    default:
        ecloaddtl.status = TRANSTATUS_EXCEPT;
        break;
    }
    if (ecloaddtl.status == TRANSTATUS_SUCC)
    {
        T_t_ecaccount ecaccount;
        memset(&ecaccount, 0 ,sizeof ecaccount);
        ret = ec_update_account(&ecloaddtl, &ecaccount);
        if(ret)
        {
            return ret;
        }
        if(comm->is_use_margin_acc())
        {
            ret = ec_update_marginacc(&ecloaddtl);
            if(ret)
            {
                return ret;
            }
        }
        // 记录操作员现金流水错误现金流水
        T_t_eccashdtl cashdtl;
        memset(&cashdtl, 0, sizeof cashdtl);
        des2src(cashdtl.accdate, ecloaddtl.accdate);
        des2src(cashdtl.refno, ecloaddtl.refno);
        char opercode[9] = {0};
        ATTR2VAR(opercode);
        des2src(cashdtl.opercode, opercode);
        char voucherno[33];
        COL2VAR(voucherno);
        des2src(cashdtl.voucherno, voucherno);
        des2src(cashdtl.crbankcode, ecloaddtl.crbankcode);
        cashdtl.amount = ecloaddtl.amount;
        ret = DB_t_eccashdtl_add(&cashdtl);
        if (ret)
        {
            if (DB_REPEAT != ret)
            {
                // 记录错误，但是还是要继续交易
                LOG(ERROR, "记录操作员现金流水错误");
                ecloaddtl.paystatus = EC_PAYSTATUS_BROKEN;
            }
            else
                ecloaddtl.paystatus = EC_PAYSTATUS_OK;
        }
        else
        {
            ecloaddtl.paystatus = EC_PAYSTATUS_OK;
        }
    }
    ret = DB_t_ecloaddtl_update_lock_by_cur(&ecloaddtl);
    if (ret)
    {
        LOG(ERROR, "更新流水数据失败");
        return E_DB_BANK_U;
    }
    if (ecloaddtl.status == TRANSTATUS_SUCC)
    {
        ST2ATTR(ecloaddtl, accdate);
        ST2ATTR(ecloaddtl, refno);
        ST2COL(ecloaddtl, field55);
        gWriter.addRow();
        return 0;
    }
    else
    {
        db_commit();
        ERRTIP(ecloaddtl.errmsg);
        return E_COMMON_ERR;
    }
    return E_COMMON_ERR;
}

FUNCTION(3006)
{
    int ret = 0;
    CAccTrans& ats = CAccTrans::GetInst();

    TRANS& trans = ats.trans;
    ATTR2ST(trans, termid);
    char accdate[9] = {0}, refno[21] = {0};
    ATTR2VAR(accdate);
    ATTR2VAR(refno);

    gReader.fetchRow();

    std::string field55;
    COL2VAR(field55);

    T_t_ecloaddtl ecloaddtl;
    memset(&ecloaddtl, 0, sizeof ecloaddtl);
    ret = DB_t_ecloaddtl_read_lock_by_cur_and_accdate_and_refno(
              accdate, refno , &ecloaddtl);
    if (ret)
    {
        LOG(ERROR, "DB_t_ecloaddtl_read_lock_by_cur_and_accdate_and_refno error,\
            ret=" << ret);
        if (DB_NOTFOUND == ret)
        {
            return E_DB_TRANSDTL_N;
        }
        else
            return E_DB_TRANSDTL_R;
    }
    if (ecloaddtl.status != TRANSTATUS_INIT)
    {
        if (ecloaddtl.status == TRANSTATUS_SUCC)
        {
            ST2ATTR(ecloaddtl, accdate);
            ST2ATTR(ecloaddtl, refno);
            ST2COL(ecloaddtl, field55);
            gWriter.addRow();
            return 0;
        }
        LOG(ERROR, "ecloaddtl is error");
        ERRTIP(ecloaddtl.errmsg);
        return E_COMMON_ERR;
    }

    des2src(ecloaddtl.field55, field55.c_str());
    ret = do_process(ecloaddtl);
    if (ret)
    {
        //DB_t_ecloaddtl_free_lock_by_cur();
        LOG(ERROR, "do_process error,ret=" << ret);
    }
    else
    {
        LOG(DEBUG, "交易成功...");
    }
    return ret;

}
