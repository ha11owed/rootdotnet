///
/// Code to help out with enums. Which are pesky.
///

#include "RootEnum.hpp"
#include "ROOTHelpers.h"
#include "ConverterErrorLog.hpp"

#include "Api.h"

#include <vector>
#include <string>
#include <stdexcept>

using std::vector;
using std::string;
using std::pair;
using std::runtime_error;

///
/// Update values -- see if we should search global scope to find some enum contents. :-)
///
void RootEnum::update_values() const
{
	///
	/// If it was done by hand, skip it.
	///

	if (_self_fill) {
		return;
	}

	///
	/// If there is already stuff in there, then bug out too
	///

	if (_values.size() > 0) {
		return;
	}

	vector<pair<string, unsigned int> > vals = ROOTHelpers::GetEnumValues(_name);
	_values.insert(_values.end(), vals.begin(), vals.end());
}

///
/// return the include filename
///
string RootEnum::include_filename() const
{
	init_cint_data();
	return _include_filename;
}

//
// What library are we in?
//
string RootEnum::LibraryName() const
{
	init_cint_data();
	return _library_name;
}

///
/// Load up cint stuff
///
void RootEnum::init_cint_data() const
{
	if (_cint_inited) {
		return;
	}
	_cint_inited = true;

	///
	/// Get the low-down from CINT
	///

	G__ClassInfo *cinfo = new G__ClassInfo (_name.c_str());
	if (!cinfo->IsValid()) {
		ConverterErrorLog::log_type_error(_name, "CINT doesn't seem to know about this enum!");
		_library_name = "libCore";
		_include_filename = "TObject.h";
	}

	///
	/// Hard time figuring out where the files are actually included here...
	///

	if (cinfo->DefFile() == 0) {
		_library_name = "libCore";
		_include_filename = "TObject.h";
	} else {
		_include_filename = cinfo->DefFile();
		int index = _include_filename.find_last_of ("/");
		index = index == string::npos ? -1 : index;
		_include_filename = _include_filename.substr(index+1);
	}
}

string RootEnum::Name() const
{
	if (_name == "enum ") {
		return "enum_";
	} else {
		return _name;
	}
}
