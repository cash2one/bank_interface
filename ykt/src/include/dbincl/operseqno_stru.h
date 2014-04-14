#ifndef  __T_operseqno_H_
#define  __T_operseqno_H_
typedef struct 
{
		int	operid;
		int	operseqno;
		int	checknum;
		char	updtime[14+1];
}T_t_operseqno;
int DB_t_operseqno_add(T_t_operseqno *pt_operseqno);
int DB_t_operseqno_read_by_operid(int v_operid,T_t_operseqno *pt_operseqno);
int DB_t_operseqno_read_lock_by_c0_and_operid(int v_operid,T_t_operseqno *pt_operseqno);
int DB_t_operseqno_update_lock_by_c0(T_t_operseqno *pt_operseqno);
int DB_t_operseqno_free_lock_by_c0();
#endif
