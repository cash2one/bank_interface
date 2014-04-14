#ifndef KS_8583_READER_H
#define KS_8583_READER_H
#include <map>

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(Typename)      \
  Typename(const Typename&);                    \
  void operator=(const Typename&)
#endif



class FieldNotFoundException : public std::exception
{
 public:
  FieldNotFoundException()
  {}
  ~FieldNotFoundException() throw()
  {}
};

#include "boost/array.hpp"
//typedef std::vector<char> BufferType;
typedef boost::array<char,4096> BufferType;

class Ks8583Define;
struct Ks8583FieldDefine;
// 8583 parser class 
class Ks8583Parser
{
 public:
  struct CalcMacParam
  {
    Ks8583Parser* parser;
    char *data;
    size_t data_len;
    char mac[64];
    unsigned char mackey[17];
    size_t mac_len;
  };
  static const size_t kMaxBitmapLength = 32;
  typedef std::map<size_t,std::string> FieldValueIndexMap; 
  typedef int (* CalcMacFunc)(CalcMacParam* param);
 private:
  // default constructor
  Ks8583Parser();

  static bool IsFileExists(const char *file_path);
  // return msg buffer bytes have been read, return 0 for no buffer
  // \param msg - message buffer
  // \param def - 8583 field define struct
  // \param value - buffer to save the data that has been extracted.
  size_t GetBufferValue(const char *msg,const Ks8583FieldDefine& def,BufferType &value);
  // return Output iterator for the buffer
  // \param def - 8583 field define struct
  // \param start_pos - buffer begin pos iterator
  BufferType::iterator SetBufferValue(const Ks8583FieldDefine& def,BufferType::iterator start_pos);
  // return output iterator for the buffer, copy string to buffer
  // \param str - source string
  // \param input - input iterator
  BufferType::iterator String2Buffer(const std::string& str,BufferType::iterator input);
  BufferType::iterator String2Buffer(const char *msg,size_t length,BufferType::iterator input);
  // copy buffer to string
  // \param begin - begin iterator
  // \param end - end iterator
  // \param str - output str
  void Buffer2String(BufferType::iterator begin,BufferType::iterator end,std::string &str);
 public:

  ~Ks8583Parser();
  // return 0 for success read 8583 definition file
  // \param define_file_path - define file 
  // \param define_name - define name which has been define in the file
  static int Load8583Define(const char *define_file_path,char *define_name);

  // call this function to free define class memory
  static void FreeAllDefines();
  // return Parser class instance , if <b>define_name</b> not found return NULL
  // Example:
  //   Ks8583Parser* parser = Ks8583Parser::GetInstance("dgposp");
  //   if( NULL == parser)
  //   {
  //     cout<<"cannot load 8583 define [dgposp]"<<endl;
  //     return;
  //   }
  // \param define_name - define name 
  static Ks8583Parser* GetInstance(const char *define_name);
  // return 0 for success parse message data , if failed return non-zero
  // \param msg - message data
  // \param msg_length - message data length (in byte)
  int UnpackData(const char *msg,size_t msg_length);
  int PackData(const BufferType::iterator begin,size_t *msg_length,const unsigned char* mackey=NULL);
  // clear field value
  void Clear();
  // return value of field by field index
  // if not found throw FieldNotFoundException
  std::string operator[](size_t field_index) const;
  // return value of field by field name
  // if not found throw FieldNotFoundException
  std::string operator[](const char *field_name) const;
  // set field value
  // return 0 for success , otherwise return -1 for failed
  int SetValueByIndex(size_t index,const std::string &value);
  int SetValueByIndex(size_t index,const char *str, size_t str_len);
  int SetValueByIndex(size_t index,int value);
  int SetValueByIndex(size_t index,
                      const BufferType::iterator begin,
                      const BufferType::iterator end);
  int SetValueByIndex(size_t index,const BufferType& buffer);
  int SetBufferByIndex(size_t index,const char *buffer, size_t buffer_len);

  int SetValueByName(const std::string &field_name,const std::string &value);
  int SetValueByName(const std::string &field_name,int value);
  int SetValueByName(const std::string &field_name,
                     const BufferType::iterator begin,
                     const BufferType::iterator end);
  int SetValueByName(const std::string &field_name,const BufferType& buffer);
  int SetBufferByName(const std::string &field_name,const char *buffer, size_t buffer_len);  
  // return true if the field exists
  bool HasField(size_t index);
  bool HasField(const std::string &field_name);

  // get field value
  int GetValueByIndex(size_t index,std::string &value);
  int GetValueByIndex(size_t index,int *value);
  int GetValueByName(const std::string &field_name,std::string &value);
  int GetValueByName(const std::string &field_name,int *value);
  // 
  BufferType::iterator DecodeBCD(const char* bcd,size_t length,BufferType::iterator buffer);
  BufferType::iterator EncodeBCD(const char* buffer,size_t length,BufferType::iterator bcd);
  BufferType::iterator EncodeHex(const char* bin_data,size_t length,BufferType::iterator hex);
  BufferType::iterator DecodeHex(const char* hex,size_t length,BufferType::iterator bin_data);

  /////////////////////////////////////////////////////////////////////////////
  inline void SetCalcMacCallback(CalcMacFunc func)
  {
    calc_mac_func_ = func;
  }
 private:
  // 数据包解析后数据
  FieldValueIndexMap field_values_;
  typedef std::map<std::string,Ks8583Define*> Field8583DefineMap;
  char bitmap_buffer_[kMaxBitmapLength];

  // 系统加载的 8583 协议定义结构
  static Field8583DefineMap s_field_defines_;
  // 当前使用
  Ks8583Define * field_define_;
  CalcMacFunc calc_mac_func_;

  DISALLOW_COPY_AND_ASSIGN(Ks8583Parser);
};

#endif
