#ifndef unionpaypos_h_
#define unionpaypos_h_
#include "8583_impl.h"
#include "ks_8583_reader.h"
#include "tcp.h"
#pragma  pack(1)
struct  POSDTLFILE
{
  char cardno[19];               //0	19	���˺�
  char transamt[12];            //19	12	���׽��
  char currency[3];             //31	3	���׻��Ҵ���
  char transtime[10];           //34	10	����ʱ��
  char termseqno[6];           //44	6	ϵͳ���ٺ�
  char authidresp[6];           //50	6	��ȨӦ���ʶ��
  char authdate[4];             //56	4	��Ȩ����
  char refno[12];               //60	12	�����ο���
  char agencycode[11];          ///72	11	���������ʶ��
  char sendercode[11];          //83	11	���ͻ�����ʶ��
  char merchtype[4];            //94	4	�̻�����
  char termno[8];               //98	8	�ܿ����ն˱�ʶ��
  char merchno[15];             //106	15	�ܿ�����ʶ��
  char merchname[40];        //121	40	�ܿ������Ƶ�ַ
  char orgitransinfo[23];       //161	23	ԭʼ������Ϣ
  char msgresoncode[4];         //184	4	����ԭ�����
  char infoflag[1];             //188	1	��˫��Ϣ��־
  char cpusseqno[9];            //189	9	CUPS��ˮ��
  char receivercode[11];        //198	11	���ջ�������
  char issuercode[11];          //209	11	������������
  char cupsnoticeflag[1];       //220	1	CUPS֪ͨ��־
  char transstartway[2];        //221	2	���׷�������
  char transtypeid[1];          //223	1	����������ʶ
  char cupsreserved[8];         //224	8	CUPS����ʹ��
  char possvrpointcode[2];      //232	2	POS�������������
  char selffeeamt[12];          //234	12	����������
  char transarea[1];            //246	1	���׵���
  char eciflag[2];              //247	2	ECI��־
  char specialfeeflag[2];       //249	2	����Ʒѱ�־
  char specialfeetype[1];       //251	1	����Ʒѵ���
  char transstarttype[1];       //252	1	���׷���ʽ
  char reserved[9];             //253	9	����ʹ��
  char ciphertext[16];       //262	16	Ӧ������
  char svrpointinputcode[3];    //278	3	��������뷽ʽ��
  char cardphyid[3];            //281	3	��Ƭ���к�
  char termreadable[1];         //284	1	�ն˶�ȡ����
  char iccndcode[1];            //285	1	IC����������
  char termperformance[6];      //286	6	�ն�����
  char termchkresult[10];       //292	10	�ն���֤���
  char unknownnum[8];           //302	8	����Ԥ֪��
  char devphyid[8];             //310	8	�ӿ��豸���к�
  char issuedata[64];           //318	64	������Ӧ������
  char apptranscnt[4];             //382	4	Ӧ�ý��׼�����
  char appinteractive[4];       //386	4	Ӧ�ý�������
  char transdate[6];            //390	6	��������
  char country[3];              //396	3	�ն˹��Ҵ���
  char respcode[2];             //399	2	������Ӧ��
  char transtype[2];            //401	2	��������
  char authamt[12];             //403	12	��Ȩ���
  char currencycode[3];         //415	3	���ױ��ִ���
  char cipherchkresult[1];      //418	1	Ӧ������У����
  char cardexpiredate[4];       //419	4	����Ч��
  char cipherdata[2];           //423	2	������Ϣ����
  char otheramt[12];            //425	12	�������
  char cardchkresult[6];        //437	6	�ֿ�����֤�������
  char termtype[2];             //443	2	�ն�����
  char filename[32];            //445	32	ר���ļ�����
  char appverno[4];             //477	4	Ӧ�ð汾��
  char transseqno[8];              //481	8	�������м�����
  char cashauthcode[6];         //489	6	�����ֽ𷢿�����Ȩ��
  char productid[24];           //495	24	����Ʒ��ʶ��Ϣ
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
	//ǩ��	
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
