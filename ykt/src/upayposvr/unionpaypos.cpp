#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stddef.h>
#include <ctime>
#include "8583_impl.h"
#include "ks_8583_reader.h"
#include "tcp.h"
#include "unionpaypos.h"
#include "logger_imp.h"
#include "config_imp.h"
#include "gnudes.h"
#include "banktagparse.hpp"
#include "esqlc.h"
#include "pubdb.h"
#ifndef byte
typedef unsigned char byte;
#endif
using namespace std;
int getSysDate()
{
    struct tm *times;
    time_t t;
    t = time(0);
    times = localtime(&t);
    return ((times->tm_year + 1900) * 10000 + (times->tm_mon + 1) * 100 + times->tm_mday);
}
int getIntFromTwoByte(const unsigned char* bytes)
{
    int r = 0, t = 0;
    t = bytes[0];
    r = t << 8;
    r |= bytes[1];
    return r;
}
//extern "C" void print_hex_dump_bytes(const char *prefix_str, int prefix_type, const void *buf, size_t len);
int intToTwoByte(int num, byte* bytes)
{
    bytes[0] = (num >> 8) & 0xFF;
    bytes[1] = num & 0xFF;
    return 0;
}

/*
      'a '   or   'A '   ->   10,   '1 '   ->   1
      If   hex1char   not   in   '0 '- '9 ', 'A '- 'F ', 'a '- 'f '   return   0
*/
static char hex_to_num(char hex1char)
{
    if((hex1char >= '0') && (hex1char <= '9')) return hex1char - '0';
    if((hex1char >= 'a') && (hex1char <= 'f')) return hex1char - 'a' + 10;
    if((hex1char >= 'A') && (hex1char <= 'F')) return hex1char - 'A' + 10;
    return 0;
}

static char hex2_to_char(const char* str)
{
    const  char c1(*(str));
    const  char c2(*(str + 1));
    return hex_to_num(c1) * 0x10 + hex_to_num(c2);
}
int hexString2Byte(const char *input, size_t inlen, byte *output)
{
    size_t i = 0, q;
    for(q = 0; q < inlen; q += 2)
    {
        output[i] = hex2_to_char(input + q);
        i++;
    }
    return i;
}
#if 0
int HexString2Byte(const byte *input, int inlen, byte *output)
{
    size_t i = 0, q;
    for(q = 0; q < inlen; q += 2)
    {
        sscanf((const char*)input + q, "%02x", output + i);
        i++;
    }
    return i;
}
#endif

int GetIntByBytes(const unsigned char HEXstr[2])
{
    unsigned char r = 0, t = 0;
    t = HEXstr[0];
    r = t << 8;
    r |= HEXstr[1];
    return r;
}
int  int_to_BCD2(int num, unsigned char* BCDstr)
{
    unsigned char* s = BCDstr;
    if(num < 0 || num > 9999)  return -1;
    s[0] = (num / 1000 % 10) << 4 | (num / 100 % 10);
    s[1] = (num / 10 % 10) << 4 | (num % 10);
    return 0;
}
int BCD2_to_int(const unsigned char* BCDstr, int* iINT)
{
    int i, j, k, n;
    const unsigned char* s = BCDstr;
    i = (s[0] & 0xF0) >> 4;
    j = s[0] & 0x0F;
    k = (s[1] & 0xF0) >> 4;
    n = s[1] & 0x0F;
    if(i >= 0 && i <= 9 &&
       j >= 0 && j <= 9 &&
       k >= 0 && k <= 9 &&
       n >= 0 && n <= 9)
    {
        *iINT = i * 1000 + i * 100 + k * 10 + n;
        return 0;
    }
    return -1;
}
int Nstr_to_BCD(int align, const char* Nstr, int NstrLen, int* BCDstrLen, unsigned char* BCDstr)
{
    int i, j, n;
    const unsigned char* sn = (const unsigned char*)Nstr;
    unsigned char* sb = BCDstr;

    n = (NstrLen + 1) / 2;
    if(2 == align)
    {
        for(i = NstrLen - 1, j = n - 1; j >= 0; j--)
        {
            if(i >= 0)
            {
                if(sn[i] < '0' || sn[i] > '9')
                    return -1;
                sb[j] = sn[i] - '0';
                i--;
            }
            if(i >= 0)
            {
                if(sn[i] < '0' || sn[i] > '9')
                    return -1;
                sb[j] = (sn[i] - '0') << 4 | sb[j];
                i--;
            }
        }
    }
    else    /*LEFT_ALIGN*/
    {
        for(i = 0, j = 0; i < NstrLen; i++, j++)
        {
            if(sn[i] < '0' || sn[i] > '9')
                return -1;
            sb[j] = (sn[i] - '0') << 4;
            i++;
            if(i < NstrLen)
            {
                if(sn[i] < '0' || sn[i] > '9')
                    return -1;
                sb[j] = sb[j] | (sn[i] - '0');
            }
        }

    }
    *BCDstrLen = n;
    return 0;
}
size_t Byte2HexString(const byte* byteArray, size_t arraySize, char* hexString)
{
    const char* Convert = "0123456789ABCDEF";
    int t = 0;
    for(size_t i = 0; i < arraySize; i++)
    {
        hexString[t] = Convert[(byte)byteArray[i] >> 4];
        hexString[t + 1] = Convert[(byte)byteArray[i] & 0xf];
        t += 2;
    }
    hexString[t] = '\0';
    return t;
}
/*
int hexdump(const void* buff, int len)
{
  char filename[10] = ".hexdump";
  FILE* fp = fopen(filename, "wb");
  if(!fp)
    return -1;
  fwrite(buff, 1, len, fp);
  fclose(fp);
  char szcmd[128] = {0};
  sprintf(szcmd, " hexdump -C -n %d %s", len, filename);
  system(szcmd);
  return 0;
}
*/
void PadLeft(std::string& str, size_t num, char paddingChar)
{
    if(num > str.size())
        str.insert(0, num - str.size(), paddingChar);
}
void PadRight(std::string& str, size_t num, char paddingChar)
{
    if(num > str.size())
        str.insert(str.size(), num - str.size(), paddingChar);
}
#include <ctype.h>
#include <stdio.h>

