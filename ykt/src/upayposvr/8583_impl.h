#ifndef _8583_IMPL_H_
#define _8583_IMPL_H_
#include <string>
#include <map>
#include "ks_8583_reader.h"
typedef enum {FDT_BCD=0,FDT_ASCII=1,FDT_BIN,FDT_BITMAP,FDT_MAC} FieldDataType;
typedef enum {FF_FIX=0,FF_LLVAR,FF_LLLVAR} FieldFormat;
typedef enum {FA_LEFT = 0, FA_RIGHT} FieldAlign;


// �ֶζ���ṹ��
struct Ks8583FieldDefine
{
  // �ֶ�����
  size_t field_index;
  // �ֶ���
  std::string name;
  // �ֶ�����
  std::string desc;
  // ��������
  int data_type;
  // ���ݳ���
  size_t length;
  // ���ݸ�ʽ
  int format;
  // ���뷽ʽ
  int align;
  // �����ַ���Ĭ��Ϊ '0'
  char pad_char;
};



class Ks8583Define
{
 private:
  // Э�鶨�����ƣ�Ψһ������
  std::string define_name_;
  // �ַ���
  std::string encoding_;
  // ����
  std::string auth_;
  // �汾��
  int version_;
  // ������
  size_t field_count_;
  // ��������
  std::string create_date_;
  // λͼ�ֶ��������
  size_t bitmap_index_;
  // MAC �ֶ�
  size_t mac_index_;
  typedef std::map<std::string,Ks8583FieldDefine*> FieldNameDefineMap;
  typedef std::map<int,Ks8583FieldDefine*> FieldIndexDefineMap;

  // �ֶζ����ֶΣ��ֶ����ֵ�
  FieldNameDefineMap field_name_map_;
  // �ֶζ����ֵ䣬�ֶ������ֵ�
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
  // return Ks8583FieldDefine ����
  // ��� field_id �����ڣ��׳� FieldNotFoundException �쳣
  // \param field_id - �ֶ�������
  Ks8583FieldDefine& operator[](size_t field_id);
  const Ks8583FieldDefine& operator[](size_t field_id) const;
  // return true ��ʾ�ֶδ��ڣ�false ��ʾ������
  // \param field_index - �ֶ�������
  bool HasFieldByIndex(size_t field_index) const;
  // return Ks8583FieldDefine ����
  // ��� field_name �����ڣ��׳� FieldNotFoundException �쳣
  // \param field_name - �ֶ���
  Ks8583FieldDefine& GetFieldByName(const char *field_name);
  Ks8583FieldDefine& operator[](const char *field_name);
  const Ks8583FieldDefine& operator[](const char *field_name) const;
  // return true ��ʾ�ֶδ��ڣ�false ��ʾ�ֶβ�����
  // \param field_name - �ֶ���
  bool HasFieldByName(const char *field_name) const;

  // ��ȡ��������
  std::string define_name() const;
  void set_define_name(const std::string &define_name);
  // ��ȡ�ַ���
  std::string encoding() const;
  void set_encoding(const std::string &encoding);
  // ��ȡ����
  std::string auth() const;
  void set_auth(const std::string &auth);
  // ��ȡ�汾
  int version() const;
  void set_version(int version);
  // ��ȡ��������
  std::string create_date() const;
  void set_create_date(const std::string &create_date);
  // ��ȡ������
  size_t field_count() const;
  void set_field_count(size_t field_count);
  // ��ȡ�����ֶ�
  size_t bitmap_index() const;
  void set_bitmap_index(size_t index);
  // ��ȡMAC�ֶ�
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
