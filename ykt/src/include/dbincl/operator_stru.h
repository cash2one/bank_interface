#ifndef  __T_operator_H_
#define  __T_operator_H_
typedef struct 
{
		int	operid;
		char	opercode[30+1];
		int	operseqno;
		int	checknum;
		int	opertype;
		int	branchno;
		int	cardno;
		char	opername[60+1];
		char	tel[30+1];
		char	mobile[30+1];
		char	email[60+1];
		char	addr[240+1];
		char	zipcode[6+1];
		char	operpwd[32+1];
		int	status;
		char	lasttransdate[8+1];
		char	menuset[800+1];
		char	funcset[600+1];
		int	sitelimit;
		int	loginflag;
		int	loginsiteno;
		char	logintime[30+1];
		char	logouttime[30+1];
		int	failnumber;
		char	opendate[8+1];
		char	closedate[8+1];
		int	rightflag;
		char	progname[90+1];
		char	progverno[60+1];
		char	ip[15+1];
		char	macaddr[17+1];
}T_t_operator;
int DB_t_operator_add(T_t_operator *pt_operator);
int DB_t_operator_read_by_opercode(const char *v_opercode,T_t_operator *pt_operator);
int DB_t_operator_update_by_opercode(char *v_opercode,T_t_operator *pt_operator);
int DB_t_operator_del_by_opercode(const char *v_opercode);
int DB_t_operator_read_lock_by_c0_and_opercode(const char *v_opercode,T_t_operator *pt_operator);
int DB_t_operator_update_lock_by_c0(T_t_operator *pt_operator);
int DB_t_operator_free_lock_by_c0();
int DB_t_operator_read_by_operid(int v_operid,T_t_operator *pt_operator);
int DB_t_operator_read_lock_by_c1_and_operid(int v_operid,T_t_operator *pt_operator);
int DB_t_operator_update_lock_by_c1(T_t_operator *pt_operator);
int DB_t_operator_free_lock_by_c1();
#endif
