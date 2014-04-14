#ifndef  __T_dpsmac_H_
#define  __T_dpsmac_H_
typedef struct 
{
		char	refno[20+1];
		char	accdate[8+1];
		int	termid;
		int	termseqno;
		int	transcode;
		char	cardasno[20+1];
		int	cardno;
		int	dpscnt;
		int	paycnt;
		int	cardbefbal;
		int	amount;
		int	dpsamt;
		int	cardaftbal;
		char	hostdate[8+1];
		char	hosttime[6+1];
		char	samno[12+1];
		char	randnum[8+1];
		char	mac1[8+1];
		char	mac2[8+1];
		char	tac[8+1];
		int	confirmed;
		int	isok;
		int	isbreak;
		char	updtime[14+1];
}T_t_dpsmac;
int DB_t_dpsmac_add(T_t_dpsmac *pt_dpsmac);
int DB_t_dpsmac_read_by_refno(const char *v_refno,T_t_dpsmac *pt_dpsmac);
int DB_t_dpsmac_read_by_accdate_and_refno(const char *v_accdate,const char *v_refno,T_t_dpsmac *pt_dpsmac);
int DB_t_dpsmac_update_by_accdate_and_refno(char *v_accdate,char *v_refno,T_t_dpsmac *pt_dpsmac);
int DB_t_dpsmac_del_by_accdate_and_refno(const char *v_accdate,const char *v_refno);
int DB_t_dpsmac_read_lock_by_c0_and_accdate_and_refno(const char *v_accdate,const char *v_refno,T_t_dpsmac *pt_dpsmac);
int DB_t_dpsmac_update_lock_by_c0(T_t_dpsmac *pt_dpsmac);
int DB_t_dpsmac_del_lock_by_c0();
int DB_t_dpsmac_free_lock_by_c0();
#endif
