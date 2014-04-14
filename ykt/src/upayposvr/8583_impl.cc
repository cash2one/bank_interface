#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <assert.h>
#include "ks_8583_reader.h"
#include "8583_impl.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
/// class Ks8583Define
Ks8583Define::Ks8583Define(): version_( 0 ), field_count_( 0 ),
  bitmap_index_( kInvalidIndex ), mac_index_( kInvalidIndex )
{
}
Ks8583Define::~Ks8583Define()
{
  FreeFieldDefines();
}
Ks8583FieldDefine& Ks8583Define::operator[]( size_t field_index )
{
  if( !HasFieldByIndex( field_index ) )
  {
    throw FieldNotFoundException();
  }
  FieldIndexDefineMap::const_iterator iter = field_index_map_.find( field_index );
  Ks8583FieldDefine* field = iter->second;
  return *field;
}
const Ks8583FieldDefine& Ks8583Define::operator[]( size_t field_index ) const
{
  if( !HasFieldByIndex( field_index ) )
  {
    throw FieldNotFoundException();
  }
  FieldIndexDefineMap::const_iterator iter = field_index_map_.find( field_index );
  const Ks8583FieldDefine* field = iter->second;
  return *field;
}
bool Ks8583Define::HasFieldByIndex( size_t field_index ) const
{
  FieldIndexDefineMap::const_iterator iter = field_index_map_.find( field_index );
  if( iter == field_index_map_.end() )
    return false;
  return true;
}
Ks8583FieldDefine& Ks8583Define::GetFieldByName( const char* field_name )
{
  FieldNameDefineMap::const_iterator iter = field_name_map_.find( field_name );
  if( iter == field_name_map_.end() )
  {
    throw FieldNotFoundException();
  }
  Ks8583FieldDefine* field = iter->second;
  return *field;
}
Ks8583FieldDefine& Ks8583Define::operator[]( const char* field_name )
{
  Ks8583FieldDefine& field = GetFieldByName( field_name );
  return field;
}
const Ks8583FieldDefine& Ks8583Define::operator[]( const char* field_name ) const
{
  //const Ks8583FieldDefine& field = (const Ks8583FieldDefine&)this->GetFieldByName(field_name);
  //return field;
  FieldNameDefineMap::const_iterator iter = field_name_map_.find( field_name );
  if( iter == field_name_map_.end() )
  {
    throw FieldNotFoundException();
  }
  Ks8583FieldDefine* field = iter->second;
  return *field;
}
bool Ks8583Define::HasFieldByName( const char* field_name ) const
{
  FieldNameDefineMap::const_iterator iter = field_name_map_.find( field_name );
  return ( iter != field_name_map_.end() );
}
std::string Ks8583Define::define_name() const
{
  return this->define_name_;
}
void Ks8583Define::set_define_name( const std::string& define_name )
{
  this->define_name_ = define_name;
}
std::string Ks8583Define::encoding() const
{
  return this->encoding_;
}
void Ks8583Define::set_encoding( const std::string& encoding )
{
  this->encoding_ = encoding;
}
std::string Ks8583Define::auth() const
{
  return this->auth_;
}
void Ks8583Define::set_auth( const std::string& auth )
{
  this->auth_ = auth;
}
int Ks8583Define::version() const
{
  return this->version_;
}
void Ks8583Define::set_version( int version )
{
  version_ = version;
}
std::string Ks8583Define::create_date() const
{
  return this->create_date_;
}
void Ks8583Define::set_create_date( const std::string& create_date )
{
  this->create_date_ = create_date;
}
size_t Ks8583Define::field_count() const
{
  return this->field_count_;
}
void Ks8583Define::set_field_count( size_t field_count )
{
  this->field_count_ = field_count;
}
size_t Ks8583Define::bitmap_index() const
{
  return this->bitmap_index_;
}
void Ks8583Define::set_bitmap_index( size_t index )
{
  this->bitmap_index_ = index;
}

bool Ks8583Define::AddFieldDefine( Ks8583FieldDefine* field_define )
{
  if( HasFieldByIndex( field_define->field_index ) )
    return false;
  field_index_map_.insert( FieldIndexDefineMap::value_type( field_define->field_index, field_define ) );
  field_name_map_.insert( FieldNameDefineMap::value_type( field_define->name, field_define ) );
  return true;
}
void Ks8583Define::FreeFieldDefines()
{
  FieldIndexDefineMap::const_iterator iter = field_index_map_.begin();
  for( ; iter != field_index_map_.end(); ++iter )
  {
    Ks8583FieldDefine* field = iter->second;
    delete field;
  }
  field_index_map_.clear();
  field_name_map_.clear();
}
size_t Ks8583Define::mac_index() const
{
  return mac_index_;
}
void Ks8583Define::set_mac_index( size_t index )
{
  mac_index_ = index;
}