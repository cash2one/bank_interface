#ifndef MSGATTR_H_
#define MSGATTR_H_
#include "message.h"
class MESSAGE_API CMsgAttr:public  Cmessage
{
private:
	string _attrName;
	Json::Value* jvRoot;
public:
	CMsgAttr(Json::Value& jv);
	int setValInt(const char* attrName,int attrVal);
	int setValUint(const char* attrName,unsigned int attrVal);
	int setValStr(const char* attrName,const char* attrVal);
	int setValDbl(const char* attrName,double attrVal);
	CMsgAttr& operator[](const char* attrName);
	int operator=(const char* attrVal);
	int operator=(const string& attrVal);
	int operator=(int attrVal);
	int operator=(unsigned int attrVal);
	int operator=(double attrVal);
};
#endif
