///
/// Code that contains common configuration.
///

#include "WrapperConfigurationInfo.hpp"
#include "CPPNetTypeMapper.hpp"
#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"
#include "RootClassMethodArg.hpp"
#include "ConverterErrorLog.hpp"
#include "ROOTHelpers.h"

#include "TTSimpleType.hpp"
#include "TPointerSimpleType.hpp"
#include "TTCPPString.hpp"
#include "TTSimpleReference.hpp"
#include "TArrayOfChar.hpp"
#include "TVoidPointer.hpp"
#include "TVectorArray.hpp"

#include "TROOT.h"
#include "TDataType.h"
#include "Api.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "shlobj.h"

using std::vector;
using std::string;
using std::set;
using std::map;
using std::find;
using std::cout;
using std::endl;
using std::ifstream;
using std::getline;

std::map<std::string, std::vector<std::string> > WrapperConfigurationInfo::_allowed_library_links;
std::map<std::string, std::vector<std::string> > WrapperConfigurationInfo::_disallowed_library_links;
bool WrapperConfigurationInfo::_allowed_library_links_ok = false;

void InitTypeDefs (void);

/// Remove all classes that are in the bad class list file
vector<string> WrapperConfigurationInfo::RemoveBrokenClasses (const vector<string> &class_list)
{
	///
	/// Load in the bad headers list
	///

	set<string> bad_headers;
	{
		ifstream input ("bad_headers.txt");
		string line;
		while (!getline(input, line).fail()) {
			bad_headers.insert(line);
		}
	}

	///
	/// Now, just march through and find the bad ones!
	///

	vector<string> result;
	for (unsigned int i = 0; i < class_list.size(); i++) {
	  if (class_list[i] == "string") {
		ConverterErrorLog::log_type_error(class_list[i], "Can't convert STL classes like string!");
		continue;
	  }
		RootClassInfo &info (RootClassInfoCollection::GetRootClassInfo(class_list[i]));
		if (bad_headers.find(info.include_filename()) == bad_headers.end()) {
			result.push_back(class_list[i]);
		} else {
			ConverterErrorLog::log_type_error(class_list[i], "Class was on bad-class list (its header doesn't build stand alone)");
		}
	}

	return result;
}

///
/// Return a list of methods that we cna't translate for whatever reason
///
set<string> WrapperConfigurationInfo::GetListOfBadMethods()
{
	set<string> methods_to_skip;

	/// TStyleManager::fgStyleManager
	methods_to_skip.insert("TStyleManager::GetSM");

	/// Some bad globals in RooFitCore library
	// RooAbsPdf::_globalSelectComp
	methods_to_skip.insert("RooAbsPdf::isSelectedComp");
	// RooNumber::infinity (RooBinning) - used in the ctor of RooBinning
	methods_to_skip.insert("RooBinning::RooBinning");
	methods_to_skip.insert("RooNormListManager::setVerbose");
	methods_to_skip.insert("RooErrorVar::removeMin");
	methods_to_skip.insert("RooErrorVar::removeMax");
	methods_to_skip.insert("RooErrorVar::removeRange");
	methods_to_skip.insert("RooRealVar::removeMin");
	methods_to_skip.insert("RooRealVar::removeMax");
	methods_to_skip.insert("RooRealVar::removeRange");
	// RooAbsReal::_doLogEvalError
	methods_to_skip.insert("RooAbsReal::evalErrorLoggingEnabled");
	methods_to_skip.insert("RooAbsReal::enableEvalErrorLogging");
	// RooAbsReal::_evalErrorList
	methods_to_skip.insert("RooAbsReal::numEvalErrorItems");
	methods_to_skip.insert("RooAbsReal::evalErrorIter");
	
	// TEveLine::fgDefaultSmooth
	methods_to_skip.insert("TEveLine::GetDefaultSmooth");
	methods_to_skip.insert("TEveLine::SetDefaultSmooth");

	// TEveTrackProjected::fgBreakTracks
	methods_to_skip.insert("TEveTrackProjected::GetBreakTracks");
	methods_to_skip.insert("TEveTrackProjected::SetBreakTracks");

	/// Bad globals from libTable
	// TDataSet(fgMainSet)
	methods_to_skip.insert("TDataSet::GetMainSet");
	methods_to_skip.insert("TDataSet::AddMain");
	// TTableDescriptor(fgColDescriptors)
	methods_to_skip.insert("TTableDescriptor::GetDescriptorPointer");
	methods_to_skip.insert("TTableDescriptor::SetDescriptorPointer");
	methods_to_skip.insert("TChair::GetRowDescriptors");
	// TDataSetIter::fgNullDataSet
	methods_to_skip.insert("TDataSetIter::operator* ()");
	methods_to_skip.insert("TDataSetIter::GetNullSet");
	// TIndexTable::fgColDescriptors
	methods_to_skip.insert("TIndexTable::GetDescriptorPointer");
	methods_to_skip.insert("TIndexTable::SetDescriptorPointer");

	/// Bad globsl from libRGL
	// TGLSelectBuffer::fbMaxSize is another undefined symbol.
	methods_to_skip.insert("TGLSelectBuffer::CanGrow");
	// TGLUtil::fgDrawQuality is missing
	methods_to_skip.insert("TGLUtil::SetDrawQuality");
	methods_to_skip.insert("TGLUtil::ResetDrawQuality");
	// TGLUtil::fgDefaultDrawQuality is missing
	methods_to_skip.insert("TGLUtil::GetDefaultDrawQuality");
	methods_to_skip.insert("TGLUtil::SetDefaultDrawQuality");
	// TGLUtil::fgColorLockCount
	methods_to_skip.insert("TGLUtil::IsColorLocked");
	
	/// libVMC
	// Missing TVirtualMC::fgMC;
	methods_to_skip.insert("TVirtualMC::GetMC");

	/// libGeomBuilder
	// TGeoTreeDialog::fgSelectedObj
	methods_to_skip.insert("TGeoTreeDialog::GetSelected");

	/// libFitPanelWrapper
	// TFitEditor::fgFitDialog is missing
	methods_to_skip.insert("TFitEditor::GetFP");

	/// Amb method: Error
	methods_to_skip.insert("TFitResult::Error");
	methods_to_skip.insert("FitResult::Error");
	methods_to_skip.insert("ROOT::Fit::FitResult::Error");
	methods_to_skip.insert("TObject::Error");

	/// TFileCacheRead - some circular definitions
	methods_to_skip.insert("TFileCacheRead::AddBranch");

	/// My code is messing up the covar returns on this guy
	methods_to_skip.insert("TGMainFrame::GetMainFrame");
	methods_to_skip.insert("TGFrame::GetMainFrame");
	methods_to_skip.insert("TGWindow::GetMainFrame");
	methods_to_skip.insert("TRootBrowser::GetMainFrame");
	methods_to_skip.insert("TRootBrowserLite::GetMainFrame");


	return methods_to_skip;
}

