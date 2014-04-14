#ifndef _MSGUTIL_H_
#define _MSGUTIL_H_
#include <map>
#include <vector>
#include "message/msgreader.h"
#include "message/msgwriter.h"
#include "pubfunc.h"
extern CMsgReader gReader;
extern CMsgWriter gWriter;
void db_trim(char *str);
namespace yktutil {

template<class T>
inline void assign_attr_copy(T& value,std::size_t max_len,const char* name);

inline void assign_attr_copy(char* value,std::size_t max_len,const char* name)
{
    strncpy(value,gReader.getAttrValAsStr(name).c_str(),max_len);
}

template<>
inline void assign_attr_copy<int>(int& value,std::size_t max_len,const char* name)
{
    value = gReader.getAttrValAsInt(name);
}
/*
template<>
inline void assign_attr_copy<char>(char& value,std::size_t max_len,const char* name)
{
    value = gReader.getAttrValAsInt(name);
}
*/
template<>
inline void assign_attr_copy<size_t>(size_t& value,std::size_t max_len,const char* name)
{
    value = gReader.getAttrValAsInt(name);
}

template<>
inline void assign_attr_copy<double>(double& value,std::size_t max_len,const char* name)
{
    value = gReader.getAttrValAsDbl(name);
}
template<>
inline void assign_attr_copy<std::string>(std::string& value,std::size_t max_len,const char* name)
{
	value = gReader.getAttrValAsStr(name);
}

template<class T>
inline void assign_col_copy(T& value,std::size_t max_len,const char* name);

inline void assign_col_copy(char* value,std::size_t max_len,const char* name)
{
    strncpy(value,gReader.getColValAsStr(name).c_str(),max_len);
}

template<>
inline void assign_col_copy<int>(int& value,std::size_t max_len,const char* name)
{
    value = gReader.getColValAsInt(name);
}
template<>
inline void assign_col_copy<std::string>(std::string& value,std::size_t max_len,const char* name)
{
	value = gReader.getColValAsStr(name);
}
/*
template<>
inline void assign_col_copy<char>(char& value,std::size_t max_len,const char* name)
{
    value = gReader.getColValAsInt(name);
}
*/
template<>
inline void assign_col_copy<size_t>(size_t& value,std::size_t max_len,const char* name)
{
    value = gReader.getColValAsInt(name);
}

template<>
inline void assign_col_copy<double>(double& value,std::size_t max_len,const char* name)
{
    value = gReader.getColValAsDbl(name);
}
template<class T>
inline void dbvar2col(const char* name,T& value);
inline void dbvar2col(const char* name,char* value)
{
	db_trim(value);
    gWriter.col[name]=value;
}
template<>
inline void dbvar2col<int>(const char* name,int& value)
{
    gWriter.col[name]=value;
}
template<>
inline void dbvar2col<size_t>(const char* name,size_t& value)
{
    gWriter.col[name]=value;
}
template<>
inline void dbvar2col<char>(const char* name,char& value)
{
    gWriter.col[name]=value;
}

template<>
inline void dbvar2col<double>(const char* name,double& value)
{
    gWriter.col[name]=D4U5(value);
}

} // namespace
#define ATTR2ST(st,var) do { yktutil::assign_attr_copy(st.var,sizeof(st.var)-1,#var);} while(0)
#define ATTR2VAR(var) do { yktutil::assign_attr_copy(var,sizeof(var)-1,#var);} while(0)
#define ATTR2STRING(var) do { yktutil::assign_attr_copy<std::string>(var,0,#var);} while(0)
#define COL2ST(st,col) do { yktutil::assign_col_copy(st.col,sizeof(st.col)-1,#col);} while(0)
#define COL2VAR(col) do { yktutil::assign_col_copy(col,sizeof(col)-1,#col);} while(0)
#define COL2STRING(var) do { yktutil::assign_col_copy<std::string>(var,0,#var);} while(0)
#define VAR2ATTR(col) do{ gWriter.attr[#col]=col;} while(0)
#define VAR2COL(c) do{ gWriter.col[#c]=c;} while(0)
#define DBVAR2COL(c) do{ yktutil::dbvar2col(#c,c);} while(0)
#define ST2ATTR(st,atr) do{gWriter.attr[#atr]=st.atr;} while(0)
#define ST2COL(st,c) do{ gWriter.col[#c]=st.c;} while(0)
#endif

