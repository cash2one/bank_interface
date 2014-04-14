#ifndef UNITPROCESS_H_
#define UNITPROCESS_H_
#include "stdafx.h"
#include "cpack.h"
#include "bupub.h"
#include "svrlink.h"
class UnitProcess
{
private:
  CSvrLink *m_svrLink;  // ��ҵ��������ĵ�����
  ST_BUNIT m_bunit;
public:
  int   errcode;
  int connectServer();
  int run();
  int init();
  UnitProcess();
  ~UnitProcess();
};
#endif
