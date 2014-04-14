#ifndef  __T_ecaccount_H_
#define  __T_ecaccount_H_
typedef struct 
{
		int	custid;
		char	custname[60+1];
		int	cardno;
		char	ecbankno[20+1];
		int	status;
		int	ecbala;
		char	updbaladate[8+1];
		char	updbalatime[6+1];
		int	consumeamt;
		char	consumeupddate[8+1];
		char	consumeupdtime[6+1];
		int	loadtotalamt;
		int	consumetotalamt;
		char	bankcode[2+1];
}T_t_ecaccount;
int DB_t_ecaccount_add(T_t_ecaccount *pt_ecaccount);
int DB_t_ecaccount_read_by_custid_and_ecbankno(int v_custid,const char *v_ecbankno,T_t_ecaccount *pt_ecaccount);
int DB_t_ecaccount_update_by_custid_and_ecbankno(int v_custid,char *v_ecbankno,T_t_ecaccount *pt_ecaccount);
int DB_t_ecaccount_read_lock_by_cur_and_custid_and_ecbankno(int v_custid,const char *v_ecbankno,T_t_ecaccount *pt_ecaccount);
int DB_t_ecaccount_update_lock_by_cur(T_t_ecaccount *pt_ecaccount);
int DB_t_ecaccount_free_lock_by_cur();
#endif
