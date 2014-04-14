#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <map>
#include <string.h>
#include <assert.h>
#include "json/json.h"
#include "ks_8583_reader.h"
#include "8583_impl.h"
#include "logger_imp.h"
using namespace std;
////////////////////////////////////////////////////////////////////////////////
/// class KsJsonReader

class KsJsonReader
{
 private:
#define ERROR_MESSAGE(msg) do { last_error_message_.str(""); last_error_message_<<msg; }while(0)
  std::stringstream last_error_message_;
  // return 0 for successs , failed return non-zero
  // \param root - Json Node
  // \param define_ptr - define class
  int RecursiveLoad(Json::Value &root,Ks8583Define *define_ptr);
  inline int DataTypeStr(const std::string &data_type)
  {
    if("ascii" == data_type)
    {
      return FDT_ASCII;
    }
    else if("bcd" == data_type)
    {
      return FDT_BCD;
    }
    else if("bin" == data_type)
    {
      return FDT_BIN;
    }
    else if("bitmap" == data_type)
    {
      return FDT_BITMAP;
    }
    else if("mac" == data_type)
    {
      return FDT_MAC;
    }
    return -1;
  }
  int FormatStr(const std::string &format)
  {
    if("fix" == format)
    {
      return FF_FIX;
    }
    else if("llvar" == format)
    {
      return FF_LLVAR;
    }
    else if("lllvar" == format)
    {
      return FF_LLLVAR;
    }
    return -1;
  }
  int AlignStr(const std::string& align)
  {
    if("left" == align)
    {
      return FA_LEFT;
    }
    else if("right" == align)
    {
      return FA_RIGHT;
    }
    return -1;
  }
 public:
  KsJsonReader();
  ~KsJsonReader();
  int LoadFromFile(const char *define_file_path,Ks8583Define *define_ptr);
  const char* GetLastError(std::string& message);
};

