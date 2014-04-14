#ifndef  __T_bankcardheader_H_
#define  __T_bankcardheader_H_
typedef struct 
{
		char	cardheader[20+1];
		char	bankcode[2+1];
}T_t_bankcardheader;
int DB_t_bankcardheader_add(T_t_bankcardheader *pt_bankcardheader);
int DB_t_bankcardheader_read_by_cardheader(const char *v_cardheader,T_t_bankcardheader *pt_bankcardheader);
int DB_t_bankcardheader_del_by_cardheader(const char *v_cardheader);
int DB_t_bankcardheader_update_by_cardheader(char *v_cardheader,T_t_bankcardheader *pt_bankcardheader);
int DB_t_bankcardheader_read_lock_by_cur_and_cardheader(const char *v_cardheader,T_t_bankcardheader *pt_bankcardheader);
int DB_t_bankcardheader_update_lock_by_cur(T_t_bankcardheader *pt_bankcardheader);
int DB_t_bankcardheader_free_lock_by_cur();
int DB_t_bankcardheader_open_select_by_c1();
int DB_t_bankcardheader_fetch_select_by_c1(T_t_bankcardheader *pt_bankcardheader);
int DB_t_bankcardheader_close_select_by_c1();
#endif
