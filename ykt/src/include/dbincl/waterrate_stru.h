#ifndef  __T_waterrate_H_
#define  __T_waterrate_H_
typedef struct 
{
		int	feetype;
		int	rate1;
		int	rate2;
		int	rate3;
		char	lastsaved[30+1];
}T_t_waterrate;
int DB_t_waterrate_add(T_t_waterrate *pt_waterrate);
int DB_t_waterrate_read_by_feetype(int v_feetype,T_t_waterrate *pt_waterrate);
int DB_t_waterrate_update_by_feetype(int v_feetype,T_t_waterrate *pt_waterrate);
int DB_t_waterrate_del_by_feetype(int v_feetype);
int DB_t_waterrate_read_lock_by_c0_and_feetype(int v_feetype,T_t_waterrate *pt_waterrate);
int DB_t_waterrate_update_lock_by_c0(T_t_waterrate *pt_waterrate);
int DB_t_waterrate_del_lock_by_c0();
int DB_t_waterrate_free_lock_by_c0();
#endif