///
/// Init the type translation system
///
void WrapperConfigurationInfo::InitTypeTranslators()
{
	///
	/// Initialize the ROOT typedef mappers...
	///

	InitTypeDefs();

	///
	/// Now, the simple type mappers for things that are so basic we can't translate them.
	///

	char *simple_types[] = {"int", "long", "double", "float", "bool", "short", "long long"};
	bool canbe_unsigned[] = {true, true, false, false, false, true, true};
	int n_simple_types = sizeof(simple_types)/sizeof(char*);

	for (int i = 0; i < n_simple_types; i++) {
	  string simple_type (simple_types[i]);

	  CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType (simple_type, simple_type));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("const " + simple_type, "const " + simple_type));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType (simple_type));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("const " + simple_type, true));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference (simple_type));
	  CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("const " + simple_type, true));

	  CPPNetTypeMapper::instance()->AddTypeMapper (new TVectorArray(simple_type));

	  if (canbe_unsigned[i]) {
		CPPNetTypeMapper::instance()->AddTypeMapper (new TVectorArray("unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("unsigned " + simple_type, "unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("const unsigned " + simple_type, "const unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("const unsigned " + simple_type, true));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("unsigned " + simple_type));
	    CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("const unsigned " + simple_type, true));
	  }
	}

	///
	/// void* - which we deal with as just a TObject of sorts...
	///

	CPPNetTypeMapper::instance()->AddTypeMapper(new TVoidPointer());

	///
	/// Char we have to handle specially since it can become a string -- and we don't want to be passing
	/// arrays back and forth. ;-) [Good way to piss people off!]
	///

	CPPNetTypeMapper::instance()->AddTypeMapper (new TTCPPString());
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTCPPString("char*"));

	CPPNetTypeMapper::instance()->AddTypeMapper (new TArrayOfChar ("char**", false));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("unsigned char", "unsigned char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("char", "char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleType ("const char", "const char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("unsigned char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TPointerSimpleType ("const unsigned char", true));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("char"));
	CPPNetTypeMapper::instance()->AddTypeMapper (new TTSimpleReference ("unsigned char", false));
}

void DefineTypeDef (const string &typedef_name, const string &base_name)
{
  CPPNetTypeMapper::instance()->AddTypedefMapping (typedef_name, base_name);
  CPPNetTypeMapper::instance()->AddTypedefMapping (typedef_name + "*", base_name + "*");
  CPPNetTypeMapper::instance()->AddTypedefMapping (typedef_name + "&", base_name + "&");

  CPPNetTypeMapper::instance()->AddTypedefMapping ("const " + typedef_name, "const " + base_name);
  CPPNetTypeMapper::instance()->AddTypedefMapping ("const " + typedef_name + "*", "const " + base_name + "*");
  CPPNetTypeMapper::instance()->AddTypedefMapping ("const " + typedef_name + "&", "const " + base_name + "&");
}

