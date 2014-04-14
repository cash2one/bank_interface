/* --------------------------------------------
 * ��������: 2010-09-06
 * ��������: �Ž�
 * �汾��Ϣ: 3.0.0.0
 * ������: ��״̬У��
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

int GetCardStatusCode(int cardno,char* statuscode,char *statusinfo)
{
	*statuscode=0;
	T_t_card  tCard;
	memset(&tCard,0,sizeof(tCard));
	int ret=DB_t_card_read_by_cardno(cardno,&tCard);
	if(ret)
	{
		LOG(ERROR,"cardno["<<cardno<<"]");
		*statuscode=CARDSTATUS_EXCEPT;
		return 0;
	}
	if(tCard.status[0]!=STATUS_NORMAL)
	{
		strcpy(statusinfo,"����ע��");
		*statuscode=CARDSTATUS_CLOSE;
		return 0;
	}	
	if('1'==tCard.lockflag[0])
	{
		strcpy(statusinfo,"��������");
		*statuscode=CARDSTATUS_LOCK;
		return 0;
	}	
	if('1'==tCard.lossflag[0])
	{
		strcpy(statusinfo,"���ѹ�ʧ");
		*statuscode=CARDSTATUS_LOST;
		return 0;
	}	
	if('1'==tCard.frozeflag[0])
	{
		strcpy(statusinfo,"���Ѷ���");
		*statuscode=CARDSTATUS_FROZE;
		return 0;
	}
	CAccTrans& ats=CAccTrans::GetInst();
	TRANS& trans=ats.trans;
	if(strncmp(tCard.expiredate,trans.transdate,8)<0)
	{
		strcpy(statusinfo,"���ѹ���");
		*statuscode=CARDSTATUS_EXPIRE;
		return 0;
	}	
	if('1'==tCard.badflag[0])
	{
		strcpy(statusinfo,"���ѵǼ�Ϊ����");
		*statuscode=CARDSTATUS_EXCEPT;
		return 0;
	}	
	return 0;
}
FUNCTION(200000)
{
	int ret=0;
	CAccTrans& ats=CAccTrans::GetInst();
	TRANS& trans=ats.trans;

	COL2ST(trans,cardno);
	if(trans.cardno<1)
	{
		ERRTIP("���Ų���Ϊ��");
		return E_COMMON_ERR;
	}
	ST2COL(trans,summary);
	if(ret)
	{
		LOG(ERROR,"GetCardStatusCode ret["<<ret<<"]cardno["<<trans.cardno<<"]");
		return ret;
	}
	gWriter.addRow();
	return 0;
}