void hexdump(void *ptr, int buflen)
{
    unsigned char *buf = (unsigned char*)ptr;
    int i, j;
    for(i = 0; i < buflen; i += 16)
    {
        printf("%06x: ", i);
        for(j = 0; j < 16; j++)
            if(i + j < buflen)
                printf("%02x ", buf[i + j]);
            else
                printf("   ");
        printf(" ");
        for(j = 0; j < 16; j++)
            if(i + j < buflen)
                printf("%c", isprint(buf[i + j]) || (buf[i + j] > 0x80) ? buf[i + j] : '.');
        printf("\n");
    }
}
string gethexdumpstr(const void *ptr, int buflen)
{
    ostringstream oss;
    const unsigned char *buf = (unsigned char*)ptr;
    int i, j;
    for(i = 0; i < buflen; i += 16)
    {
        oss << std::hex << std::setw(6) << std::setfill('0') << i << ": ";
        for(j = 0; j < 16; j++)
            if(i + j < buflen)
                oss << std::hex << std::setw(2) << std::setfill('0') << ((int)buf[i + j]) << " ";
            else
                oss << "   ";
        oss << " ";
        for(j = 0; j < 16; j++)
        {
            if(i + j < buflen)
            {
                if(isprint(buf[i + j]) || (buf[i + j] > 0x80))
                    oss << buf[i + j];
                else
                    oss << ".";
            }
        }
        oss << std::endl;
    }
    return oss.str();
}
//计算MAC报文
int  calcmac4unionpay(unsigned char* uKey, unsigned char* uMacData, int iMacDataLen, unsigned char* uMac)
{
    int i = 0;
    des_context ctx;
    byte uResultBlock[16];
    byte uResultBlock2[16];
    memset(uResultBlock, 0, sizeof(uResultBlock));
    memset(uResultBlock2, 0, sizeof(uResultBlock2));
    int nBlock = 0;
    if(iMacDataLen % 8)
        nBlock = iMacDataLen / 8 + 1; // 总块数
    else
        nBlock = iMacDataLen / 8;
    byte* buf = new byte[nBlock * 8 + 1];
    memset(buf, 0, nBlock * 8 * sizeof(byte));
    memcpy(buf, uMacData, iMacDataLen * sizeof(byte));
    memcpy(uResultBlock, buf, 8);
    for(i = 1; i < nBlock; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            uResultBlock[j] ^= buf[i * 8 + j];
        }
    }
    delete []buf;
    char szHexStr[17] = {0};
    Byte2HexString(uResultBlock, 8, szHexStr);
    memcpy(uResultBlock2, szHexStr, 16);
    des_set_key(&ctx, uKey);
    des_encrypt(&ctx, uResultBlock2, uResultBlock);
    for(int i = 0; i < 8; i++)
    {
        uResultBlock[i] ^= uResultBlock2[8 + i];
    }
    des_encrypt(&ctx, uResultBlock, uResultBlock2);
    memcpy(uMac, uResultBlock2, 4);
    return 0;
}
void hex2dec(const char* sHexStr, int iHexLen, unsigned char* uDecBuf, int& iDecBUfLen)
{
    int i = 0;
    unsigned long ul;
    char sHexTmp[3];
    int offset = 0;
    int dlen = iHexLen / 2;

    memset(sHexTmp, 0, sizeof(sHexTmp));
    if(iHexLen % 2)
    {
        sHexTmp[0] = '0';
        sHexTmp[1] = sHexStr[0];
        ul = strtoul(sHexTmp, NULL, 16);
        uDecBuf[0] = (unsigned char)ul;
        offset++;
    }
    for(i = 0; i < dlen; i++)
    {
        memcpy(sHexTmp, &sHexStr[2 * i + offset], 2);
        ul = strtoul(sHexTmp, NULL, 16);
        uDecBuf[i + offset] = (unsigned char)ul;
    }
    iDecBUfLen = i + offset;
}

