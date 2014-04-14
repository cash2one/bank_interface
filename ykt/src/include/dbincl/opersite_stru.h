#ifndef  __T_opersite_H_
#define  __T_opersite_H_
typedef struct 
{
		int	operid;
		int	siteno;
		char	lastsaved[30+1];
}T_t_opersite;
int DB_t_opersite_add(T_t_opersite *pt_opersite);
int DB_t_opersite_read_by_operid_and_siteno(int v_operid,int v_siteno,T_t_opersite *pt_opersite);
int DB_t_opersite_update_by_operid_and_siteno(int v_operid,int v_siteno,T_t_opersite *pt_opersite);
int DB_t_opersite_del_by_operid_and_siteno(int v_operid,int v_siteno);
int DB_t_opersite_read_lock_by_c0_and_operid_and_siteno(int v_operid,int v_siteno,T_t_opersite *pt_opersite);
int DB_t_opersite_update_lock_by_c0(T_t_opersite *pt_opersite);
int DB_t_opersite_free_lock_by_c0();
#endif