KsJsonReader::KsJsonReader()
{
}
KsJsonReader::~KsJsonReader()
{
}
const char* KsJsonReader::GetLastError(std::string& message)
{
  message = last_error_message_.str();
  return message.c_str();
}
int KsJsonReader::LoadFromFile(const char *define_file_path,Ks8583Define *define_ptr)
{
  std::ifstream ifs(define_file_path,ifstream::binary|std::ios_base::in);

  char buffer[2048+1];
  stringstream content;
  size_t read_length;
  while(!ifs.eof())
  {
    read_length = sizeof(buffer) - 1;
    ifs.read(buffer,read_length);
    read_length = ifs.gcount();
    buffer[read_length]=0;
    content<<buffer;
  }
  ifs.close();
  Json::Reader reader;
  Json::Value root;
  if(!reader.parse(content.str(),root))
  {
    ERROR_MESSAGE("Paser Error : "<<reader.getFormatedErrorMessages());
    return -1;
  }
  int ret = RecursiveLoad(root,define_ptr);
  if(ret)
  {
    define_ptr->FreeFieldDefines();
    return ret;
  }
  Ks8583FieldDefine& field_def = define_ptr->operator []((size_t)0);

  if(field_def.length > Ks8583Define::kMaxBitmapLength)
  {
    define_ptr->FreeFieldDefines();
    ERROR_MESSAGE("Bitmap Length error");
    return -1;
  }
  return 0;
}
int KsJsonReader::RecursiveLoad(Json::Value &root,Ks8583Define *define_ptr)
{
  std::string value;
  value = root.get("encoding","GBK").asString();
  define_ptr->set_encoding(value);
  value = root.get("define_name","").asString();
  define_ptr->set_define_name(value);
  value = root.get("auth","").asString();
  define_ptr->set_auth(value);
  value = root.get("create_date","").asString();
  define_ptr->set_create_date(value);

  size_t f_count = root.get("field_count","0").asUInt();
  if( f_count < 2)
  {
    ERROR_MESSAGE("Field count must at least more than one fields");
    return -1;
  }
  define_ptr->set_field_count(f_count);

  Json::Value field_define_value = root.get("fields_define","");
  Json::Value::Members mbs = field_define_value.getMemberNames();
  for(size_t i = 0;i < mbs.size(); ++i)
  {
    Json::Value field = field_define_value[mbs[i]];
    Ks8583FieldDefine *field_define = new Ks8583FieldDefine();
    field_define->field_index = atoi(mbs[i].c_str());
    field_define->name = field.get("name","").asString();
    if(field_define->name.length() == 0)
    {
      ERROR_MESSAGE("Field index["<<field_define->field_index<<"] dose not have name attribute");
      return -1;
    }
    field_define->desc = field.get("desc",field_define->name).asString();
    
    value = field.get("data_type","").asString();
    field_define->data_type = DataTypeStr(value);
    if( field_define->data_type == -1)
    {
      ERROR_MESSAGE("Field ["<<field_define->name<<"] data type error["<<value<<"]");
      return -1;
    }
    if(field_define->data_type == FDT_BITMAP)
    {
      define_ptr->set_bitmap_index(field_define->field_index);
    }
    else if(field_define->data_type == FDT_MAC)
    {
      define_ptr->set_mac_index(field_define->field_index);
    }

    value = field.get("format","").asString();
    field_define->format = FormatStr(value);
    if( field_define->format == -1)
    {
      ERROR_MESSAGE("Field ["<<field_define->name<<"] data format error["<<field_define->format<<"]");
      return -1;
    }

    field_define->length = field.get("length",0).asInt();
    if(field_define->length <= 0)
    {
      ERROR_MESSAGE("Field ["<<field_define->name<<"] data length error["<<field_define->length<<"]");
      return -1;
    }

    field_define->pad_char = (char)field.get("pad",EOF).asInt();

    /*
      if((field_define->format == FF_LLVAR ||
      field_define->format == FF_LLLVAR)
      && field_define->pad_char == EOF)
      {
      ERROR_MESSAGE("Field ["<<field_define->name<<"] must specify pad-char");
      return -1;
      }
    */

    value = field.get("align","left").asString();
    field_define->align = AlignStr(value);
    if(field_define->align == -1)
    {
      ERROR_MESSAGE("Field ["<<field_define->name<<"] align error["<<value<<"]");
      return -1;
    }
    define_ptr->AddFieldDefine(field_define);
  }
  if(define_ptr->bitmap_index() == Ks8583Define::kInvalidIndex)
  {
    ERROR_MESSAGE("Bitmap field is not specified");
    return -1;
  }
  return 0;
}
bool Ks8583Define::IsFieldSet(const char *bitmap,size_t index)
{
  assert(index > 0);
  return (bitmap[(index-1) >> 3] & (0x80 >> ((index-1) % 8))) ? true : false;
}
void Ks8583Define::SetField(char *bitmap,size_t index)
{
  assert(index > 0);
  bitmap[(index-1) >> 3] |= (0x80 >> ((index - 1) % 8));
}

