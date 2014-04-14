// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MESSAGE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MESSAGE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifndef MSGWRITER_H_
#define MSGWRITER_H_
#include "message.h"
#include "msgattr.h"
#include "msgcol.h"
//#include "..\\..\\transfer\\transfer\\transfer.h"

class MESSAGE_API CMsgWriter: public Cmessage
{
private:
  Json::Value jvRoot;
  Json::Value jvRow;
  Json::Value jvArrayRow;
  Json::Value jvArrayColName;
  Json::Value jvArrayColDesc;
  Json::Value* pjvRowData;
  string _jsonstr;
  string _printstr;
public:
  CMsgAttr attr;
  CMsgCol	col;
  CMsgWriter( void );
  void clear();
  void setIgnoreCase( bool ignorecase = true );
  int getSetColIndex( const char* colName );
  int setAttrValInt( const char* attrName, int attrVal );
  int setAttrValUint( const char* attrName, unsigned int attrVal );
  int setAttrValStr( const char* attrName, const char* attrVal );
  int setAttrValDbl( const char* attrName, double attrVal );
  int setColDesc( const char* colName, const char* colDesc );
  int setColValStr( const char* colName, const char* colVal );
  int setColValInt( const char* colName, int colVal );
  int setColValUint( const char* colName, unsigned int colVal );
  int setColValDbl( const char* colName, double colVal );
  int cloneRow();
  int addRow();
  int serialize();
  const string& getMsgStr();
  const string& getPrintMsgStr();
  int SendData();
};
#endif
