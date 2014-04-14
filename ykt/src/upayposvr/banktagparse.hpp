#ifndef BANKTAGPARSE_HPP
#define BANKTAGPARSE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)
#include<sstream>
#include<string>
#include<map>
#include<iomanip>
using namespace std;
#ifndef byte
typedef unsigned char byte;
#endif

class CBankTagParse
{
public:
  map<string,string> m_Tag;
  CBankTagParse()
  {
  }
  int getIntFromTwoByte(const unsigned char* bytes)
  {
    int r = 0, t = 0;
    t = bytes[0];
    r = t << 8;
    r |= bytes[1];
    return r;
  }
  char hex_to_num( char hex1char)
  {
    if( ( hex1char >= '0' ) && ( hex1char <= '9' ) ) return hex1char - '0';
    if( ( hex1char >= 'a' ) && ( hex1char <= 'f' ) ) return hex1char - 'a' + 10;
    if( ( hex1char >= 'A' ) && ( hex1char <= 'F' ) ) return hex1char - 'A' + 10;
    return 0;
  }
  char hex2_to_char(const char* str )
  {
    const  char c1( *( str ) );
    const  char c2( *( str+1 ) );
    return hex_to_num( c1 ) * 0x10 + hex_to_num( c2 );
  }      
  char xtod(char c)
  {
    if (c>='0' && c<='9') return c-'0';
    if (c>='A' && c<='F') return c-'A'+10;
    if (c>='a' && c<='f') return c-'a'+10;
    return c=0;        // not Hex digit
  }
  /*
  int hex_to_int(unsigned char hex[], int count)
  {
    int sum = 0;
    int i;
    int temp;

    for (i = 0; i < count; i++)
    {
      temp = (hex[i] & 0xF0) >> 4;
      sum = sum*16 + temp;
      temp = (hex[i] & 0x0F);
      sum = sum*16 + temp;
    }
    return sum;
  }
  */

  std::size_t get_int_from_hexstr(const char* data, std::size_t length =2)
  {
    std::string temp(data, length);
    return static_cast<std::size_t>(strtoul(temp.c_str(), NULL, 16));
  }
  std::size_t parseTagValue(const char* data,std::string& tag,std::string& tagValue)
  {
    std::size_t offset = 0;
    std::size_t length = 0;
    if(data[offset + 1] == 'F')
    {
      tag = string(data + offset, 4);
      offset += 4;
    }
    else
    {
      tag = string(data + offset, 2);
      offset += 2;
    }
    if(data[offset] < '8')
    {
      length = get_int_from_hexstr(data+offset);
      offset += 2;
    }
    else
    {
      std::size_t llen = get_int_from_hexstr(data+offset);
      llen &= 0x7F;
      offset += 2;
      length = get_int_from_hexstr(data + offset,llen);
      offset += llen * 2;
    }
    tagValue = string(data + offset, length * 2);
    offset += length * 2;
    return offset;
  }
  int parseData(const char* data,size_t datasize)
  {
    if(!m_Tag.empty())
      m_Tag.clear();
    for(std::size_t i = 0 ; i < datasize;)
    {
      std::string  tag;
      std::string  tagValue;
      size_t fl = parseTagValue(data + i,tag,tagValue);
      if(fl == 0)
        return 0;
      m_Tag[tag]  = tagValue;
      i += fl;
    }
    return m_Tag.size();
  }
  std::string getDumpString()
  {
    ostringstream oss;
    oss<<"{"<<endl;
    for(map<string,string>::iterator it= m_Tag.begin();it!=m_Tag.end();)
    {
      oss<<"\""<<it->first<<"\"=>\""<<it->second<<"\"";
      it++;
      if(it==m_Tag.end())
        break;
      oss<<","<<endl;
    }
    oss<<endl<<"}";
    return oss.str();
  }
};
#endif