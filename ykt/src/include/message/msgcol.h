// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MESSAGE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MESSAGE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifndef MSGCOL_H_
#define MSGCOL_H_
#include "message.h"
class MESSAGE_API CMsgCol: public  Cmessage
{
private:
  string	_colName;
  Json::Value* _jvRow;
  Json::Value* _jvColName;
  Json::Value* _jvColDesc;
  int getIndex( const char* colName );
public:
  CMsgCol( Json::Value& jvColName, Json::Value& jvColDesc, Json::Value& jvRow );
  CMsgCol& operator[]( const char* colName );
  int operator=( const char* colVal );
  int operator=( const string& colVal );
  int operator=( int colVal );
  int operator=( char colVal );
  int operator=( unsigned int colVal );
  int operator=( double colVal );
  void clear();
  int setValStr( const char* colName, const char* colVal );
  int setValInt( const char* colName, int colVal );
  int setValUint( const char* colName, unsigned int colVal );
  int setValDbl( const char* colName, double colVal );
  int setDesc( const char* colName, const char* colDesc );
};
#endif