////////////////////////////////////////////////////////////////////////////////
/// class Ks8583Parser
Ks8583Parser::Field8583DefineMap Ks8583Parser::s_field_defines_;
Ks8583Parser::Ks8583Parser():field_define_(NULL),calc_mac_func_(NULL)
{
  memset(bitmap_buffer_,0,sizeof bitmap_buffer_);
}
Ks8583Parser::~Ks8583Parser()
{
}
int Ks8583Parser::Load8583Define(const char *define_file_path,char *define_name)
{
  if(!IsFileExists(define_file_path))
    return -1;
  KsJsonReader reader;
  Ks8583Define *define_ptr = new Ks8583Define();
  int ret = reader.LoadFromFile(define_file_path,define_ptr);
  if(ret)
  {
    delete define_ptr;
    std::string msg;
    cout<<reader.GetLastError(msg)<<std::endl;
    return -1;
  }
  s_field_defines_.insert(Field8583DefineMap::value_type(define_ptr->define_name(),define_ptr));
  strcpy(define_name,define_ptr->define_name().c_str());
  return 0;
}
void Ks8583Parser::FreeAllDefines()
{
  Field8583DefineMap::const_iterator iter = s_field_defines_.begin();
  for(;iter != s_field_defines_.end();++iter)
  {
    Ks8583Define *define_ptr = iter->second;
    delete define_ptr;
  }
  s_field_defines_.clear();
}
bool Ks8583Parser::IsFileExists(const char *file_path)
{
  std::ifstream ifs(file_path);
  if( ifs.fail() )
  {
    return false;
  }
  return true;
}
Ks8583Parser* Ks8583Parser::GetInstance(const char *define_name)
{
  Field8583DefineMap::const_iterator iter = s_field_defines_.find(define_name);
  if(iter == s_field_defines_.end())
    return NULL;
  Ks8583Parser *parser = new Ks8583Parser();
  parser->field_define_ = iter->second;
  return parser;
}
std::string Ks8583Parser::operator[](size_t field_index) const
{
  FieldValueIndexMap::const_iterator iter;
  iter = field_values_.find(field_index);
  if(iter == field_values_.end())
  {
    throw FieldNotFoundException();
  }
  return iter->second;
}
std::string Ks8583Parser::operator[](const char *field_name) const
{
  if(!field_define_->HasFieldByName(field_name))
  {
    throw FieldNotFoundException();
  }
  Ks8583FieldDefine& def = field_define_->operator [] (field_name);
  return this->operator[](def.field_index);
}
int Ks8583Parser::UnpackData(const char *msg,size_t msg_length)
{
  Clear();
  size_t field_index = 0,offset=0;
  size_t bitmap_index = field_define_->bitmap_index();
  size_t mac_index = field_define_->mac_index();
  Ks8583FieldDefine& bitmap_define = field_define_->operator[](bitmap_index); // bitmap field
  BufferType value;

  // fields before bitmap field
  for(size_t i = 0;i < bitmap_index; ++i)
  {
    if(field_define_->HasFieldByIndex(i))
    {
      Ks8583FieldDefine& current_def = field_define_->operator[](i);
      value.assign(0);
      offset += GetBufferValue(msg+offset,current_def,value);
      this->SetValueByIndex(current_def.field_index,value);
    }
  }
  // get bitmap field
  value.assign(0);
  offset += GetBufferValue(msg+offset,bitmap_define,value);
  for(size_t i = 0;i < bitmap_define.length;++i)
  {
    bitmap_buffer_[i] = value[i];
  }
  // get fields after bitmap field
  size_t mac_data_len = 0;
  char input_mac_data[64]={0};
  size_t input_mac_len = 0;
  for(size_t i = bitmap_index+1;i < field_define_->field_count(); ++i)
  {
    // if bitmap is set
    if(field_define_->HasFieldByIndex(i) &&
       field_define_->IsFieldSet(bitmap_buffer_,i))
    {
      value.assign(0);
      Ks8583FieldDefine& current_def = field_define_->operator[](i);
      mac_data_len = offset;
      offset += GetBufferValue(msg+offset,current_def,value);
      this->SetValueByIndex(current_def.field_index,value);
      if(mac_index == current_def.field_index)
      {
        // it must be the last one , break
        input_mac_len = offset-mac_data_len;
        memcpy(input_mac_data,msg+mac_data_len,input_mac_len);
        break;
      }
    }
  }
  if(offset != msg_length)
  {
  	LOG(ERROR,"length error:offset="<<offset<<",msg len="<<msg_length);
    return -1;
  }
#if 0
  if(input_mac_len>0)
  {
    // check MAC 
    if(!calc_mac_func_)
      return -3;
    CalcMacParam param={0};
    param.parser = this;
    param.data = const_cast<char*>(msg);
    param.data_len = mac_data_len;

    if(!calc_mac_func_(&param))
    {
      if(param.mac_len == 0)
        return -3;
      if(memcmp(param.mac,input_mac_data,param.mac_len)==0 
        && param.mac_len == input_mac_len)
        return 0;
      return -2;
    }
    else
    {
      return -4;
    }
  }
#endif
  return 0;
}
size_t Ks8583Parser::GetBufferValue(const char *msg,const Ks8583FieldDefine& def,
                                    BufferType &value)
{
  const char *p = msg;
  BufferType header_buffer;
  BufferType::iterator header_end = header_buffer.begin();
  size_t copy_length = 0,header_length = 0;
  switch(def.format)
  {
    case FF_FIX:
      copy_length = def.length;
      header_length = 0;
      break;
    case FF_LLLVAR:
      header_length = 2;
      break;
    case FF_LLVAR:
      header_length = 1;
      break;
    default:
      return 0;
  }
  if(header_length > 0)
  {
    header_buffer.assign(0);
    header_end = DecodeBCD(msg,header_length,header_buffer.begin());
    copy_length = (size_t)atoi(header_buffer.data());
  }
  // data length error!
  if(copy_length > def.length || copy_length == 0)
    return 0;
  std::string a;
  p = msg + header_length;
  switch(def.data_type)
  {
    case FDT_BITMAP:
      // for(size_t i = 0;i < copy_length;++i)
      //   value[i] = p[i];
      String2Buffer(p,copy_length,value.begin());
      return copy_length;
    case FDT_ASCII:
      // a = std::string(p,copy_length);
      // std::copy(value.begin(),a.begin(),a.end());
      String2Buffer(p,copy_length,value.begin());
      return (header_length + copy_length);
    case FDT_BIN:
    case FDT_MAC:
      // encode hex format
      this->EncodeHex(p,copy_length,value.begin());
      return (header_length + copy_length);
    case FDT_BCD:
      size_t bcd_length;
      if(copy_length % 2 == 0)
        bcd_length = copy_length/2;
      else
        bcd_length = (copy_length+1)/2;
      this->DecodeBCD(p,bcd_length,value.begin());
      value[copy_length]=0;
      return (header_length + bcd_length);
    default:
      return 0;
  }
}
BufferType::iterator Ks8583Parser::DecodeBCD(const char* bcd,size_t length,BufferType::iterator buffer)
{
  BufferType::iterator iter;
  size_t i;
  for(i=0,iter = buffer;i < length;++i)
  {
    *iter = ((bcd[i] >> 4) & 0x0F) + '0';
    ++iter;
    *iter = (bcd[i] & 0x0F) + '0';
    ++iter;
  }
  return iter;
}
BufferType::iterator Ks8583Parser::EncodeBCD(const char* buffer,size_t length,BufferType::iterator bcd)
{
  BufferType::iterator iter;
  size_t i;
  for(i = 0,iter = bcd;i < length; i+=2)
  {
    *iter = ((buffer[i] - '0')<<4) | (buffer[i+1] - '0');
    ++iter;
  }
  return iter;
}
BufferType::iterator Ks8583Parser::EncodeHex(const char* bin_data,size_t length,BufferType::iterator hex)
{
  size_t i;
  std::stringstream temp;
  char str[10] = {0};
  for(i = 0;i < length; ++i)
  {
    memset(str,0,sizeof str);
    sprintf(str,"%02X",(unsigned char)bin_data[i]);
    temp<<str;
  }
  return String2Buffer(temp.str(),hex);

}
BufferType::iterator Ks8583Parser::DecodeHex(const char* hex,size_t length,BufferType::iterator bin_data)
{
  size_t i;
  BufferType::iterator iter;
  char temp[3]={0};
  for(i = 0,iter = bin_data;i < length; i+=2)
  {
    memcpy(temp,hex+i,2);
    *iter = (char)strtoul(temp,NULL,16);
    ++iter;
  }
  return iter;
}

