/* --------------------------------------------
 * 创建日期: 2011-11-21
 * 程序作者: 闻剑
 * 版本信息: 1.0.0.0
 * 程序功能:
 * --------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "errdef.h"
#include "pubdef.h"
#include "pubdb.h"
#include "pubfunc.h"
#include "dbfunc.h"
#include "acctrans.h"
#include <iostream>
#include <string>
#include <sstream>
#include "unitfunc.h"
#include "config_imp.h"
#include "unitprocess.h"
#include "bdefine.h"

using namespace std;

#define ERRTYPE_NOERROR	 		0	//没有错误
#define ERRTYPE_NETWORK	 		1	//网络故障
#define ERRTYPE_DATABASE		2	//数据库故障
#define ERRTYPE_SYSTEM			3   //系统错误
#define ERRTYPE_BUSINESS		9	//正常的业务处理错误

CSvrLink* g_pSvrLink;  // 与业务调度中心的连接
CLogFile g_LogFile;   // 被移到SvrLink.cpp中，作为系统必要的属性
FUN_MAP gFunMap;
CMsgReader	gReader;
CMsgWriter  gWriter;
char gVersion[128];
#define SETERRMSG(message) do { std::ostringstream oss;oss<<message;errmsg = oss.str();}while(0)

int ProcessRequest(TRUSERID* handle, ST_PACK* reqPack);
int CallBDFunc(int fno, TRUSERID* handle, ST_CPACK* rpack, ST_PACK* pArrays, int* iRetCode, char* szMsg)
{
  int ret = 0;
  *iRetCode = 0;
  *szMsg = 0;
  LOG(DEBUG, "CallBDFunc reqtype=" << rpack->head.RequestType);
  ret = ProcessRequest(handle, &(rpack->pack));
  if(ret)
  {
    return ret;
  }
  gReader.clear();
  gWriter.clear();
  return 0;
}
int process(TRUSERID* handle, int iRequest, ST_PACK* rPack, int* pRetCode, char* szMsg)
{
  return 0;
}
TBDefine g_XBDefines[] =
{
  INSERT_FUNCTION(200000 , process, "xxx", "tc", 1)
  INSERT_FUNCTION(0, NULL, "END BPFunctions List", "wj", 0) // the last mark line，Don't remove this line
};
int WriteAppInfo(int bccBaseFuncNo, int bccID)
{
  // 这里需要各个业务类型不同作相应处理：
  // 当需要采用动态注册功能的时候，这里可以进行本BU组向BCC注册功能的处理
  TBDefine* pBDefine;
  for(pBDefine = g_XBDefines; pBDefine->RequestType != 0; pBDefine++)
  {
    g_pSvrLink->UpdateRegFunction(pBDefine->RequestType, pBDefine->szRTName, pBDefine->szProgrammer, pBDefine->iPriority, pBDefine->bStoped);
  }
  return(0);
}
UnitProcess::UnitProcess()
{
  errcode = 0;
  m_svrLink = NULL;
  memset(&m_bunit, 0, sizeof(m_bunit));
}
UnitProcess::~UnitProcess()
{
  if(m_svrLink)
  {
    m_svrLink->Close();
    m_svrLink = NULL;
    g_LogFile.Close();
  }
}
int UnitProcess::init()
{
  strcpy(m_bunit.szSvrIP, config_obj.server_ip.c_str());
  m_bunit.iSvrPort = config_obj.server_port;
  m_bunit.iHBInterval = 5000;
  strcpy(m_bunit.szBUGroupID, "BU1");
  if(NULL == m_svrLink)
  {
    g_pSvrLink = BUPubInitialize(g_XBDefines, CallBDFunc, WriteAppInfo, &g_LogFile);
    SetLogShowLevel(0);
    g_LogFile.Open("bulog");
    g_LogFile.RemoveOverdueLogFile(10);
    m_svrLink = g_pSvrLink ;
  }
  ResetBPFunctions();
  return 0;
}
int UnitProcess::connectServer()
{
  for(int i = 0; i < 30; i++)
  {
    int ret = m_svrLink->ToLink(&m_bunit);
    if(ret == 1)
    {
      LOG(INFO, "与业务调度中心(BCC)的连接成功");
      break;
    }
    else if(ret == -100)
    {
      LOG(NOTICE, "估计业务调度中心(BCC)尚未正式启动，或者检查配置中的[SERVER]项参数");
      sleep(1);
    }
    else
    {
      // 应该属于故障，或对方并不是BCC
      return ret;
    }
  }
  return 0;
}
int UnitProcess::run()
{
  int ret;
  ret = init();
  if(ret)
    return ret;
  ret = ConnectDb(config_obj.connectinfo.c_str());
  if(ret)
  {
    LOG(ERROR, "连接数据库失败，系统启动失败");
    return ret;
  }
  if(connectServer())
  {
    LOG(ERROR, "connect to [" << m_bunit.szSvrIP << ":" << m_bunit.iSvrPort << "] error");
    return -1;
  }
  while(m_svrLink->LinkOK())
  {
    m_svrLink->Processing(&m_bunit);
    if(m_svrLink->bExit) break;
  }
  LOG(DEBUG, "exit");
  return 0;
}
int GetReqDataLen(ST_PACK* reqPack)
{
  if(NULL == reqPack)
    return -1;
  ST_SDPACK* psd = (ST_SDPACK*)reqPack;
  return psd->usDataLength;
}
int GetReqData(ST_PACK* reqPack, string& strReq)
{
  if(NULL == reqPack)
    return -1;
  ST_SDPACK* psd = (ST_SDPACK*)reqPack;
  if(psd->usDataLength > 8000 || psd->usDataLength < 1)
  {
    return -1;
  }
  strReq.assign(psd->data, psd->usDataLength);
  return psd->usDataLength;
}
const char* GetReqData(ST_PACK* reqPack)
{
  if(NULL == reqPack)
    return NULL;
  ST_SDPACK* psd = (ST_SDPACK*)reqPack;
  if(psd->usDataLength > 8000 || psd->usDataLength < 1)
  {
    return NULL;
  }
  psd->data[psd->usDataLength] = 0;
  return psd->data;
}

int PutRespData(const char* respData, int respDataLen)
{
  if(NULL == respData)
    return -1;
  ST_PACK sPack;
  ST_SDPACK* psd = (ST_SDPACK*)&sPack;
  memcpy(psd->data, respData, respDataLen);
  psd->usDataLength = respDataLen;
  return PutRowData(&sPack);
}
int HandleError(int errtype, int retcode, const char* retmsg)
{
  gWriter.clear();
  if(gReader.parsed())
    gWriter.attr["funcno"] = gReader.getAttrValAsInt("funcno");
  switch(errtype)
  {
  case ERRTYPE_NETWORK:
    gWriter.attr["errname"] = "网络故障";
    break;
  case ERRTYPE_DATABASE:
    gWriter.attr["errname"] = "数据库故障";
    break;
  case ERRTYPE_SYSTEM:
    gWriter.attr["errname"] = "系统错误";
    break;
  case ERRTYPE_BUSINESS:
    gWriter.attr["errname"] = "业务处理失败";
    break;
  default:
    errtype = ERRTYPE_BUSINESS;
    gWriter.attr["errname"] = "业务处理失败";
    break;
  }
  if(strlen(retmsg) < 1)
  {
    if(retcode)
    {
      CAccTrans& ats = CAccTrans::GetInst();
      gWriter.attr["retmsg"] = ats.GetErrMsgTip(retcode).c_str();
    }
  }
  else
    gWriter.attr["retmsg"] = retmsg;
  gWriter.attr["dbmsg"] = g_sqlmsg;
  gWriter.attr["errtype"] = errtype;
  gWriter.attr["retcode"] = retcode;
  gWriter.serialize();
  const string& respstr = gWriter.getMsgStr();
  LOG(DEBUG, "Error Resp: " << respstr);
  return PutRespData(respstr.c_str(), respstr.size());
}
int ProcessRequest(TRUSERID* handle, ST_PACK* reqPack)
{
  int ret = 0;
  int funcno = 0;
  string errmsg;
  const char* sendbuff = GetReqData(reqPack);
  if(!sendbuff)
  {
    SETERRMSG("非法报文");
    LOG(ERROR, errmsg);
    HandleError(ERRTYPE_SYSTEM, ret, "报文解析错误");
    return ret;
  }
  try
  {

    //    LOG( DEBUG, "request message: " << sendbuff );
    ret = gReader.parse(sendbuff);
    if(ret)
    {
      SETERRMSG("报文解析错误");
      LOG(ERROR, "parse string failed,string:" << sendbuff);
      HandleError(ERRTYPE_SYSTEM, ret, "报文解析错误");
      return ret;
    }
    LOG(DEBUG, "parse request message ok");
    funcno = gReader.getAttrValAsInt("funcno");
    if(funcno < 1)
    {
      SETERRMSG("未发现funcno属性,报文不合法");
      HandleError(ERRTYPE_SYSTEM, -1, errmsg.c_str());
      LOG(ERROR, errmsg);
      return -1;
    }
    LOG(DEBUG, "read funcno ok");
  }
  catch(runtime_error theRuntimeError)
  {
    SETERRMSG("非法的请求报文:" << theRuntimeError.what());
    LOG(ERROR, errmsg);
    HandleError(ERRTYPE_SYSTEM, -1, errmsg.c_str());
    return -1;
  }
  catch(...)
  {
    SETERRMSG("非法的请求报文");
    LOG(ERROR, errmsg);
    HandleError(ERRTYPE_SYSTEM, -2, errmsg.c_str());
    return -2;
  }
  if(!CheckDbConnected())
  {
    LOG(INFO, "database is disconnected,reconnecting ...");
    ret = ConnectDb(config_obj.connectinfo.c_str());
    if(ret)
    {
      SETERRMSG("数据库连接已断开,重连失败,交易取消");
      HandleError(ERRTYPE_DATABASE, ret, errmsg.c_str());
      return ret;
    }
    LOG(INFO, "reconnect to database OK");
  }
  CAccTrans& ats = CAccTrans::GetInst();
  ret = ats.Reset();
  if(ret)
    return ret;
  LOG(DEBUG, "request message " << funcno << ": " << sendbuff);
  FUN_MAP::iterator it = gFunMap.find(funcno);
  if(it == gFunMap.end())
  {
    SETERRMSG("系统不支持功能号" << funcno);
    LOG(ERROR, errmsg);
    HandleError(ERRTYPE_SYSTEM, -3, errmsg.c_str());
    return -3;
  }
  PROCESSUNIT function = gFunMap[funcno];
  ret = (*function)();
  if(ret)
  {
    db_rollback();
    HandleError(ERRTYPE_BUSINESS, ret, errmsg.c_str());
    LOG(ERROR, "call function ret=" << ret << ",funcno=" << funcno);
    return ret;
  }
  //LOG(DEBUG,"call "<<funcno<<"transaction end");
  db_commit();
  gWriter.attr["funcno"] = funcno;
  gWriter.attr["errtype"] = 0;
  gWriter.attr["retcode"] = 0;
  LOG(DEBUG, funcno << "serialze start");
  gWriter.serialize();
  LOG(DEBUG, funcno << "serialze end");
  const string& respstr = gWriter.getMsgStr();
  if(respstr.size() < 1024)
  {
    LOG(DEBUG, "response message " << funcno << ": " << respstr);
  }
  PutRespData(respstr.c_str(), respstr.size());
  LOG(DEBUG, "call function success funcno=" << funcno);
  return 0;
}

