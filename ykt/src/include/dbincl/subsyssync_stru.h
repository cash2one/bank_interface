#ifndef  __T_subsyssync_H_
#define  __T_subsyssync_H_
typedef struct 
{
		int	sysid;
		int	synctype;
		char	sysverno[14+1];
}T_t_subsyssync;
int DB_t_subsyssync_add(T_t_subsyssync *pt_subsyssync);
int DB_t_subsyssync_read_by_sysid_and_synctype(int v_sysid,int v_synctype,T_t_subsyssync *pt_subsyssync);
int DB_t_subsyssync_update_by_sysid_and_synctype(int v_sysid,int v_synctype,T_t_subsyssync *pt_subsyssync);
int DB_t_subsyssync_del_by_sysid(int v_sysid);
int DB_t_subsyssync_read_lock_by_c0_and_sysid_and_synctype(int v_sysid,int v_synctype,T_t_subsyssync *pt_subsyssync);
int DB_t_subsyssync_update_lock_by_c0(T_t_subsyssync *pt_subsyssync);
int DB_t_subsyssync_free_lock_by_c0();
#endif