bool  CheckKey(unsigned char* uKey, int iKeyType, unsigned char* uData, int iDataLen)
{
    if(iDataLen != 24 && iDataLen != 40 && iDataLen != 56)
        return false;
    unsigned char initData[8] = {0};
    unsigned char uResultBlock[16] = {0};
    if(1 == iKeyType)   //单倍
    {
        des_context ctx;
        des_set_key(&ctx, uKey);
        des_decrypt(&ctx, uData, uResultBlock);

        unsigned char macData[8] = {0};
        des_set_key(&ctx, uResultBlock);
        des_encrypt(&ctx, initData, macData);
        if(memcmp(macData, uData + 8, 4) != 0)
        {
            LOG(ERROR, "mac error1");
            return false;
        }
    }
    else            //双倍
    {
        des3_context ctx3;
        des3_set_2keys(&ctx3, uKey, uKey + 8);
        des3_decrypt(&ctx3, uData, uResultBlock);
        des3_decrypt(&ctx3, uData + 8, uResultBlock + 8);

        unsigned char macData[8] = {0};
        des3_set_2keys(&ctx3, uResultBlock, uResultBlock + 8);
        des3_encrypt(&ctx3, initData, macData);
        if(memcmp(macData, uData + 16, 4) != 0)
        {
            LOG(ERROR, "check Pin Key error");
            return false;
        }
        LOG(DEBUG, "check Pin Key OK");
        des_context ctx;
        des_set_key(&ctx, uKey);
        des_decrypt(&ctx, uData + 20, uResultBlock);
        des_set_key(&ctx, uResultBlock);
        des_encrypt(&ctx, initData, macData);
        if(memcmp(macData, uData + 36, 4) != 0)
        {
            LOG(ERROR, "check Mac Key error");
            return false;
        }
        LOG(DEBUG, "check Mac Key OK");
    }
    return true;
}
int CalcMac(Ks8583Parser::CalcMacParam* param)
{
    des_context ctx;
    int nBlock = 0;
    int iMacDataLen = param->data_len;
    if(iMacDataLen % 8)
        nBlock = iMacDataLen / 8 + 1; // 总块数
    else
        nBlock = iMacDataLen / 8;
    unsigned char* buff = new unsigned char[nBlock * 8];
    memset(buff, 0, nBlock * 8);
    memcpy(buff, param->data, iMacDataLen);
    byte tempbuff[8];
    memcpy(tempbuff, buff, sizeof(tempbuff));

    for(int n = 1; n < nBlock; n++)
    {
        for(int i = 0; i < 8; i++)
        {
            tempbuff[i] ^= buff[n * 8 + i];
        }
    }
    delete[] buff;
    unsigned char ucResultBuff[17];
    memset(ucResultBuff, 0, sizeof(ucResultBuff));

    Byte2HexString(tempbuff, 8, (char*)ucResultBuff);
    memcpy(tempbuff, ucResultBuff, sizeof(tempbuff));

    //取前8字节加密
    des_set_key(&ctx, param->mackey);
    des_encrypt(&ctx, tempbuff, tempbuff);

    for(int i = 0; i < 8; i++)
    {
        tempbuff[i] ^= ucResultBuff[8 + i];
    }
    des_set_key(&ctx, param->mackey);
    des_encrypt(&ctx, tempbuff, tempbuff);
    Byte2HexString(tempbuff, 8, (char*)ucResultBuff);
    memcpy(param->mac, ucResultBuff, 8);
    param->mac_len = 8;
    LOG(INFO, "mac:[" << param->mac << "]")
    return 0;
}
#define MYSET(x,c) memset(x,c,sizeof(x));
void CUnionPayPos::fillFixData(POSDTLFILE& pdf)
{
    memcpy(pdf.currency, "156", 3);
    //MYSET(pdf.termseqno, 0x20);
    MYSET(pdf.authdate, 0x20);
    MYSET(pdf.agencycode, 0x20);
    MYSET(pdf.sendercode, 0x20);
    MYSET(pdf.merchtype, 0x20);
    MYSET(pdf.merchname, 0x20);
    MYSET(pdf.orgitransinfo, '0');
    MYSET(pdf.msgresoncode, '0');
    MYSET(pdf.infoflag, '0');
    MYSET(pdf.cpusseqno, '0');
    MYSET(pdf.receivercode, 0x20);
    MYSET(pdf.issuercode, 0x20);
    MYSET(pdf.cupsnoticeflag, '0');
    memcpy(pdf.transstartway, "03", 2);
    MYSET(pdf.transtypeid, 0x20);
    MYSET(pdf.cupsreserved, 0x20);
    MYSET(pdf.possvrpointcode, '0');
    memset(pdf.selffeeamt, 0x20, 1);
    memset(pdf.selffeeamt + 1, '0', 11);
    MYSET(pdf.transarea, '0');
    MYSET(pdf.eciflag, '0');
    MYSET(pdf.specialfeeflag, 0x20);
    MYSET(pdf.specialfeetype, 0x20);
    MYSET(pdf.transstarttype, '0');
    MYSET(pdf.reserved, 0x20);
    memcpy(pdf.svrpointinputcode, "072", 3);
    MYSET(pdf.cardphyid, '0');
    MYSET(pdf.termreadable, '6');
    MYSET(pdf.iccndcode, '0');
    MYSET(pdf.cipherchkresult, 0x20);
    memcpy(pdf.crlf, "\x0D\x0A", 2);
    //MYSET(pdf.crlf,0);
}
#define SAFECOPY(dest,val) do{  strncpy(dest,val,sizeof(dest)); } while(0)
bool CUnionPayPos::getMsgType(const char* posdata, int datalen, std::string& msgtype, int& infocode)
{
    const int TPDU_LEN = 11;
    m_parser.Clear();
    char buff[2000];
    size_t bufflen = hexString2Byte(posdata, datalen, (byte*)buff);
    int ret = m_parser.UnpackData(buff + TPDU_LEN, bufflen - TPDU_LEN);
    if(ret)
    {
        LOG(ERROR, "unpack data error ret=" << ret);
        return false;
    }
    size_t fieldno = 0;
    if(m_parser.GetValueByIndex(fieldno, msgtype) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    fieldno = 60;
    std::string sFieldVal;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    infocode = atoi(string(sFieldVal.c_str() + 8, 3).c_str());
    return true;
}
bool CUnionPayPos::calcKeyMac(const char* key, const char* data, char* mac)
{
    unsigned char initData[8] = {0};
    unsigned char uResultBlock[16] = {0};
    unsigned char byteArrayKey[33] = {0};
    hexString2Byte(key, 32, byteArrayKey);
    unsigned char byteArrayData[33] = {0};
    hexString2Byte(data, 32, byteArrayData);

    des3_context ctx3;
    des3_set_2keys(&ctx3, byteArrayKey, byteArrayKey + 8);
    des3_decrypt(&ctx3, byteArrayData, uResultBlock);
    des3_decrypt(&ctx3, byteArrayData + 8, uResultBlock + 8);

    unsigned char macData[8] = {0};
    des3_set_2keys(&ctx3, uResultBlock, uResultBlock + 8);
    des3_encrypt(&ctx3, initData, macData);
    Byte2HexString(macData, 4, mac);
    return 0;
}
bool CUnionPayPos::parseSignInData(POSDTLFILE& pdf)
{
    size_t fieldno;
    string sFieldVal;
    fieldno = 11;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.termseqno, sFieldVal.c_str());
    fieldno = 41;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.termno, sFieldVal.c_str());
    fieldno = 42;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.merchno, sFieldVal.c_str());
    fieldno = 60;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    m_nBatchno = atoi(string(sFieldVal.c_str() + 2, 6).c_str());
    return true;
}
int GetUnionPayPubkey(int v_keyid, char* keydata);
bool CUnionPayPos::doPosStausData(POSDTLFILE& pdf, char* posdata)
{
    int ret = 0;
    size_t fieldno;
    string sFieldVal;
    fieldno = 41;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.termno, sFieldVal.c_str());
    fieldno = 42;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.merchno, sFieldVal.c_str());
    fieldno = 60;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    m_nBatchno = atoi(string(sFieldVal.c_str() + 2, 6).c_str());
    m_nInfoCode = atoi(string(sFieldVal.c_str() + 8, 3).c_str());
    LOG(DEBUG, "60:batchno" << m_nBatchno << "infocode" << m_nInfoCode);
    fieldno = 62;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    unsigned char ucReqCode[4] = {0};
    hexString2Byte(sFieldVal.c_str(), 6, ucReqCode);
    int nReqCode = atoi((char*)ucReqCode);
    LOG(DEBUG, "62:" << sFieldVal << "-->" << nReqCode);
    char szSysDate[9] = {0};
    char szSysTime[7] = {0};
    ret = db_getsysdatetime(szSysDate, szSysTime);
    if(ret)
    {
        LOG(ERROR, "db_getsysdate ret=" << ret);
        return false;
    }
    m_parser.Clear();
    ret = m_parser.SetValueByIndex(0, "0830");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(12, szSysTime, 6);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(13, szSysDate + 2, 4);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(39, "00");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(41, pdf.termno, sizeof(pdf.termno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(42, pdf.merchno, sizeof(pdf.merchno));
    if(ret < 0)
        return false;
    char szBuff[256] = {0};
    sprintf(szBuff, "00%06d%03d", m_nBatchno, m_nInfoCode);
    ret = m_parser.SetValueByIndex(60, string(szBuff));
    if(ret < 0)
        return false;
    char szKeyBuff[1000];
    memset(szKeyBuff, 0, sizeof(szKeyBuff));
    switch(nReqCode)
    {
        case 100:
        {
            strcpy(szKeyBuff, "32");
            ret = GetUnionPayPubkey(0, szKeyBuff + 2);
            if(ret)
            {
                LOG(ERROR, "GetUnionPayPubkey error ret=" << ret);
                return false;
            }
        }
        break;
        case 208:
        {
            strcpy(szKeyBuff, "32");
            ret = GetUnionPayPubkey(8, szKeyBuff + 2);
            if(ret)
            {
                LOG(ERROR, "GetUnionPayPubkey error ret=" << ret);
                return false;
            }
        }
        break;
        case 216:
        {
            strcpy(szKeyBuff, "33");
            ret = GetUnionPayPubkey(16, szKeyBuff + 2);
            if(ret)
            {
                LOG(ERROR, "GetUnionPayPubkey error ret=" << ret);
                return false;
            }
        }
        break;
        default:
            LOG(ERROR, "reqcode[" << nReqCode << "] error ");
            return false;
    }
    ret = m_parser.SetValueByIndex(62, szKeyBuff);
    if(ret < 0)
        return false;
    BufferType msg;
    size_t msglen = 0;
    size_t pack8583len = 0;
    char tpdu[] = "6000000718603100311903";
    int len = 0;
    Nstr_to_BCD(2, tpdu, strlen(tpdu), &len, (byte*)msg.begin() + msglen);
    msglen += len;
    m_parser.PackData(msg.begin() + msglen, &pack8583len, NULL);
    msglen += pack8583len;
    LOG(DEBUG, "\n" << gethexdumpstr(msg.data(), msglen));
    Byte2HexString((const byte*)(msg.data()), msglen, posdata);
    LOG(DEBUG, "msglen:" << msglen);
    LOG(DEBUG, "data:" << posdata);
    return true;
}
bool CUnionPayPos::doPosPubKeyData(POSDTLFILE& pdf, char* posdata)
{
    size_t fieldno;
    string sFieldVal;
    fieldno = 41;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.termno, sFieldVal.c_str());
    fieldno = 42;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.merchno, sFieldVal.c_str());
    fieldno = 60;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    m_nBatchno = atoi(string(sFieldVal.c_str() + 2, 6).c_str());
    m_nInfoCode = atoi(string(sFieldVal.c_str() + 8, 3).c_str());
    fieldno = 62;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }

    CBankTagParse TagParse;
    if(TagParse.parseData(sFieldVal.c_str(), sFieldVal.size()) < 1)
    {
        LOG(ERROR, "parse tag Data failed");
        return false;
    }
    LOG(DEBUG, "tag num:" << TagParse.m_Tag.size());
    LOG(DEBUG, "tag dump:" << TagParse.getDumpString());
    map<string, string>::iterator it;
    if((it = TagParse.m_Tag.find("9F06")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9F06 not found");
        return false;
    }
    char rid[256] = {0};
    strcpy(rid, it->second.c_str());

    if((it = TagParse.m_Tag.find("9F22")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9F22 not found");
        return false;
    }
    char keyIndex[33] = {0};
    strcpy(keyIndex, it->second.c_str());
    T_t_upaypubkey upaypubkey;
    memset(&upaypubkey, 0, sizeof(upaypubkey));
    int ret = DB_t_upaypubkey_read_by_rid_and_keyindex(rid, keyIndex, &upaypubkey);
    if(ret)
    {
        LOG(ERROR, "query by rid[" << rid << "]index[" << keyIndex << "] not found");
        return false;
    }
    char szSysDate[9] = {0};
    char szSysTime[7] = {0};
    ret = db_getsysdatetime(szSysDate, szSysTime);
    if(ret)
    {
        LOG(ERROR, "db_getsysdate ret=" << ret);
        return false;
    }
    m_parser.Clear();
    ret = m_parser.SetValueByIndex(0, "0810");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(12, szSysTime, 6);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(13, szSysDate + 2, 4);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(37, pdf.refno, sizeof(pdf.refno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(39, "00");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(41, pdf.termno, sizeof(pdf.termno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(42, pdf.merchno, sizeof(pdf.merchno));
    if(ret < 0)
        return false;
    char szBuff[256] = {0};
    sprintf(szBuff, "00%06d%03d", m_nBatchno, m_nInfoCode);
    ret = m_parser.SetValueByIndex(60, szBuff);
    if(ret < 0)
        return false;
    char keycontent[801] = {0};
    strcpy(keycontent, "31");
    strcat(keycontent, upaypubkey.content);
    ret = m_parser.SetValueByIndex(62, keycontent);
    if(ret < 0)
        return false;

    BufferType msg;
    size_t msglen = 0;
    size_t pack8583len = 0;
    char tpdu[] = "6000000718603100311903";
    int len = 0;
    Nstr_to_BCD(2, tpdu, strlen(tpdu), &len, (byte*)msg.begin() + msglen);
    msglen += len;
    m_parser.PackData(msg.begin() + msglen, &pack8583len, NULL);
    msglen += pack8583len;
    LOG(DEBUG, "\n" << gethexdumpstr(msg.data(), msglen));
    Byte2HexString((const byte*)(msg.data()), msglen, posdata);
    LOG(DEBUG, "msglen:" << msglen);
    LOG(DEBUG, "data:" << posdata);
    return true;
}
bool CUnionPayPos::doPosPubKeyDataEnd(POSDTLFILE& pdf, char* posdata)
{
    int ret = 0;
    size_t fieldno;
    string sFieldVal;
    fieldno = 41;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.termno, sFieldVal.c_str());
    fieldno = 42;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.merchno, sFieldVal.c_str());
    fieldno = 60;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    m_nBatchno = atoi(string(sFieldVal.c_str() + 2, 6).c_str());
    m_nInfoCode = atoi(string(sFieldVal.c_str() + 8, 3).c_str());
    char szSysDate[9] = {0};
    char szSysTime[7] = {0};
    ret = db_getsysdatetime(szSysDate, szSysTime);
    if(ret)
    {
        LOG(ERROR, "db_getsysdate ret=" << ret);
        return false;
    }
    m_parser.Clear();
    ret = m_parser.SetValueByIndex(0, "0810");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(12, szSysTime, 6);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(13, szSysDate + 2, 4);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(37, pdf.refno, sizeof(pdf.refno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(39, "00");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(41, pdf.termno, sizeof(pdf.termno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(42, pdf.merchno, sizeof(pdf.merchno));
    if(ret < 0)
        return false;
    char szBuff[256] = {0};
    sprintf(szBuff, "00%06d%03d", m_nBatchno, m_nInfoCode);
    ret = m_parser.SetValueByIndex(60, szBuff);
    if(ret < 0)
        return false;

    BufferType msg;
    size_t msglen = 0;
    size_t pack8583len = 0;
    char tpdu[] = "6000000718603100311903";
    int len = 0;
    Nstr_to_BCD(2, tpdu, strlen(tpdu), &len, (byte*)msg.begin() + msglen);
    msglen += len;
    m_parser.PackData(msg.begin() + msglen, &pack8583len, NULL);
    msglen += pack8583len;
    LOG(DEBUG, "\n" << gethexdumpstr(msg.data(), msglen));
    Byte2HexString((const byte*)(msg.data()), msglen, posdata);
    LOG(DEBUG, "msglen:" << msglen);
    LOG(DEBUG, "data:" << posdata);
    return true;
}
bool CUnionPayPos::parsePosData(POSDTLFILE& pdf, bool parseAll)
{
    size_t fieldno;
    string sFieldVal;
    fieldno = 0;
    fieldno = 2;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    PadRight(sFieldVal, 19);
    SAFECOPY(pdf.cardno, sFieldVal.c_str());
    fieldno = 3;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    fieldno = 4;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    PadLeft(sFieldVal, 12, '0');
    SAFECOPY(pdf.transamt, sFieldVal.c_str());
    fieldno = 11;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    PadLeft(sFieldVal, 6, '0');
    SAFECOPY(pdf.termseqno, sFieldVal.c_str());
    PadLeft(sFieldVal, 12, '0');
    SAFECOPY(pdf.refno, sFieldVal.c_str());
    fieldno = 13;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(INFO, "field " << fieldno << " not found");
        memset(pdf.transtime, '0', 4);
    }
    else
    {
        memcpy(pdf.transtime, sFieldVal.c_str(), 4);
    }
    fieldno = 12;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(INFO, "field " << fieldno << " not found");
        memset(pdf.transtime + 4, '0', 6);
    }
    else
    {
        memcpy(pdf.transtime + 4, sFieldVal.c_str(), 6);
    }
    if(parseAll)
    {
        fieldno = 14;
        if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
        {
            LOG(INFO, "field " << fieldno << " not found");
            //MYSET(pdf.cardexpiredate, '0');
            return false;
        }
        else
        {
            SAFECOPY(pdf.cardexpiredate, sFieldVal.c_str());
        }
        fieldno = 22;
        if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
        {
            LOG(ERROR, "get field index " << fieldno << " error");
            return false;
        }
        SAFECOPY(pdf.svrpointinputcode, sFieldVal.c_str());
        fieldno = 25;
        if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
        {
            LOG(ERROR, "get field index " << fieldno << " error");
            return false;
        }
        SAFECOPY(pdf.possvrpointcode, sFieldVal.c_str());
    }
	//根据银联要求，添加卡片序列号字段
	fieldno = 23;
	if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
	{
		LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
	PadLeft(sFieldVal, 3, '0');
    SAFECOPY(pdf.cardphyid, sFieldVal.c_str());
	
    fieldno = 41;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.termno, sFieldVal.c_str());

    fieldno = 42;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.merchno, sFieldVal.c_str());
    fieldno = 49;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    SAFECOPY(pdf.currency, sFieldVal.c_str());
    fieldno = 60;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    m_nBatchno = atoi(string(sFieldVal.c_str() + 2, 6).c_str());
    if(!parseAll)
        return true;
    fieldno = 15;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        //允许没有该域
    }
    else
    {
        if(sFieldVal.size())
        {			
			sFieldVal= sFieldVal.substr(0,sFieldVal.find_last_not_of("0")+1); 
            m_nRecordType = atoi(sFieldVal.c_str());
        }
    }
    fieldno = 55;
    if(m_parser.GetValueByIndex(fieldno, sFieldVal) < 0)
    {
        LOG(ERROR, "get field index " << fieldno << " error");
        return false;
    }
    CBankTagParse TagParse;
    if(TagParse.parseData(sFieldVal.c_str(), sFieldVal.size()) < 1)
    {
        LOG(ERROR, "parse tag Data failed");
        return false;
    }
    LOG(DEBUG, "tag num:" << TagParse.m_Tag.size());
    LOG(DEBUG, "tag dump:" << TagParse.getDumpString());
    //tag:9F26
    map<string, string>::iterator it;
    if((it = TagParse.m_Tag.find("9F26")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9F26 not found");
        return false;
    }
    SAFECOPY(pdf.ciphertext, it->second.c_str());
    //tag:9F33
    if((it = TagParse.m_Tag.find("9F33")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9F33 not found");
        return false;
    }
    SAFECOPY(pdf.termperformance, it->second.c_str());
    //tag:95
    if((it = TagParse.m_Tag.find("95")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 95 not found");
        return false;
    }
    SAFECOPY(pdf.termchkresult, it->second.c_str());
    //tag:9F37
    if((it = TagParse.m_Tag.find("9F37")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9F37 not found");
        return false;
    }
    SAFECOPY(pdf.unknownnum, it->second.c_str());
    //tag:9F1E
    if((it = TagParse.m_Tag.find("9F1E")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 9F1E not found");
        MYSET(pdf.devphyid, '0');
    }
    else
    {
        SAFECOPY(pdf.devphyid, it->second.c_str());
    }
    //tag:9F10
    if((it = TagParse.m_Tag.find("9F10")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9F10 not found");
        return false;
    }
    std::string sTagVal = it->second;
    PadRight(sTagVal, 64);
    SAFECOPY(pdf.issuedata, sTagVal.c_str());
    //tag:9F36
    if((it = TagParse.m_Tag.find("9F36")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9F36 not found");
        return false;
    }
    SAFECOPY(pdf.apptranscnt, it->second.c_str());
    //tag:82
    if((it = TagParse.m_Tag.find("82")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 82 not found");
        return false;
    }
    SAFECOPY(pdf.appinteractive, it->second.c_str());
    //tag:9A
    if((it = TagParse.m_Tag.find("9A")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9A not found");
        return false;
    }
    SAFECOPY(pdf.transdate, it->second.c_str());
    //tag:9F1A
    if((it = TagParse.m_Tag.find("9F1A")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9F1A not found");
        return false;
    }
    SAFECOPY(pdf.country, it->second.c_str() + 1);
    //tag:8A
    if((it = TagParse.m_Tag.find("8A")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 8A not found");
        return false;
    }
    else
    {
        unsigned char byteArray[20] = {0};
        hexString2Byte(it->second.c_str(), it->second.size(), byteArray);
        SAFECOPY(pdf.respcode, (char*)byteArray);
    }
    //tag:9C
    /*if((it = TagParse.m_Tag.find("9C")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9C not found");
        return false;
    }
    SAFECOPY(pdf.transtype, it->second.c_str());*/
    SAFECOPY(pdf.transtype, "00");
    //tag:9F02
    if((it = TagParse.m_Tag.find("9F02")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 9F02 not found");
    }
    else
    {
        SAFECOPY(pdf.authamt, it->second.c_str());
    }
    //tag:5F2A
    if((it = TagParse.m_Tag.find("5F2A")) == TagParse.m_Tag.end())
    {
        LOG(ERROR, "tag 5F2A not found");
        return false;
    }
    SAFECOPY(pdf.currencycode, it->second.c_str() + 1);
    //tag:9F27
    if((it = TagParse.m_Tag.find("9F27")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 9F27 not found");
        //MYSET(pdf.cipherdata, 0x20);
        SAFECOPY(pdf.cipherdata, "40");
    }
    else
    {
    	if(strncmp(it->second.c_str(), "00", 2)==0)
			SAFECOPY(pdf.cipherdata, "40");
		else
        	SAFECOPY(pdf.cipherdata, it->second.c_str());
    }
    //tag:9F03
    if((it = TagParse.m_Tag.find("9F03")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 9F03 not found");
        MYSET(pdf.otheramt, '0');
    }
    else
    {
        sTagVal = it->second;
        PadLeft(sTagVal, 12, '0');
        SAFECOPY(pdf.otheramt, sTagVal.c_str());
    }
    //tag:9F34
    if((it = TagParse.m_Tag.find("9F34")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 9F34 not found");
        MYSET(pdf.cardchkresult, 0x20);
    }
    else
    {
        SAFECOPY(pdf.cardchkresult, it->second.c_str());
    }
    //tag:9F35
    if((it = TagParse.m_Tag.find("9F35")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 9F35 not found");
        MYSET(pdf.termtype, 0x20);
    }
    else
    {
        SAFECOPY(pdf.termtype, it->second.c_str());
    }
    //tag:84
    if((it = TagParse.m_Tag.find("84")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 84 not found");
        MYSET(pdf.filename, 0x20);
    }
    else
    {
        sTagVal = it->second;
        PadRight(sTagVal, 32);
        SAFECOPY(pdf.filename, sTagVal.c_str());
    }
    //tag:9F09
    if((it = TagParse.m_Tag.find("9F09")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 9F09 not found");
        MYSET(pdf.appverno, '0');
    }
    else
    {
        SAFECOPY(pdf.appverno, it->second.c_str());
    }
    //tag:9F41
    if((it = TagParse.m_Tag.find("9F41")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 9F41 not found");
        MYSET(pdf.transseqno, '0');
    }
    else
    {
        SAFECOPY(pdf.transseqno, it->second.c_str());
    }
    //tag:9F74
    if((it = TagParse.m_Tag.find("9F74")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 9F74 not found");
        MYSET(pdf.authidresp, '0');
        //MYSET(pdf.cashauthcode, '0');
        SAFECOPY(pdf.cashauthcode, "ECC001");
    }
    else
    {
        unsigned char byteArray[20] = {0};
        hexString2Byte(it->second.c_str(), it->second.size(), byteArray);
        SAFECOPY(pdf.authidresp, (char*)byteArray);
        //SAFECOPY(pdf.cashauthcode, (char*)byteArray);
        SAFECOPY(pdf.cashauthcode, "ECC001");
    }
    //tag:9F63
    if((it = TagParse.m_Tag.find("9F63")) == TagParse.m_Tag.end())
    {
        LOG(INFO, "tag 9F63 not found");
        MYSET(pdf.productid, 0x20);
    }
    else
    {
        SAFECOPY(pdf.productid, it->second.c_str());
    }
    fillFixData(pdf);
    LOG(INFO, "parse pos data success");
    return true;
}
bool CUnionPayPos::packPosRespData(const POSDTLFILE& pdf, char* posdata)
{
    int ret = 0;
    char szSysDate[9] = {0};
    ret = db_getsysdate(szSysDate);
    if(ret)
    {
        LOG(ERROR, "db_getsysdate ret=" << ret);
        return false;
    }
    m_parser.Clear();
    ret = m_parser.SetValueByIndex(0, "0210");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(3, "000000");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(4, pdf.transamt, sizeof(pdf.transamt));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(11, pdf.termseqno, sizeof(pdf.termseqno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(12, pdf.transtime + 4, 6);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(13, pdf.transtime, 4);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(15, szSysDate + 4, 4);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(25, "00");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(32, "0");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(37, pdf.refno, sizeof(pdf.refno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(39, "00");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(41, pdf.termno, sizeof(pdf.termno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(42, pdf.merchno, sizeof(pdf.merchno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(44, "00");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(49, pdf.currency, sizeof(pdf.currency));
    if(ret < 0)
        return false;
    char szBuff[256] = {0};
    sprintf(szBuff, "36%06d%03d", m_nBatchno, m_nInfoCode);
    ret = m_parser.SetValueByIndex(60, szBuff);
    if(ret < 0)
        return false;
    BufferType msg;
    size_t msglen = 0;
    size_t pack8583len = 0;
    char tpdu[] = "6000000718603100311903";
    int len = 0;
    Nstr_to_BCD(2, tpdu, strlen(tpdu), &len, (byte*)msg.begin() + msglen);
    msglen += len;
    m_parser.PackData(msg.begin() + msglen, &pack8583len, NULL);
    msglen += pack8583len;
    LOG(DEBUG, "\n" << gethexdumpstr(msg.data(), msglen));
    Byte2HexString((const byte*)(msg.data()), msglen, posdata);
    LOG(DEBUG, "msglen:" << msglen);
    LOG(DEBUG, "data:" << posdata);
    return true;
}
bool CUnionPayPos::packSignInRespData(const POSDTLFILE& pdf, char* posdata)
{
    int ret = 0;
    char szSysDate[9] = {0};
    char szSysTime[7] = {0};
    ret = db_getsysdatetime(szSysDate, szSysTime);
    if(ret)
    {
        LOG(ERROR, "db_getsysdate ret=" << ret);
        return false;
    }
    m_parser.Clear();
    ret = m_parser.SetValueByIndex(0, "0810");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(11, pdf.termseqno, sizeof(pdf.termseqno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(12, szSysTime, 6);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(13, szSysDate + 2, 4);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(32, "0");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(37, pdf.refno, sizeof(pdf.refno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(39, "00");
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(41, pdf.termno, sizeof(pdf.termno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(42, pdf.merchno, sizeof(pdf.merchno));
    if(ret < 0)
        return false;
    char szBuff[256] = {0};
    sprintf(szBuff, "00%06d003", m_nBatchno);
    ret = m_parser.SetValueByIndex(60, string(szBuff));
    if(ret < 0)
        return false;
    //查询密钥
    T_t_upaykey upaykey;
    memset(&upaykey, 0, sizeof(upaykey));
    ret = DB_t_upaykey_read_by_id(1, &upaykey);
    if(ret)
    {
        LOG(ERROR, "query pos key error ret=" << ret);
        return false;
    }
    char szKeyBuff[81] = {0};
    char szPIKMac[9] = {0};
    calcKeyMac(upaykey.mk, upaykey.pik, szPIKMac);
    char szMAKMac[9] = {0};
    calcKeyMac(upaykey.mk, upaykey.mak, szMAKMac);
    memcpy(szKeyBuff, upaykey.pik, 32);
    memcpy(szKeyBuff + 32, szPIKMac, 8);
    memcpy(szKeyBuff + 40, upaykey.mak, 32);
    memcpy(szKeyBuff + 72, szMAKMac, 8);
    ret = m_parser.SetValueByIndex(62, szKeyBuff);
    if(ret < 0)
        return false;
    BufferType msg;
    size_t msglen = 0;
    size_t pack8583len = 0;
    char tpdu[] = "6000000718603100311903";
    int len = 0;
    Nstr_to_BCD(2, tpdu, strlen(tpdu), &len, (byte*)msg.begin() + msglen);
    msglen += len;
    m_parser.PackData(msg.begin() + msglen, &pack8583len, NULL);
    msglen += pack8583len;
    //intToTwoByte((int)(msglen - 2), (byte*)msg.begin());
    LOG(DEBUG, "\n" << gethexdumpstr(msg.data(), msglen));
    Byte2HexString((const byte*)(msg.data()), msglen, posdata);
    LOG(DEBUG, "msglen:" << msglen);
    LOG(DEBUG, "data:" << posdata);
    // LOG(DEBUG,"\n"<<gethexdumpstr( msg.data(), msglen ));
    return true;
}
bool CUnionPayPos::packErrRespData(const char* msgtype, const char* errcode, const POSDTLFILE& pdf, char* posdata)
{
    int ret = 0;
    char szSysDate[9] = {0};
    char szSysTime[7] = {0};
    ret = db_getsysdatetime(szSysDate, szSysTime);
    if(ret)
    {
        LOG(ERROR, "db_getsysdate ret=" << ret);
        return false;
    }
    m_parser.Clear();
    ret = m_parser.SetValueByIndex(0, msgtype);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(11, pdf.termseqno, sizeof(pdf.termseqno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(12, szSysTime, 6);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(13, szSysDate + 2, 4);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(39, errcode, 2);
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(41, pdf.termno, sizeof(pdf.termno));
    if(ret < 0)
        return false;
    ret = m_parser.SetValueByIndex(42, pdf.merchno, sizeof(pdf.merchno));
    if(ret < 0)
        return false;
    BufferType msg;
    size_t msglen = 0;
    size_t pack8583len = 0;
    char tpdu[] = "6000000718603100311903";
    int len = 0;
    Nstr_to_BCD(2, tpdu, strlen(tpdu), &len, (byte*)msg.begin() + msglen);
    msglen += len;
    m_parser.PackData(msg.begin() + msglen, &pack8583len, NULL);
    msglen += pack8583len;
    //intToTwoByte((int)(msglen - 2), (byte*)msg.begin());
    LOG(DEBUG, "\n" << gethexdumpstr(msg.data(), msglen));
    Byte2HexString((const byte*)(msg.data()), msglen, posdata);
    return true;
}
