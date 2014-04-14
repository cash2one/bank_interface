#ifndef CLSOPER_H_
#define CLSOPER_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "errdef.h"
#include "pubdb.h"
#include "pubdef.h"
#include "pubfunc.h"
#include "dbfunc.h"

namespace ekingstar
{
  typedef struct
  {
	int  branchno;	//操作员网点号
  	int	 operid;
    int  operseqno; 	//操作员流水号
    char opercode[9];	//操作员号
    int  transcode; 	//交易码
    char refno[21]; 	//交易参考号
    int  checknum;		//校验和
    int  revflag;		//0-正常交易1-冲正交易
    int  termid;		//终端编号
    int  termsno;		//终端发送方流水号
    int  termseqno; 	//终端主机流水号
	char termdate[9];
	char termtime[7];
    char reqflag[2];
    char chkopercode[9];//审核操作员
    int  offlineflag;	//是否联机交易1-脱机交易0-联机交易
    int  usecardflag;	//1与卡有关系的交易0-与卡无关系的交易
    int  cardtype;		//卡类型
    int  cardphytype;	//卡物理类型
    int  cardphytypecode;//卡物理类型代码
    char cardphyid[17]; //物理卡号
    char cardpwd[9];	//卡密码
    char expiredate[9];	//有效期
    int  cardno;		//卡号
    int  amount;		//总金额
    int  dpscnt;		//卡充值次数
    int  paycnt;		//卡消费次数
    int  aftdpscnt; 	//卡充值后次数
    int  aftpaycnt; 	//卡消费后次数
    int  transflag; 	//次数类型，1充值2消费
    int  dpsamt;		//充值金额
    int  feeamt;
    int  cardbefbal;	//交易前卡余额
    int  cardaftbal;	//交易后卡余额
    char showcardno[11];//显示卡号
    int  feetype;		//卡收费类型
    int  custid;		//客户号
    char custname[61];	//客户名
    char stuempno[21];	//学工号
    char deptcode[31];	//部门号
    int  shopid;		//商户号
    char cardaccno[11]; //卡帐户号
    char eaccno[11];	//电子钱包账号
    char shopaccno[11]; //商户账号
    char draccno[21];	//输入值:借方账号
    char craccno[21];	//输入值:贷方账号
    int  fundtype;		//资金类型 1-现金2-支票3-经费本4-个人帐户存款5-银行存款
    char voucherno[21]; //凭证号
    char drsubjno[21];	//输出值:借方科目
    char crsubjno[21];	//输出值:贷方科目
    char draccname[61]; //输出值:借方账户名
    char craccname[61]; //输出值:贷方账户名
    int  transtype; 		//交易类型
    int  subseqno;			//子流水号
    double	inputamt;		//输入金额
    double	transamt;		//交易金额
    double	addamt; 		//补写金额
    double	unusedamt;		//剩余金额
    double	totalfeeamt;	//总收费金额(输出)
    double	totaltransamt;	//总交易金额(输出)
    double	forgift;		//押金金额
    char summary[61];		//摘要
    int  errcode;			//错误码
    char remark[241];		//备注
  } REQPACK;
  class COper
  {
  private:
    T_t_operator oper;
    bool bLock;
    bool bLoad;
//    const TRANS& m_req;	
	T_t_operdtl  m_operdtl;
  public:  	
	COper();
    ~COper();
    int loadOper(bool update = false);
    T_t_operator& getOper();
	void attach(const REQPACK& req);
    int getOperid();
    int check(bool unlimited = false);
    int update();
	int setOffline();
    int confirmOK();
    int confirmCancel();
    int confirmFailed();
	int  saveOperdtl(int transstatus=TRANSTATUS_INIT);
	int findOrgiOperdtl();	
	T_t_operdtl& getOperdtl();
  };
}
#endif
