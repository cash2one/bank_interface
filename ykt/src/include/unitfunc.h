#ifndef _UNITFUNC_H_
#define _UNITFUNC_H_
#include <map>
#include <vector>
#include "message/msgreader.h"
#include "message/msgwriter.h"
#include "logger_imp.h"
#include "msgutil.h"
#include "errdef.h"
#include "pubdef.h"
#include "pubdb.h"
#include "pubfunc.h"
#include "dbfunc.h"
#include "dbfunc_foo.h"
#include "acctrans.h"
#include "transfunc.h"
#include "clsoper.h"
//////////////////////////////////////////////////////////////////////
// 目前只考虑单一数据库连接

typedef int (*PROCESSUNIT)();

typedef std::map<int,PROCESSUNIT> FUN_MAP;
extern FUN_MAP gFunMap;
inline bool doRegister(int regtype,PROCESSUNIT unit)
{
	gFunMap[regtype]=unit;
	return true;
}

#define FUNCTION(reqtype) int PROCESSUNIT##reqtype(); \
		bool _breg##reqtype = doRegister(reqtype,PROCESSUNIT##reqtype); \
		int PROCESSUNIT##reqtype()

#endif