void Ks8583Parser::Clear()
{
  field_values_.clear();
  memset(bitmap_buffer_,0,sizeof bitmap_buffer_);
}
int Ks8583Parser::SetValueByIndex(size_t index,const std::string &value)
{
  if(!field_define_->HasFieldByIndex(index))
  {
  	LOG(ERROR,"not define No."<<index<<" field");
    return -1;
  }
  FieldValueIndexMap::iterator iter = field_values_.find(index);
  if(iter != field_values_.end())
  {
    field_values_.erase(iter);
  }
  Ks8583FieldDefine &def = field_define_->operator [](index);
  size_t value_length = value.length();
  if(def.data_type == FDT_BIN || def.data_type == FDT_MAC)
  {
    if(value_length % 2 != 0)
    {	
  	  LOG(ERROR,"No."<<index<<" bit or mac data length error");
      return -1;
    }
    value_length /= 2;
  }
  if( value_length > def.length)
  {  
  	LOG(ERROR,"No."<<index<<" value length["<<value_length<<"] greater than define");
    return -1;
  }
  if(def.format == FF_FIX && def.pad_char == EOF
     && value_length != def.length)
  {
	  LOG(ERROR,"No."<<index<<" fix and eof error");
      return -1;
  }
  field_values_.insert(FieldValueIndexMap::value_type(index,value));
  return 0;
}
int Ks8583Parser::SetValueByIndex(size_t index,const char *str, size_t str_len)
{
	std::string value(str,str_len);
  return SetValueByIndex(index,value);
}
int Ks8583Parser::SetValueByIndex(size_t index,int value)
{
  std::stringstream val;
  val<<value;
  return this->SetValueByIndex(index,val.str());
}
int Ks8583Parser::SetValueByIndex(size_t index,
                                  const BufferType::iterator begin,
                                  const BufferType::iterator end)
{
  std::string a(begin,end);
  return SetValueByIndex(index,a);
}
int Ks8583Parser::SetValueByIndex(size_t index,const BufferType& buffer)
{
  return SetValueByIndex(index,buffer.data());
}
int Ks8583Parser::SetBufferByIndex(size_t index,const char *buffer, size_t buffer_len)
{
  BufferType temp;
  temp.assign(0);
  BufferType::iterator end = EncodeHex(buffer,buffer_len,temp.begin());
  *end = 0;
  return SetValueByIndex(index,std::string(temp.data()));
}
int Ks8583Parser::SetValueByName(const std::string &field_name,const std::string &value)
{
  if(!field_define_->HasFieldByName(field_name.c_str()))
    return -1;
  Ks8583FieldDefine& def = field_define_->GetFieldByName(field_name.c_str());
  return SetValueByIndex(def.field_index,value);
}
int Ks8583Parser::SetValueByName(const std::string &field_name,int value)
{
  if(!field_define_->HasFieldByName(field_name.c_str()))
    return -1;
  Ks8583FieldDefine& def = field_define_->GetFieldByName(field_name.c_str());
  return SetValueByIndex(def.field_index,value);

}
int Ks8583Parser::SetValueByName(const std::string &field_name,
                                 const BufferType::iterator begin,
                                 const BufferType::iterator end)
{
  std::string a(begin,end);
  return SetValueByName(field_name,a);
}
int Ks8583Parser::SetValueByName(const std::string &field_name,const BufferType& buffer)
{
  return SetValueByName(field_name,buffer.data());
}
int Ks8583Parser::SetBufferByName(const std::string &field_name,const char *buffer, size_t buffer_len)
{
  BufferType temp;
  temp.assign(0);
  BufferType::iterator end = EncodeHex(buffer,buffer_len,temp.begin());
  *end = 0;
  return SetValueByName(field_name,std::string(temp.data()));
}

