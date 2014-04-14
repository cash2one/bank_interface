#ifndef  __T_eccashdtl_H_
#define  __T_eccashdtl_H_
typedef struct 
{
		char	accdate[8+1];
		char	refno[20+1];
		char	opercode[8+1];
		int	amount;
		char	crbankcode[10+1];
}T_t_eccashdtl;
int DB_t_eccashdtl_add(T_t_eccashdtl *pt_eccashdtl);
int DB_t_eccashdtl_read_by_accdate_and_refno(const char *v_accdate,const char *v_refno,T_t_eccashdtl *pt_eccashdtl);
int DB_t_eccashdtl_update_by_accdate_and_refno(char *v_accdate,char *v_refno,T_t_eccashdtl *pt_eccashdtl);
int DB_t_eccashdtl_read_lock_by_cur_and_accdate_and_refno(const char *v_accdate,const char *v_refno,T_t_eccashdtl *pt_eccashdtl);
int DB_t_eccashdtl_update_lock_by_cur(T_t_eccashdtl *pt_eccashdtl);
int DB_t_eccashdtl_free_lock_by_cur();
#endif
