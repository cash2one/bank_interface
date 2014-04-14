#ifndef  __T_operdtl_H_
#define  __T_operdtl_H_
typedef struct 
{
		int	id;
		char	hostdate[8+1];
		char	hosttime[6+1];
		int	operid;
		int	operseqno;
		char	refno[20+1];
		int	transcode;
		int	termid;
		char	termdate[8+1];
		char	termtime[6+1];
		int	chkoperid;
		int	status;
		char	transinfo[480+1];
		char	reqdata[1000+1];
}T_t_operdtl;
int DB_t_operdtl_add(T_t_operdtl *pt_operdtl);
int DB_t_operdtl_read_by_operid_and_operseqno(int v_operid,int v_operseqno,T_t_operdtl *pt_operdtl);
int DB_t_operdtl_del_by_id(int v_id);
int DB_t_operdtl_read_lock_by_c0_and_id(int v_id,T_t_operdtl *pt_operdtl);
int DB_t_operdtl_update_lock_by_c0(T_t_operdtl *pt_operdtl);
int DB_t_operdtl_del_lock_by_c0();
int DB_t_operdtl_free_lock_by_c0();
#endif