///
/// Look through ROOT's typedefs and put translations into our library
///
void InitTypeDefs (void)
{
	TIter i_typedef (gROOT->GetListOfTypes());
	TDataType *typedef_spec;
	while ((typedef_spec = static_cast<TDataType*>(i_typedef.Next())) != 0)
	{
		string typedef_name = typedef_spec->GetName();
		string base_name = typedef_spec->GetFullTypeName();

		///
		/// typedefs for function definitions we don't allow. This is a bit of
		/// a kludge work-around, but not sure exactly how to check...
		///

		//G__TypedefInfo g (typedef_name.c_str());
		//string t1 = g.Name();
		//string t2 = g.TrueName();
		//auto t5 = g.Value();
		if (string(typedef_spec->GetTitle()).find("(*") != string::npos) {
			continue;
		}

		/// Special cases. :(
		if (typedef_name.find("Func_t") != string::npos
			|| typedef_name.find("Fun_t") != string::npos) {
			continue;
		}

		///
		/// Check to see if it is a pointer or is a const and modify accordingly.
		/// TODO: This code doesn't work, but that, according to Axel, is because it is broken. Wait for REFLEX I guess.
		///

		if (typedef_name == "Option_t") {
			base_name = "const char";
		}

#ifdef notyet
		cout << typedef_name << " -> " << base_name << " (flags: " << std::hex << typedef_spec->Property() << ")" << endl;

		if (typedef_spec->Property() & G__BIT_ISCONSTANT) {
			base_name = "const " + base_name;
		}
		if (typedef_spec->Property() & kIsPointer) {
			base_name += "*";
		}
#endif

		///
		/// Now, add several varriations of the typedef to the translator - including the const because
		/// our typdef translator isn't very smart!!! :-)
		///

		DefineTypeDef (typedef_name, base_name);
	}

	///
	/// There are some that seem to be left out as well...
	///

	DefineTypeDef ("Text_t", "char");
}

/// Do we know about allowed libraries for this guy?
bool WrapperConfigurationInfo::CheckAllowedLibraries(const std::string &library_name)
{
	init_allowed_library_links();
	return _allowed_library_links.find(library_name) != _allowed_library_links.end();
}
/// Do we know about disallowed libraries?
bool WrapperConfigurationInfo::CheckDisallowedLibraries(const std::string &library_name)
{
	init_allowed_library_links();
	return _disallowed_library_links.find(library_name) != _disallowed_library_links.end();
}

/// Certian (and not) libraries are built to have very restricted linkages. This is mostly to cut
/// circular dependences.
bool WrapperConfigurationInfo::IsLibraryLinkRestricted(const std::string &library_name)
{
	return CheckAllowedLibraries(library_name) || CheckDisallowedLibraries(library_name);
}

///
/// Return the list of libraries that this guy is allowed to link to!
///
vector<string> WrapperConfigurationInfo::AllowedLibraryLinks(const std::string &library_name)
{
	init_allowed_library_links();

	map<string, vector<string> >::const_iterator items = _allowed_library_links.find(library_name);
	if (items == _allowed_library_links.end()) {
		return vector<string> (); // SHould never happen! Call IsResitrctedFirst!!!
	}
	return items->second;
}

///
/// Return the list of libraries that this guy is not allowed to link to!
///
vector<string> WrapperConfigurationInfo::DisallowedLibraryLinks(const std::string &library_name)
{
	init_allowed_library_links();

	map<string, vector<string> >::const_iterator items = _disallowed_library_links.find(library_name);
	if (items == _disallowed_library_links.end()) {
		return vector<string> (); // SHould never happen! Call IsResitrctedFirst!!!
	}
	return items->second;
}

///
/// Setup the list of allowed links.
///
void WrapperConfigurationInfo::init_allowed_library_links()
{
	if (_allowed_library_links_ok) {
		return;
	}
	_allowed_library_links_ok = true;

	/// libCore can't link to anything. We have to start somewhere! :-)
	_allowed_library_links["libCore"] = vector<string> ();

	/// libTreePlayer links to libTree. Don't allow the reverse!
	_disallowed_library_links["libTree"].push_back("libTreePlayer");

	/// libGpad can't depend on the open GL stuff
	_disallowed_library_links["libGpad"].push_back("libRGL");

	/// libNet can't depend on libProof!
	_disallowed_library_links["libNet"].push_back("libProof");
	_disallowed_library_links["libNet"].push_back("libTree");

	/// Proof can't depend on the proof player!
	_disallowed_library_links["libProof"].push_back("libProofPlayer");
}

