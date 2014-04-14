#ifndef  __T_ecloaddtl_H_
#define  __T_ecloaddtl_H_
typedef struct 
{
		char	accdate[8+1];
		char	refno[20+1];
		char	acctime[6+1];
		int	termid;
		int	termseqno;
		char	transdate[8+1];
		char	transtime[6+1];
		int	cardno;
		int	custid;
		char	cardphyid[16+1];
		char	stuempno[20+1];
		char	custname[20+1];
		int	amount;
		int	ecbalance;
		char	drbankcardno[20+1];
		char	crbankcardno[20+1];
		char	drbankcode[2+1];
		char	crbankcode[2+1];
		int	status;
		int	revflag;
		int	writecardflag;
		int	drchkflag;
		char	drchktime[14+1];
		char	drrefno[30+1];
		int	crchkflag;
		char	crchktime[14+1];
		char	crrefno[30+1];
		int	paystatus;
		char	payrefno[20+1];
		char	paymac[8+1];
		int	errcode;
		char	errmsg[256+1];
		char	field55[1000+1];
}T_t_ecloaddtl;
int DB_t_ecloaddtl_add(T_t_ecloaddtl *pt_ecloaddtl);
int DB_t_ecloaddtl_read_by_accdate_and_refno(const char *v_accdate,const char *v_refno,T_t_ecloaddtl *pt_ecloaddtl);
int DB_t_ecloaddtl_update_by_accdate_and_refno(char *v_accdate,char *v_refno,T_t_ecloaddtl *pt_ecloaddtl);
int DB_t_ecloaddtl_read_lock_by_cur_and_accdate_and_refno(const char *v_accdate,const char *v_refno,T_t_ecloaddtl *pt_ecloaddtl);
int DB_t_ecloaddtl_update_lock_by_cur(T_t_ecloaddtl *pt_ecloaddtl);
int DB_t_ecloaddtl_free_lock_by_cur();
#endif
