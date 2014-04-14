#ifndef unionpaypos_h_
#define unionpaypos_h_
#include "8583_impl.h"
#include "ks_8583_reader.h"
#include "tcp.h"
#pragma  pack(1)
struct  POSDTLFILE
{
  char cardno[19];               //0	19	主账号
  char transamt[12];            //19	12	交易金额
  char currency[3];             //31	3	交易货币代码
  char transtime[10];           //34	10	交易时间
  char termseqno[6];           //44	6	系统跟踪号
  char authidresp[6];           //50	6	授权应答标识码
  char authdate[4];             //56	4	授权日期
  char refno[12];               //60	12	检索参考号
  char agencycode[11];          ///72	11	代理机构标识码
  char sendercode[11];          //83	11	发送机构标识码
  char merchtype[4];            //94	4	商户类型
  char termno[8];               //98	8	受卡机终端标识码
  char merchno[15];             //106	15	受卡方标识码
  char merchname[40];        //121	40	受卡方名称地址
  char orgitransinfo[23];       //161	23	原始交易信息
  char msgresoncode[4];         //184	4	报文原因代码
  char infoflag[1];             //188	1	单双信息标志
  char cpusseqno[9];            //189	9	CUPS流水号
  char receivercode[11];        //198	11	接收机构代码
  char issuercode[11];          //209	11	发卡机构代码
  char cupsnoticeflag[1];       //220	1	CUPS通知标志
  char transstartway[2];        //221	2	交易发起渠道
  char transtypeid[1];          //223	1	交易特征标识
  char cupsreserved[8];         //224	8	CUPS保留使用
  char possvrpointcode[2];      //232	2	POS服务点条件代码
  char selffeeamt[12];          //234	12	本方手续费
  char transarea[1];            //246	1	交易地域
  char eciflag[2];              //247	2	ECI标志
  char specialfeeflag[2];       //249	2	特殊计费标志
  char specialfeetype[1];       //251	1	特殊计费档次
  char transstarttype[1];       //252	1	交易发起方式
  char reserved[9];             //253	9	保留使用
  char ciphertext[16];       //262	16	应用密文
  char svrpointinputcode[3];    //278	3	服务点输入方式码
  char cardphyid[3];            //281	3	卡片序列号
  char termreadable[1];         //284	1	终端读取能力
  char iccndcode[1];            //285	1	IC卡条件代码
  char termperformance[6];      //286	6	终端性能
  char termchkresult[10];       //292	10	终端验证结果
  char unknownnum[8];           //302	8	不可预知数
  char devphyid[8];             //310	8	接口设备序列号
  char issuedata[64];           //318	64	发卡行应用数据
  char apptranscnt[4];             //382	4	应用交易记数器
  char appinteractive[4];       //386	4	应用交互特征
  char transdate[6];            //390	6	交易日期
  char country[3];              //396	3	终端国家代码
  char respcode[2];             //399	2	交易响应码
  char transtype[2];            //401	2	交易类型
  char authamt[12];             //403	12	授权金额
  char currencycode[3];         //415	3	交易币种代码
  char cipherchkresult[1];      //418	1	应用密文校验结果
  char cardexpiredate[4];       //419	4	卡有效期
  char cipherdata[2];           //423	2	密文信息数据
  char otheramt[12];            //425	12	其它金额
  char cardchkresult[6];        //437	6	持卡人验证方法结果
  char termtype[2];             //443	2	终端类型
  char filename[32];            //445	32	专用文件名称
  char appverno[4];             //477	4	应用版本号
  char transseqno[8];              //481	8	交易序列计数器
  char cashauthcode[6];         //489	6	电子现金发卡行授权码
  char productid[24];           //495	24	卡产品标识信息
  char crlf[2];                 //0x0D0x0A
};
#pragma pack()

int getIntFromTwoByte(const unsigned char* bytes);
void PadLeft(std::string &str,size_t num, char paddingChar = ' ');
void PadRight(std::string &str,size_t num,char paddingChar = ' ');
class CUnionPayPos 
{
protected:
  Ks8583Parser& m_parser;
  unsigned char m_ucPIK[17];
  unsigned char m_ucMAK[17];
public:
	int  m_nBatchno;
	int  m_nPosSeqno;
	int  m_nInfoCode;
	int  m_nRecordType;
	//签到	
	CUnionPayPos(Ks8583Parser& parser):m_parser(parser)
  {
     memset(m_ucPIK,0,sizeof(m_ucPIK));
     memset(m_ucMAK,0,sizeof(m_ucMAK));
     m_nBatchno=0;
     m_nPosSeqno=0;
	 m_nInfoCode=0;
	 m_nRecordType=0;
  }
  bool getMsgType(const char* posdata, int datalen, std::string& msgtype,int& infocode);
  bool calcKeyMac(const char* key, const char* data, char* mac);
  bool parsePosData(POSDTLFILE& pdf,bool parseAll);
  bool parseSignInData(POSDTLFILE& pdf);
  bool packSignInRespData(const POSDTLFILE& pdf,char* posdata);
  bool doPosStausData(POSDTLFILE& pdf,char* posdata);
  bool doPosPubKeyData(POSDTLFILE& pdf,char* posdata);
  bool doPosPubKeyDataEnd(POSDTLFILE& pdf,char* posdata);
  bool packPosRespData(const POSDTLFILE& pdf,char* posdata);
  bool packErrRespData(const char* msgtype,const char* errcode, const POSDTLFILE& pdf, char* posdata);
  void fillFixData(POSDTLFILE& pdf);
};
#endif