// return true if the field exists
bool Ks8583Parser::HasField(size_t index)
{
  return (this->field_values_.find(index) != this->field_values_.end());
}
bool Ks8583Parser::HasField(const std::string &field_name)
{
  if(field_define_->HasFieldByName(field_name.c_str()))
  {
    Ks8583FieldDefine &def = field_define_->GetFieldByName(field_name.c_str());
    return HasField(def.field_index);
  }
  return false;
}
int Ks8583Parser::PackData(BufferType::iterator begin,size_t *msg_length,const unsigned char* mackey)
{
  size_t field_index,bitmap_index,mac_index;
  BufferType::iterator offset=begin;
  bitmap_index = field_define_->bitmap_index();
  mac_index = field_define_->mac_index();
  for(field_index = 0; field_index < bitmap_index; ++field_index)
  {
    if(field_define_->HasFieldByIndex(field_index))
    {
      // copy to
      Ks8583FieldDefine& def = field_define_->operator[](field_index);
      BufferType::iterator next_offset;
      next_offset = SetBufferValue(def,offset);
      if(next_offset == offset)
        return -1;
      offset = next_offset;
    }
  }
  // copy bitmap
  Ks8583FieldDefine& bitmap_def = field_define_->operator[](bitmap_index);
  memset(bitmap_buffer_,0,sizeof bitmap_buffer_);
  memcpy(offset,bitmap_buffer_,bitmap_def.length);
  BufferType::iterator bitmap_offset = offset;
  offset += bitmap_def.length;
  // copy fields
  for(field_index = bitmap_index+1; field_index < field_define_->field_count(); ++field_index)
  {
    if(HasField(field_index) && field_define_->HasFieldByIndex(field_index))
    {
     
      // copy to
      Ks8583FieldDefine& def = field_define_->operator[](field_index);
      BufferType::iterator next_offset;
      next_offset = SetBufferValue(def,offset); 
      if(next_offset != offset)
        field_define_->SetField(bitmap_buffer_,field_index);
      offset = next_offset;
    }
  }
  // 计算MAC
  if(mackey)
  {
    if(!calc_mac_func_ || mac_index == Ks8583Define::kInvalidIndex)
      return -2;
    field_define_->SetField(bitmap_buffer_,mac_index);
    memcpy(bitmap_offset,bitmap_buffer_,bitmap_def.length);

    CalcMacParam param={0};
    param.data = begin;
    memcpy(param.mackey,mackey,16);
    param.data_len = offset - begin;
    param.parser = this;
    if(calc_mac_func_(&param))
      return -3;
    
    this->SetBufferByIndex(mac_index,param.mac,param.mac_len);
    Ks8583FieldDefine& def = field_define_->operator[](mac_index);
    BufferType::iterator next_offset;
    next_offset = SetBufferValue(def,offset); 
    offset = next_offset;
  }
  else
  {
    // copy bitmap again
    memcpy(bitmap_offset,bitmap_buffer_,bitmap_def.length);
  }
  
  
  *msg_length = offset - begin;
  return 0;
}

