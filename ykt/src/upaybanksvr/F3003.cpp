/* --------------------------------------------
 * 创建日期: 2011-11-21
 * 程序作者: 汤成
 * 版本信息: 3.0.0.0
 * 程序功能: 圈存写卡确认
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

FUNCTION(3003)
{
    T_t_ecloaddtl ecloaddtl;
    int ret;

    char accdate[9], refno[21];
    memset(accdate, 0, sizeof accdate);
    memset(refno, 0, sizeof refno);
    ATTR2VAR(accdate);
    ATTR2VAR(refno);

    memset(&ecloaddtl, 0, sizeof ecloaddtl);
    ret = DB_t_ecloaddtl_read_lock_by_cur_and_accdate_and_refno(accdate, refno, &ecloaddtl);
    if (ret)
    {
        LOG(ERROR, "DB_t_ecloaddtl_read_lock_by_cur_and_accdate_and_refno error,ret=" << ret);
        if (DB_NOTFOUND == ret)
        {
            return E_DB_TRANSDTL_N;
        }
        else
            return E_DB_TRANSDTL_R;
    }
    if (ecloaddtl.status != TRANSTATUS_SUCC)
    {
        ERRTIP("流水状态不正确");
        DB_t_ecloaddtl_free_lock_by_cur();
        return  E_COMMON_ERR;
    }
    if (ecloaddtl.writecardflag != 0)
    {
    if(ecloaddtl.writecardflag == 1)
    	{
        ERRTIP("写卡标志已确认");
        DB_t_ecloaddtl_free_lock_by_cur();
        return  E_COMMON_ERR;
    	}
    }
    int writecard;
    gReader.fetchRow();
    COL2VAR(writecard);
    ecloaddtl.writecardflag = writecard;

    ret = DB_t_ecloaddtl_update_lock_by_cur(&ecloaddtl);
    if (ret)
    {
        return E_DB_TRANSDTL_U;
    }

    ec_bank_trans* comm = ec_bank_trans_load::instance()->get_transfer(
                              ecloaddtl.drbankcode, ecloaddtl.crbankcode);
    if (!comm)
    {
        ERRTIP("不支持转账模式");
        return E_COMMON_ERR;
    }
    ec_bank_trans_param param;
    string balalimit, limitperload, balacycle, expiredate, ac;
    COL2VAR(balalimit);
    COL2VAR(limitperload);
    COL2VAR(balacycle);
    COL2VAR(expiredate);
    COL2VAR(ac);
    param["balalimit"] = balalimit;
    param["limitperload"] = limitperload;
    param["balacycle"] = balacycle;
    param["expiredate"] = expiredate;
    param["ic_result"] = ac;

    CAccTrans& ats = CAccTrans::GetInst();
    ats.trans.termid = ecloaddtl.termid;
    ret = ats.GetTermSeqno();
    if (ret)
        return ret;
    {
        stringstream ss;
        ss << std::setw(6) << std::setfill('0') << ats.trans.termseqno;
        param["seqno"] = ss.str();
    }
    ret = db_commit();
    if (ret)
    {
        LOG(ERROR, "提交事务失败!");
        return E_COMMON_ERR;
    }
    int flag = (ecloaddtl.writecardflag == 1) ? ec_bank_trans::wc_success : ec_bank_trans::wc_failed;
    ret = comm->post_transfer(ecloaddtl , param , flag);
    if (ret)
    {
        //ERRTIP("发送给银行写卡确认失败");
        return E_COMMON_ERR;
    }
    VAR2ATTR(accdate);
    VAR2ATTR(refno);
    return 0;
}
