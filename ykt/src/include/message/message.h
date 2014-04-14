// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MESSAGE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MESSAGE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifndef MESSAGE_H_
#define MESSAGE_H_
#include "json/json.h"
#include<string>
#include<map>
using namespace std;
#ifdef WIN32
#ifdef MESSAGE_EXPORTS
#define MESSAGE_API __declspec(dllexport)
#else
#define MESSAGE_API __declspec(dllimport)
#endif
#else 
#define MESSAGE_API
#endif
#define ATTR_ROWCNT	"rowcnt"
#define ATTR_COLCNT "colcnt"
#define ATTR_COLNAME	"colname"
#define ATTR_COLDESC	"coldesc"



#define MESSAGE_READER_ASSERT( condition, message ) if (!( condition )) throw std::runtime_error( message );
typedef map<string,int> COLMAP;

class MESSAGE_API Cmessage {
protected:
	int iRowCnt;
	COLMAP colIndexMap;
  bool bIgnoreCase;
public:
	Cmessage(void);
void setIgnoreCase(bool ignorecase=true);
int getColIndex(const char* colName);
int getColCnt();
int getRowCnt();
int jsonVal2Str(const Json::Value& jsonVal,string& val);
int jsonVal2Int(const Json::Value& jsonVal,int& val);
int jsonVal2Uint(const Json::Value& jsonVal,unsigned int& val);
int jsonVal2Dbl(const Json::Value& jsonVal,double& val);
string jsonValAsStr(const Json::Value& jsonVal);
int jsonValAsInt(const Json::Value& jsonVal);
unsigned int jsonValAsUint(const Json::Value& jsonVal);
double jsonValAsDbl(const Json::Value& jsonVal);
};
#endif
