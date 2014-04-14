#ifndef _8583_IMPL_H_
#define _8583_IMPL_H_
#include <string>
#include <map>
#include "ks_8583_reader.h"
typedef enum {FDT_BCD=0,FDT_ASCII=1,FDT_BIN,FDT_BITMAP,FDT_MAC} FieldDataType;
typedef enum {FF_FIX=0,FF_LLVAR,FF_LLLVAR} FieldFormat;
typedef enum {FA_LEFT = 0, FA_RIGHT} FieldAlign;


// 字段定义结构体
struct Ks8583FieldDefine
{
  // 字段索引
  size_t field_index;
  // 字段名
  std::string name;
  // 字段描述
  std::string desc;
  // 数据类型
  int data_type;
  // 数据长度
  size_t length;
  // 数据格式
  int format;
  // 补齐方式
  int align;
  // 补齐字符，默认为 '0'
  char pad_char;
};



class Ks8583Define
{
 private:
  // 协议定义名称，唯一的名称
  std::string define_name_;
  // 字符集
  std::string encoding_;
  // 作者
  std::string auth_;
  // 版本号
  int version_;
  // 域数量
  size_t field_count_;
  // 创建日期
  std::string create_date_;
  // 位图字段索引编号
  size_t bitmap_index_;
  // MAC 字段
  size_t mac_index_;
  typedef std::map<std::string,Ks8583FieldDefine*> FieldNameDefineMap;
  typedef std::map<int,Ks8583FieldDefine*> FieldIndexDefineMap;

  // 字段定义字段，字段名字典
  FieldNameDefineMap field_name_map_;
  // 字段定义字典，字段索引字典
  FieldIndexDefineMap field_index_map_;
 public:
  // max bitmap length in byte
  static const size_t kMaxBitmapLength = 32;
  // invalid index
  static const size_t kInvalidIndex = 0xFFFFFFFF;
  // default constructor
  Ks8583Define();
  // default destructor
  ~Ks8583Define();
  // free all Ks8583FieldDefine memory
  void FreeFieldDefines();
  // return Ks8583FieldDefine 引用
  // 如果 field_id 不存在，抛出 FieldNotFoundException 异常
  // \param field_id - 字段索引号
  Ks8583FieldDefine& operator[](size_t field_id);
  const Ks8583FieldDefine& operator[](size_t field_id) const;
  // return true 表示字段存在，false 表示不存在
  // \param field_index - 字段索引号
  bool HasFieldByIndex(size_t field_index) const;
  // return Ks8583FieldDefine 引用
  // 如果 field_name 不存在，抛出 FieldNotFoundException 异常
  // \param field_name - 字段名
  Ks8583FieldDefine& GetFieldByName(const char *field_name);
  Ks8583FieldDefine& operator[](const char *field_name);
  const Ks8583FieldDefine& operator[](const char *field_name) const;
  // return true 表示字段存在，false 表示字段不存在
  // \param field_name - 字段名
  bool HasFieldByName(const char *field_name) const;

  // 获取定义名称
  std::string define_name() const;
  void set_define_name(const std::string &define_name);
  // 获取字符集
  std::string encoding() const;
  void set_encoding(const std::string &encoding);
  // 获取作者
  std::string auth() const;
  void set_auth(const std::string &auth);
  // 获取版本
  int version() const;
  void set_version(int version);
  // 获取创建日期
  std::string create_date() const;
  void set_create_date(const std::string &create_date);
  // 获取域数量
  size_t field_count() const;
  void set_field_count(size_t field_count);
  // 获取索引字段
  size_t bitmap_index() const;
  void set_bitmap_index(size_t index);
  // 获取MAC字段
  size_t mac_index() const;
  void set_mac_index(size_t index);
  // return true for success add , otherwise return false
  // \param field_define - field
  bool AddFieldDefine(Ks8583FieldDefine* field_define);
  // bitmap function
  bool IsFieldSet(const char *bitmap,size_t index);
  void SetField(char *bitmap,size_t index);
 private:
  DISALLOW_COPY_AND_ASSIGN(Ks8583Define);
};


#endif // _8583_IMPL_H_
