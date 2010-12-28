///
/// Implementation for a field that we are keeping track of
///

#include "RootClassField.hpp"
#include "CPPNetTypeMapper.hpp"
#include "ConverterErrorLog.hpp"
#include "ROOTHelpers.h"

#include <TDataMember.h>
#include <TClass.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

///
/// Null initalizer. Normally this shouldn't happen. :-)
///
RootClassField::RootClassField(void)
	: _root_field (0)
{
}

///
/// Track a field that we can (we hope) access.
///
RootClassField::RootClassField(TDataMember *f)
	: _root_field(f)
{
}

RootClassField::~RootClassField(void)
{
}

string RootClassField::NETName() const
{
	return _root_field->GetName();
}

string RootClassField::NETType() const
{
	return CPPNetTypeMapper::instance()->GetNetInterfaceTypename(_root_field->GetTypeName());
}

string RootClassField::CPPType() const
{
	return _root_field->GetTypeName();
}

///
/// Returns the name of the class that this guy is defined in.
///
string RootClassField::ClassOfFieldDefinition(void) const
{
	return _root_field->GetClass()->GetName();
}

///
/// Record who our parent class is
///
void RootClassField::ResetParent (const RootClassInfo *parent)
{
}

///
/// is_less_than
///
///  Compare left and right and decide who is greater. For now we are using
/// a straight name based comparison.
///
bool RootClassField::is_less_than(const RootClassField &right) const
{
	return string(_root_field->GetName()) < string(right._root_field->GetName());
}

///
/// CPPName - the name that is valid in C++
///
string RootClassField::CPPName() const
{
	return _root_field->GetName();
}

///
/// Get all the referenced class types. This is actually pretty simple here - as
/// there is only the field type (no worries about arguments, etc.). The C++ names
/// are returned.
///
vector<string> RootClassField::get_all_referenced_raw_types(void) const
{
	vector<string> result;
	result.push_back (_root_field->GetTypeName());
	return result;
}

vector<string> RootClassField::get_all_referenced_root_types(void) const
{
	auto all (get_all_referenced_raw_types());
	vector<string> result;
	for (unsigned int i = 0; i < all.size(); i++) {
		auto allTypes = ROOTHelpers::GetTemplateArguments(all[i]);
		for (int j = 0; j < allTypes.size(); j++) {
			if (ROOTHelpers::IsClass(allTypes[j])) {
				result.push_back(allTypes[j]);
			}
		}
	}
	return result;
}

///
/// Can we translate this guy?
///
bool RootClassField::can_be_translated() const
{
  vector<string> all_types (get_all_referenced_raw_types());

  for (int i = 0; i < all_types.size(); i++) {
    if (!CPPNetTypeMapper::instance()->has_mapping(all_types[i])) {
	  ConverterErrorLog::log_type_error(all_types[i], "No type converter");
      return false;
    }
  }
}
