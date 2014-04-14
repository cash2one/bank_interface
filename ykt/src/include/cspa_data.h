#pragma(1) 
//对公圈存发送
typedef struct 
{
		char	card_no[19];//卡号
		char	card_seqno[3];//卡序号，前补0
		char	card_expiredate[8];//卡有效期，YYYYMMDD
		char	cash_type[3];//币种,数字类型，固定长
		char	card_bal[12];//卡片余额，9(10)V99<===>0000000012.34   12位,无小数点,前补0
		char	init_amt[12];//圈存金额，9(10)V99<===>0000000012.34   12位,无小数点,前补0
		char	trans_time[10];//传输时间，MMDDhhmmss
		char	sys_trackno[12];//系统标识(２位)+地区码(２位)+交易流水号(8位)
									//传输时间和系统跟踪号唯一确定一笔CSP交易
		char	term_time[6];//终端时间，hhmmss
		char	term_date[4];//终端日期,MMDD
		char	term_no[8];//终端编号，固定长
		char 	term_addr[40];//终端地址，后补空格，固定长
		char	pub_account[17];//对公帐号，前补00000
		char    IC_data[255];//IC数据,ARQC及ARQC成分(TLV)格式，遵循银联规范55域，后补空格，固定长
}csp_send_pub_init;

//对公圈存接收
typedef struct 
{
		char	card_no[19];//卡号
		char	card_seqno[3];//卡序号，前补0
		char	card_expiredate[8];//卡有效期，YYYYMMDD
		char	cash_type[3];//币种,数字类型，固定长
		char	init_amt[12];//圈存金额，9(10)V99<===>0000000012.34   12位,无小数点,前补0
		char	not_register_amt[12];//未登余额，9(10)V99<===>0000000012.34   12位,无小数点,前补0
		char    poundage[9];//手续费,9(7)V99<==>0000012.34  9位，无小数点，前补0
		char	trans_time[10];//传输时间，MMDDhhmmss
		char	sys_trackno[12];//系统标识(２位)+地区码(２位)+交易流水号(8位)
		char	term_time[6];//终端时间，hhmmss
		char	term_date[4];//终端日期,MMDD
		char	term_no[8];//终端编号，固定长
		char	pub_account[17];//对公帐号，前补00000
		char    ICCD_trans_time[14];//ICCD交易时间	,	YYYYMMDDHHMMSS
		char    ICCD_refno[6];//ICCD流水号	X(6)
		char    IC_data[255];//IC数据,ARQC及ARQC成分(TLV)格式，遵循银联规范55域，后补空格，固定长
}csp_recv_pub_init;


//对公圈存冲正发送
typedef struct 
{
		char	card_no[19];//卡号
		char	card_seqno[3];//卡序号，前补0
		char	card_expiredate[8];//卡有效期，YYYYMMDD
		char	cash_type[3];//币种,数字类型，固定长
		char	init_amt[12];//圈存金额，9(10)V99<===>0000000012.34   12位,无小数点,前补0
		char	trans_time[10];//传输时间，MMDDhhmmss
		char	sys_trackno[12];//系统标识(２位)+地区码(２位)+交易流水号(8位)
		char	term_time[6];//终端时间，hhmmss
		char	term_date[4];//终端日期,MMDD
		char	term_no[8];//终端编号，固定长
		char	pub_account[17];//对公帐号，前补00000
		char    raw_trans_info[42];//原始交易传输时间(10)＋原始交易系统跟踪号(12) ，后补空格，固定长
}csp_send_pub_reverse ;


//对公圈存冲正接收
typedef struct 
{
		char	card_no[19];//卡号
		char	card_seqno[3];//卡序号，前补0
		char	card_expiredate[8];//卡有效期，YYYYMMDD
		char	cash_type[3];//币种,数字类型，固定长
		char	init_amt[12];//圈存金额，9(10)V99<===>0000000012.34   12位,无小数点,前补0
		char	trans_time[10];//传输时间，MMDDhhmmss
		char	sys_trackno[12];//系统标识(２位)+地区码(２位)+交易流水号(8位)
		char	term_time[6];//终端时间，hhmmss
		char	term_date[4];//终端日期,MMDD
		char	term_no[8];//终端编号，固定长
		char	pub_account[17];//对公帐号，前补00000
		char    ICCD_trans_time[14];//ICCD交易时间	,	YYYYMMDDHHMMSS
		char    ICCD_refno[6];//ICCD流水号	X(6)
}csp_recv_pub_reverse;

#pragma()