///
/// Remove libraries we know we can't translate from the list!
///
vector<string> WrapperConfigurationInfo::RemoveBadLibraries(const std::vector<std::string> &library_list)
{
	vector<string> bad_libs;
	bad_libs.push_back("gdk-1.3");
	bad_libs.push_back("iconv-1.3");
	bad_libs.push_back("libEGPythia6");
	bad_libs.push_back("libFFTW");
	bad_libs.push_back("libHbook");
	bad_libs.push_back("libOracle");
	bad_libs.push_back("libPyROOT");
	bad_libs.push_back("libQtGSI");
	bad_libs.push_back("libRMySQL");
	bad_libs.push_back("libNew");
	bad_libs.push_back("libCintex");
	bad_libs.push_back("libReflex");
	vector<string> libraries(library_list);
	for (unsigned int i = 0; i < bad_libs.size(); i++) {
		vector<string>::iterator itr;
		while ((itr = find(libraries.begin(), libraries.end(), bad_libs[i])) != libraries.end()) {
			libraries.erase(itr);
		}
	}

	return libraries;
}

///
/// Return the list of classes that, if built in the particular given library, will cause bad things to be linked in!
///
vector<string> WrapperConfigurationInfo::BadClassLibraryCrossReference(const std::string &library_name, const std::vector<std::string> &class_list)
{
	///
	/// If there is nothign we know, then there are no classes to remove!
	///

	if (!IsLibraryLinkRestricted(library_name)) {
		return vector<string>();
	}

	///
	/// Ok -- do it carefully!
	///

	vector<string> allowed_libraries (AllowedLibraryLinks(library_name));
	vector<string> disallowed_libraries (DisallowedLibraryLinks(library_name));
	vector<string> result;
	for (unsigned int i = 0; i < class_list.size(); i++) {
	  if (!ROOTHelpers::IsClass(class_list[i])) {
		continue;
	  }
		RootClassInfo &class_info (RootClassInfoCollection::GetRootClassInfo(class_list[i]));
		if (class_info.LibraryName() != library_name) {
			bool bad_library = false;
			if (CheckAllowedLibraries(library_name)) {
				if (find(allowed_libraries.begin(), allowed_libraries.end(), class_info.LibraryName()) == allowed_libraries.end()) {
					bad_library = true;
				}
			}
			if (CheckDisallowedLibraries(library_name)) {
				if (find(disallowed_libraries.begin(), disallowed_libraries.end(), class_info.LibraryName()) != disallowed_libraries.end()) {
					bad_library = true;
				}
			}
			if (bad_library) {
				result.push_back(class_list[i]);
			}
		}
	}
	return result;
}

///
/// Find and get all the .libs we should be loading!
///
vector<string> WrapperConfigurationInfo::GetAllRootDLLS()
{
	///
	/// The directory we will scan. Somehow this should be passed in or looked up with ROOTSYS. Later.
	// TODO: load directory to scan from something real like ROOTSYS or command line.
	/// We search for all .lib flies... And assume they can be turned into dll files...
	///

	const int root_sys_buffer_size = 1024;
	char root_sys_buffer[root_sys_buffer_size];
	::GetEnvironmentVariableA ("ROOTSYS", root_sys_buffer, root_sys_buffer_size);
	string dir_to_scan (root_sys_buffer);
	dir_to_scan += "\\bin";

	///
	/// Use the standard WIN32 find file code
	///

	WIN32_FIND_DATAA data_finder;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	
	string scan_wildcard = dir_to_scan + "\\*.dll";
	hFind = FindFirstFileA (scan_wildcard.c_str(), &data_finder);
	if (hFind == INVALID_HANDLE_VALUE) {
		cout << "Unable to search for librareis using " << scan_wildcard << "!" << endl;
		return vector<string> ();
	}

	vector<string> result;
	do {
		string fname = data_finder.cFileName;
		int end_of_lib = fname.find(".dll");
		result.push_back(fname.substr(0,end_of_lib));
	} while (FindNextFileA(hFind, &data_finder) != 0);

	FindClose(hFind);

	return result;
}

///
/// Some methods have different definitions in CINT and ROOT. We have to kludge our way around this
/// as there is no way for us to really know what is going on here.
///
void WrapperConfigurationInfo::FixUpMethodArguments (const RootClassInfo *class_info, const string &method_name, vector<RootClassMethodArg> &args)
{
	///
	/// TTree::Process and anything below it use Process(void *selector...) when they really mean "Process(TSelector *...).
	///

	auto inher = class_info->GetInheritedClassesDeep();
	if (method_name == "Process"
		&& (class_info->CPPName() == "TTree"
		|| find(inher.begin(), inher.end(), "TTree") != inher.end())) {
			if (args[0].CPPTypeName() == "void*") {
				args[0].ResetType("TSelector*", "TSelector");
			}
	}
}
