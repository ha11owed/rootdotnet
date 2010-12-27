#include "ROOTHelpers.h"
#include "ConverterErrorLog.hpp"

#include <iostream>
#include <algorithm>
#include <exception>
#include <iterator>

#include "TROOT.h"
#include "TClass.h"
#include "TClassTable.h"
#include "TSystem.h"
#include "TGlobal.h"

#include "Api.h"

using std::string;
using std::vector;
using std::map;
using std::find;
using std::for_each;
using std::pair;
using std::make_pair;
using std::transform;
using std::back_inserter;

using std::cout;
using std::endl;
using std::exception;

std::map<std::string, std::vector<std::pair<std::string, unsigned int> > > ROOTHelpers::_all_enums;


class load_library
{
public:
	inline void operator() (const string &lib)
	{
		int result = gSystem->Load (lib.c_str());
		if (result != 0 && result != 1) {
			throw exception (("Failed to load library '" + lib + "'").c_str());
		}
	}
};

class extract_enum_classname {
public:
	inline extract_enum_classname (vector<string> &class_names)
		: _classes (class_names)
	{}
	inline void operator() (const pair<string, vector<pair<string, int> > > &item)
	{
		_classes.push_back(item.first);
	}
private:
	vector<string> &_classes;
};


///
/// Scan root's list of classes for ones that are in the passed in libraries.
///
vector<string> ROOTHelpers::GetAllClassesInLibraries(const vector<string> &library_names)
{
	///
	/// First, make sure all the libraries are loaded...
	///

	for_each (library_names.begin(), library_names.end(), load_library());

	///
	/// Re-do the library names into something that we will be able to use below. Mostly, this means
	/// removing any directories and .dll or .so's.
	///

	vector<string> shortLibNames;
	transform(
		library_names.begin(), library_names.end(),
		back_inserter<vector<string> >(shortLibNames),
		[] (const string &s) -> string {
			string lib (s);
			auto lastDir = lib.find_last_of('\\');
			if (lastDir != lib.npos) {
				lib = lib.substr(lastDir+1);
			}
			auto lastDot = lib.find_last_of('.');
			if (lastDot != lib.npos) {
				lib = lib.substr(0, lastDot);
			}
			return lib;
	});

	///
	/// Great. Next, get the list of classes root knows about and filter them by the list of libraries that
	/// we have been asked about...
	///

	vector<string> results;

	int num_classes = gClassTable->Classes();
	gClassTable->Init();
	for (int i = 0; i < num_classes; i++)
	{
		string class_name = gClassTable->Next();
		if (class_name.find("::") != class_name.npos) {
			ConverterErrorLog::log_type_error (class_name, "Private namespace not supported yet.");
			continue; // Don't do private namespace for now.
		}
		if (class_name.find("<") != class_name.npos) {
			ConverterErrorLog::log_type_error (class_name, "Tempalte classes not supported yet.");
			continue; // Don't do templates for now.
		}
		if (!IsClass(class_name)) {
			continue;
		}

		string shared_library (GetClassLibraryName(class_name));

		if (find(shortLibNames.begin(), shortLibNames.end(), shared_library) != shortLibNames.end()) {
			results.push_back (class_name);
		}
	}

	return results;
}

map<string, string> gClassLibraryNameCache;

void ROOTHelpers::ForceClassLibraryname (const std::string &class_name, const std::string &library_name)
{
	gClassLibraryNameCache[class_name] = library_name;
}


string ROOTHelpers::GetClassLibraryName (const std::string &class_name)
{
	if (gClassLibraryNameCache.find(class_name) != gClassLibraryNameCache.end()) {
		return gClassLibraryNameCache[class_name];
	}

	TClass *cls = TClass::GetClass(class_name.c_str(), true);
	if (!cls || !cls->GetSharedLibs()) {
		gClassLibraryNameCache[class_name] = "";
		return "";
	}

	string shared_libraries (cls->GetSharedLibs());
	string shared_library = shared_libraries;
	int first_space = shared_libraries.find(" ");
	if (first_space != shared_libraries.npos) {
		shared_library = shared_libraries.substr(0, first_space);
	}

	int dot_index = shared_library.find(".");
	if (dot_index != shared_library.npos) {
		shared_library = shared_library.substr(0, dot_index);
	}

	gClassLibraryNameCache[class_name] = shared_library;
	return shared_library;
}

/// True if this guy is a class, actually!
bool ROOTHelpers::IsClass (const std::string &class_name)
{
	TClass *cls = TClass::GetClass(class_name.c_str(), true);
	if (!cls) {
		return false;
	}

	/// Make sure there is a dictionary loaded for this guy!

	if (cls->GetClassInfo() == 0) {
	  return false;
	}

	/// Now see if it is declared as a class.

	return (cls->Property() & kIsClass) != 0;
}

/// True if this guy is an enum!
bool ROOTHelpers::IsEnum (const std::string &enum_name)
{
	G__ClassInfo *info = new G__ClassInfo (enum_name.c_str());
	bool is_enum = true;
	if (!info->IsValid()) {
		is_enum = false;
	} else {
		is_enum = (info->Property() & kIsEnum) != 0;
	}
	delete info;
	return is_enum;
}

	/// Return all globalls we can find
map<string, vector<string> > ROOTHelpers::GetAllGlobals()
{
	TIter iterator (gROOT->GetListOfGlobals());
	TGlobal *g;
	map<string, vector<string> > result;
	while ((g = static_cast<TGlobal*>(iterator.Next())) != 0) {
		if ((g->Property() & kIsEnum) == 0) {
			result[g->GetTypeName()].push_back(g->GetName());
		}
	}
	return result;
}

/// Helper to rip off the first item in the list.
class grab_enum_name
{
public:
	void operator() (const pair<string, vector<pair<string, unsigned int> > > &item)
	{
		_result.push_back(item.first);
	}
	vector<string> result(void) {return _result;}
private:
	vector<string> _result;
};

/// Return all the enums we can find
vector<string> ROOTHelpers::GetAllEnums()
{
	LoadAllEnums();
	return for_each (_all_enums.begin(), _all_enums.end(), grab_enum_name()).result();
}

/// Load up the internal cache if not done already
void ROOTHelpers::LoadAllEnums(void)
{
	if (_all_enums.size() > 0) {
		return;
	}

	TIter iterator (gROOT->GetListOfGlobals());
	TGlobal *g;
	while ((g = static_cast<TGlobal*>(iterator.Next())) != 0) {
		if ((g->Property() & kIsEnum) != 0) {
			const unsigned int *pvalue = (const unsigned int *)g->GetAddress();
			string enum_name(g->GetTypeName());
			if (enum_name.find("::") != enum_name.npos) {
				continue;
			}
			_all_enums[enum_name].push_back(make_pair(string(g->GetName()), *pvalue));
		}
	}
}

///
/// Return the enum values for a particular enum dude.
const vector<pair<string, unsigned int> > &ROOTHelpers::GetEnumValues(const std::string &enum_type)
{
	LoadAllEnums();
	return _all_enums[enum_type];
}
