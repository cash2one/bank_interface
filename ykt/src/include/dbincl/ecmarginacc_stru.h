#ifndef  __T_ecmarginacc_H_
#define  __T_ecmarginacc_H_
typedef struct 
{
		char	bankcode[2+1];
		int	initialmargin;
		int	balance;
		char	lastupdtime[14+1];
}T_t_ecmarginaccount;
int DB_t_ecmarginaccount_add(T_t_ecmarginaccount *pt_ecmarginaccount);
int DB_t_ecmarginaccount_update_by_bankcode(char *v_bankcode,T_t_ecmarginaccount *pt_ecmarginaccount);
int DB_t_ecmarginaccount_read_lock_by_cur_and_bankcode(const char *v_bankcode,T_t_ecmarginaccount *pt_ecmarginaccount);
int DB_t_ecmarginaccount_update_lock_by_cur(T_t_ecmarginaccount *pt_ecmarginaccount);
int DB_t_ecmarginaccount_free_lock_by_cur();
#endif
