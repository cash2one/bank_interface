#pragma(1) 
//�Թ�Ȧ�淢��
typedef struct 
{
		char	card_no[19];//����
		char	card_seqno[3];//����ţ�ǰ��0
		char	card_expiredate[8];//����Ч�ڣ�YYYYMMDD
		char	cash_type[3];//����,�������ͣ��̶���
		char	card_bal[12];//��Ƭ��9(10)V99<===>0000000012.34   12λ,��С����,ǰ��0
		char	init_amt[12];//Ȧ���9(10)V99<===>0000000012.34   12λ,��С����,ǰ��0
		char	trans_time[10];//����ʱ�䣬MMDDhhmmss
		char	sys_trackno[12];//ϵͳ��ʶ(��λ)+������(��λ)+������ˮ��(8λ)
									//����ʱ���ϵͳ���ٺ�Ψһȷ��һ��CSP����
		char	term_time[6];//�ն�ʱ�䣬hhmmss
		char	term_date[4];//�ն�����,MMDD
		char	term_no[8];//�ն˱�ţ��̶���
		char 	term_addr[40];//�ն˵�ַ���󲹿ո񣬹̶���
		char	pub_account[17];//�Թ��ʺţ�ǰ��00000
		char    IC_data[255];//IC����,ARQC��ARQC�ɷ�(TLV)��ʽ����ѭ�����淶55�򣬺󲹿ո񣬹̶���
}csp_send_pub_init;

//�Թ�Ȧ�����
typedef struct 
{
		char	card_no[19];//����
		char	card_seqno[3];//����ţ�ǰ��0
		char	card_expiredate[8];//����Ч�ڣ�YYYYMMDD
		char	cash_type[3];//����,�������ͣ��̶���
		char	init_amt[12];//Ȧ���9(10)V99<===>0000000012.34   12λ,��С����,ǰ��0
		char	not_register_amt[12];//δ����9(10)V99<===>0000000012.34   12λ,��С����,ǰ��0
		char    poundage[9];//������,9(7)V99<==>0000012.34  9λ����С���㣬ǰ��0
		char	trans_time[10];//����ʱ�䣬MMDDhhmmss
		char	sys_trackno[12];//ϵͳ��ʶ(��λ)+������(��λ)+������ˮ��(8λ)
		char	term_time[6];//�ն�ʱ�䣬hhmmss
		char	term_date[4];//�ն�����,MMDD
		char	term_no[8];//�ն˱�ţ��̶���
		char	pub_account[17];//�Թ��ʺţ�ǰ��00000
		char    ICCD_trans_time[14];//ICCD����ʱ��	,	YYYYMMDDHHMMSS
		char    ICCD_refno[6];//ICCD��ˮ��	X(6)
		char    IC_data[255];//IC����,ARQC��ARQC�ɷ�(TLV)��ʽ����ѭ�����淶55�򣬺󲹿ո񣬹̶���
}csp_recv_pub_init;


//�Թ�Ȧ���������
typedef struct 
{
		char	card_no[19];//����
		char	card_seqno[3];//����ţ�ǰ��0
		char	card_expiredate[8];//����Ч�ڣ�YYYYMMDD
		char	cash_type[3];//����,�������ͣ��̶���
		char	init_amt[12];//Ȧ���9(10)V99<===>0000000012.34   12λ,��С����,ǰ��0
		char	trans_time[10];//����ʱ�䣬MMDDhhmmss
		char	sys_trackno[12];//ϵͳ��ʶ(��λ)+������(��λ)+������ˮ��(8λ)
		char	term_time[6];//�ն�ʱ�䣬hhmmss
		char	term_date[4];//�ն�����,MMDD
		char	term_no[8];//�ն˱�ţ��̶���
		char	pub_account[17];//�Թ��ʺţ�ǰ��00000
		char    raw_trans_info[42];//ԭʼ���״���ʱ��(10)��ԭʼ����ϵͳ���ٺ�(12) ���󲹿ո񣬹̶���
}csp_send_pub_reverse ;


//�Թ�Ȧ���������
typedef struct 
{
		char	card_no[19];//����
		char	card_seqno[3];//����ţ�ǰ��0
		char	card_expiredate[8];//����Ч�ڣ�YYYYMMDD
		char	cash_type[3];//����,�������ͣ��̶���
		char	init_amt[12];//Ȧ���9(10)V99<===>0000000012.34   12λ,��С����,ǰ��0
		char	trans_time[10];//����ʱ�䣬MMDDhhmmss
		char	sys_trackno[12];//ϵͳ��ʶ(��λ)+������(��λ)+������ˮ��(8λ)
		char	term_time[6];//�ն�ʱ�䣬hhmmss
		char	term_date[4];//�ն�����,MMDD
		char	term_no[8];//�ն˱�ţ��̶���
		char	pub_account[17];//�Թ��ʺţ�ǰ��00000
		char    ICCD_trans_time[14];//ICCD����ʱ��	,	YYYYMMDDHHMMSS
		char    ICCD_refno[6];//ICCD��ˮ��	X(6)
}csp_recv_pub_reverse;

#pragma()