BufferType::iterator Ks8583Parser::SetBufferValue(const Ks8583FieldDefine& def,BufferType::iterator start_pos)
{
  
  BufferType::iterator header_pos;
  std::stringstream header_str;
  size_t data_length;
  header_pos = start_pos;
  data_length = field_values_[def.field_index].length();
  if(data_length == 0)
    return start_pos;
  if(def.data_type == FDT_BIN)
  {
    if(data_length % 2 != 0)
      return start_pos;
    data_length /= 2;
  }
  switch(def.format)
  {
    case FF_FIX:
      header_pos = start_pos;
      break;
    case FF_LLVAR:
      header_str<<std::setw(2)<<std::setfill('0')<<data_length;
      header_pos = EncodeBCD(header_str.str().c_str(),2,header_pos);
      break;
    case FF_LLLVAR:
      header_str<<std::setw(4)<<std::setfill('0')<<data_length;
      header_pos = EncodeBCD(header_str.str().c_str(),4,header_pos);
      break;
    default:
      return start_pos;
  }
  size_t align_length = 0;
  size_t copy_length = data_length;
  if(def.length > data_length)
  {
    switch(def.data_type)
    {
      case FDT_BITMAP:
      case FDT_ASCII:
        align_length = def.length - data_length;
        break;
        // bin , bcd format is compressed data
      case FDT_BIN:
      case FDT_MAC:
        copy_length = data_length * 2;
        break;
      case FDT_BCD:
        if(data_length % 2 != 0)
          copy_length = data_length + 1;
        align_length = (def.length - copy_length) / 2;
        break;
      default:
        return start_pos;
    }
  }
  // 右对齐
  if (align_length > 0 && def.align == FA_RIGHT && def.format == FF_FIX)
  {
    std::fill(header_pos,header_pos+align_length,def.pad_char);
    header_pos+=align_length;
  }
  std::string copy_value;
  // 数据域
  switch(def.data_type)
  {
    case FDT_BITMAP: // not support bitmap field
      return start_pos;
    case FDT_ASCII:
      //header_pos = std::copy(header_pos,field_values_[def.field_index].begin(),field_values_[def.field_index].begin()+copy_length);
      header_pos = String2Buffer(field_values_[def.field_index],header_pos);
      break;
    case FDT_BCD:
      // FIXME : 因为左补或右补 的问题，这里效率比较低
      
      if(data_length % 2 != 0)
      {
        if(def.align == FA_LEFT)
          copy_value = field_values_[def.field_index] + "0";
        else
        {
          copy_value = "0";
          copy_value += field_values_[def.field_index];
        }
      }
      else
      {
        copy_value = field_values_[def.field_index];
      }
      header_pos = EncodeBCD(copy_value.c_str(),copy_length,header_pos);
      break;
    case FDT_BIN:
    case FDT_MAC:
      header_pos = DecodeHex(field_values_[def.field_index].c_str(),copy_length,header_pos);
      break;
    default:
      return start_pos;
  }
  // 左对齐
  if (align_length > 0 && def.align == FA_LEFT && def.format == FF_FIX)
  {
    std::fill(header_pos,header_pos+align_length,def.pad_char);
    header_pos+=align_length;
  }
  return header_pos;
}
BufferType::iterator Ks8583Parser::String2Buffer(const std::string& str,BufferType::iterator input)
{
  for(size_t i = 0;i < str.length(); ++i)
  {
    *input = str[i];
    ++input;
  }
  return input;
}
void Ks8583Parser::Buffer2String(BufferType::iterator begin,BufferType::iterator end,std::string &str)
{
  str.assign(begin,end);
}
BufferType::iterator Ks8583Parser::String2Buffer(const char *msg,size_t length,BufferType::iterator input)
{
  for(size_t i = 0;i < length; ++i)
  {
    *input = msg[i];
    ++input;
  }
  return input;
}
int Ks8583Parser::GetValueByIndex(size_t index,std::string &value)
{
  value="";
  if(!this->HasField(index))
    return -1;
  value = field_values_[index];
  return 0;
}
int Ks8583Parser::GetValueByIndex(size_t index,int *value)
{
  std::string strvalue;
  if(GetValueByIndex(index,strvalue))
    return -1;
  *value = atoi(strvalue.c_str());
  return 0;
}
int Ks8583Parser::GetValueByName(const std::string &field_name,std::string &value)
{
  if(!field_define_->HasFieldByName(field_name.c_str()))
  {
    return -1;
  }
  Ks8583FieldDefine& def = field_define_->operator [](field_name.c_str());
  return GetValueByIndex(def.field_index,value);
}
int Ks8583Parser::GetValueByName(const std::string &field_name,int *value)
{
  std::string strvalue;
  if(GetValueByName(field_name,strvalue))
    return -1;
  *value = atoi(strvalue.c_str());
  return 0;
}
