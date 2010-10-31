#include "RootClassMethodArg.hpp"
#include "CPPNetTypeMapper.hpp"

#include "TMethodArg.h"

#include <sstream>

using std::string;
using std::ostringstream;

/// Counds argument numbers!
int RootClassMethodArg::_counter = 0;

void RootClassMethodArg::reset_arg_counter()
{
	_counter = 1;
}

RootClassMethodArg::RootClassMethodArg(void)
: _root_arg(0), _index (_counter)
{
	_counter++;
}

RootClassMethodArg::RootClassMethodArg(TMethodArg *arg)
: _root_arg(arg), _index(_counter)
{
	_counter++;
}

///
/// Return the CPP typename
///
string RootClassMethodArg::CPPTypeName (void) const
{
	return _root_arg->GetFullTypeName();
}

///
/// Return the raw CPP class name (fi any) without things like "*", etc.
///
string RootClassMethodArg::RawCPPTypeName() const
{
	return _root_arg->GetTypeName();
}

///
/// Return the NET typename
///
string RootClassMethodArg::NETTypeName (void) const
{
	return CPPNetTypeMapper::instance()->GetNetTypename(CPPTypeName());
}

///
/// Do we know about this type?
///
bool RootClassMethodArg::can_be_translated (void) const
{
  return CPPNetTypeMapper::instance()->has_mapping(CPPTypeName());
}

/// Return the NET interface typename
string RootClassMethodArg::NETInterfaceTypeName (void) const
{
	return CPPNetTypeMapper::instance()->GetNetInterfaceTypename(CPPTypeName());
}

///
/// Return the default name of the arugment
///
string RootClassMethodArg::get_argname() const
{
	string name (_root_arg->GetName());
	if (name.size() == 0) {
		ostringstream aname;
		aname << "arg" << _index;
		return aname.str();
	}
	if (name == "array") { // No, I'm not kidding.
	  name = "a_array";
	}
	return name;
}
