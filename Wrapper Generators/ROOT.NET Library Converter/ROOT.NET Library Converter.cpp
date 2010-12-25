///
/// ROOT.NET Library Converter
///
///  Attempts to wrap the ROOT libraries on a library-by-library basis. Leaves be hehind a full blown solution with
/// a subproject for every library. That can be built from the command line or opened in the IDE and built there.
///

#include "LibraryConverterDriver.h"
#include "WrapperConfigurationInfo.hpp"

#include <string>
#include <vector>
#include <algorithm>

using std::string;
using std::vector;
using std::for_each;

int main(int argc, char* argv[])
{
	///
	/// Parameters to be filled by the command line arguments
	///

	string ouput_base_directory;	// Where the solution and sub-projects will be written.
	vector<string> libraries;		// List of libraries we will convert.

	///
	/// Command line parsing
	///

	ouput_base_directory = "..\\..\\Wrappers\\ROOTDotNetByLib";

	for (int i = 1; i < argc; i++) {
	  string arg = argv[i];
	  if (arg == "-d") { // Destination directory
	    i++;
	    ouput_base_directory = argv[i];
	  }
	}

	///
	/// Tell the translation system what libraries to look at!
	///

	LibraryConverterDriver driver;

	vector<string> base_root_libs (WrapperConfigurationInfo::GetAllRootDLLS());
	for_each(base_root_libs.begin(), base_root_libs.end(), [&driver] (string &s) { driver.translate_classes_in_library(s); });

	///
	/// Set the output info
	///

	driver.set_output_solution_directory(ouput_base_directory);
	driver.write_solution(true);

	///
	/// Do the work!
	///

	driver.translate();

	return 0;
}

#ifdef notyet

#include "WrapperConfigurationInfo.hpp"
#include "ROOTHelpers.h"
#include "ClassInterfaceRepositoryState.hpp"
#include "ClassTranslator.hpp"
#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"
#include "ConverterErrorLog.hpp"
#include "SourceEmitter.hpp"

#include "TApplication.h"
#include "TSystem.h"

#include "shlobj.h"

#include <iostream>
#include <string>
#include <vector>
#include <tchar.h>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <fstream>
#include <iterator>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::find;
using std::runtime_error;
using std::map;
using std::for_each;
using std::pair;
using std::getline;
using std::ifstream;
using std::ofstream;
using std::make_pair;
using std::copy;
using std::back_inserter;
using std::transform;
using std::inserter;

/// Special version of the translator that only writes out the .hpp files when needed
class LibraryClassTranslator: public ClassTranslator
{
public:
	inline LibraryClassTranslator (const string &dirname)
		: ClassTranslator(dirname)
	{}

protected:
	/// Emit the .hpp file only if the two libraries are the same
	virtual bool emit_this_header (const RootClassInfo &class_being_wrapped, const RootClassInfo &dependent_class)
	{
		return class_being_wrapped.LibraryName() == dependent_class.LibraryName();
	}
	virtual bool emit_this_enum (const RootClassInfo &class_being_wrapped, const RootEnum &dependent_enum)
	{
		return class_being_wrapped.LibraryName() == dependent_enum.LibraryName();
	}

	/// Dump out the headers if we are orking on the core library (only)
	void emit_hpp_headers (SourceEmitter &emitter)
	{
	  ClassTranslator::emit_hpp_headers(emitter);
	  if (OutputDir().find("libCore") != OutputDir().npos) {
	    emitter.include_file("root_type_helpers.hpp");
	  }
	}
};

/// Make sure a directory exists, if not, create it.
void check_dir (const string &dir_name);

/// Create a solution file
void make_solution_file (const string &output_dir, const vector<pair<string, string> > &project_guids);

/// Helper class to generate project files.
class create_project_files
{
public:
	inline create_project_files (const string &proj_dir, const ClassTranslator &trans, const vector<string> &global_link_dep, const vector<string> &extra_files)
		: _base_dir (proj_dir), _translator(trans), _global_link_dependencies(global_link_dep), _extra_files (extra_files)
	{}
	void operator() (const pair<string, vector<string> > &library_classes);
	vector<pair<string, string> > ProjectGuids (void) const {return _project_guid;}
private:
	const string _base_dir;
	const vector<string> &_extra_files;
	vector<pair<string, string> > _project_guid;
	const ClassTranslator &_translator;
	const vector<string> &_global_link_dependencies;
};

/// Projects depend on one and the other -- we will "fill them in" -- this has to be done after we are all finished, unfortunately.
class fill_in_project_references
{
public:
	fill_in_project_references (const string &proj_dir, const ClassTranslator &trans, vector<pair<string, string> > project_guids);
	void operator() (const pair<string, vector<string> > &library_classes);
	vector<pair<string, string> > ProjectGuids (void) const {return _project_guid;}
private:
	const string _base_dir;
	vector<pair<string, string> > _project_guid;
	const ClassTranslator &_translator;
	map<string, string> _project_guids;
};

/// Look at a class and see if it uses a library that it shouldn't be in its inherritance path.
class find_bad_inheritance_classes
{
public:
	void operator() (const string &class_name);
	vector<string> BadClasses (void) const {return _bad_classes;}
private:
	vector<string> _bad_classes;
};

/// Write out a line to declare this assembly as a friend.
class make_assembly_as_friend
{
public:
	inline make_assembly_as_friend (ostream &output)
		: _output (output)
	{}
	inline void operator() (const pair<string, vector<string> > &item)
	{
		_output << "[assembly:InternalsVisibleTo(\"" << item.first << "Wrapper\")];" << endl;
	}
private:
	ostream &_output;
};

int main(int argc, char* argv[])
{
	///
	/// Parameters to be filled by the command line arguments
	///

	string ouput_base_directory;	// Where the solution and sub-projects will be written.
	vector<string> libraries;		// List of libraries we will convert.

	///
	/// Command line parsing
	///

	ouput_base_directory = "..\\..\\Wrappers\\ROOTDotNetByLib";
	vector<string> base_root_libs (WrapperConfigurationInfo::GetAllRootDLLS());
	libraries.insert(libraries.end(), base_root_libs.begin(), base_root_libs.end());

	for (int i = 1; i < argc; i++) {
	  string arg = argv[i];
	  if (arg == "-d") { // Destination directory
	    i++;
	    ouput_base_directory = argv[i];
	  }
	}

	libraries = WrapperConfigurationInfo::RemoveBadLibraries(libraries);

	cout << "Output project will be in '" << ouput_base_directory << "'." << endl;

	///
	/// Experimental
	///

	bool add_to_base_libraries = true;
	vector<string> files_to_load;
	files_to_load.push_back("MuonInBJet.cpp");
	files_to_load.push_back("BTagJet.cpp");

	vector<string> specific_classes_to_translate; // Contains classes we should translate
	specific_classes_to_translate.push_back("BTagJet");
	specific_classes_to_translate.push_back("MuonInBJet");

	string output_library_name = "additional"; // Leave empty string for defualt behavior.

	///
	/// Start up ROOT
	///

	int nargs = 2;
	char *myargv[2];
	myargv[0] = "ROOT.NET Library Converter.exe";
	myargv[1] = "-b";
	TApplication *app = new TApplication ("ROOT.NET Library Converter", &nargs, myargv);

	///
	/// Get the type system up and running
	///

	WrapperConfigurationInfo::InitTypeTranslators();
	RootClassInfoCollection::SetBadMethods(WrapperConfigurationInfo::GetListOfBadMethods());

	///
	/// Load in any extra files we are supposed to load in
	///

	for (int i = 0; i < files_to_load.size(); i++) {
		int result = gSystem->CompileMacro(files_to_load[i].c_str(), "k");
		if (result != 1) {
			cout << "Unable to compile the file " << files_to_load[i] << "! Quitting!" << endl;
			return 0;
		}
	}

	///
	/// Get all the classes we are going to do now...
	///

	vector<string> all_classes = ROOTHelpers::GetAllClassesInLibraries (libraries);

	///
	/// Get all the enums that we are going to do as well
	///

	vector<string> all_enums = ROOTHelpers::GetAllEnums();

	///
	/// Next, remove any classes that are in the bad list.
	///

	all_classes = WrapperConfigurationInfo::RemoveBrokenClasses(all_classes);

	///
	/// Remove classes that violate the library linkages.
	///

	{
		find_bad_inheritance_classes get_them;
		for_each (all_classes.begin(), all_classes.end(), get_them);
		vector<string> bad_inherit_classes (get_them.BadClasses());
		for (unsigned int i = 0; i < bad_inherit_classes.size(); i++) {
			vector<string>::iterator itr;
			ConverterErrorLog::log_type_error (bad_inherit_classes[i], "Can't wrap this class because one of its inherited classes will cause a circular library dependency.");
			while ((itr = find(all_classes.begin(), all_classes.end(), bad_inherit_classes[i])) != all_classes.end())
			{
				all_classes.erase(itr);
			}
		}
	}

	///
	/// Convert. Inform framework of the list of classes & enums that should be translated. If we are doing
	/// just an "add-on" then make sure to just queue up the ones we want.
	///

	ClassInterfaceRepositoryState rep_state;

	if (add_to_base_libraries) {
		for (unsigned int i = 0; i < all_classes.size(); i++) {
			rep_state.register_class_translation (all_classes[i]);
		}
		for (unsigned int i = 0; i < all_enums.size(); i++) {
			rep_state.register_enum_translation (all_enums[i]);
		}

		for (unsigned int i = 0; i < specific_classes_to_translate.size(); i++) {
			rep_state.request_class_translation(specific_classes_to_translate[i]);
		}
	} else {
		for (unsigned int i = 0; i < all_classes.size(); i++) {
			rep_state.request_class_translation (all_classes[i]);
		}
		for (unsigned int i = 0; i < all_enums.size(); i++) {
			rep_state.request_enum_translation (all_enums[i]);
		}
	}

	///
	/// Trnslate the enums. Put them in the CORE library because we can't tell where they go!
	///

	LibraryClassTranslator translator (ouput_base_directory);
	map<string, vector<string> > files_by_library;

	while (rep_state.enums_to_translate()) {
		string enum_name = rep_state.next_enum();
		cout << "Translating enum " << enum_name << endl;
		RootEnum info (enum_name);
		string libName = info.LibraryName();
		if (output_library_name.size() > 0) {
			libName = output_library_name;
		}
		string output_dir = ouput_base_directory + "\\" + libName + "\\Source";
		files_by_library[libName].push_back(info.Name());
		check_dir (output_dir);
		translator.SetOutputDir (output_dir);
		translator.translate (info);
	}

	///
	/// Now translate the classes
	///

	while (rep_state.classes_to_translate()) {
		string class_name = rep_state.next_class();
		cout << "Translating " << class_name << endl;
		RootClassInfo &info (RootClassInfoCollection::GetRootClassInfo(class_name));
		string libName = info.LibraryName();
		if (output_library_name.size() > 0) {
			libName = output_library_name;
		}
		string output_dir = ouput_base_directory + "\\" + libName + "\\Source";
		files_by_library[libName].push_back("N" + class_name);
		check_dir (output_dir);
		translator.SetOutputDir (output_dir);
		translator.translate (info);
	}
	translator.finalize_make_publics();

	///
	/// Write out any .hpp files that are going to get required...
	///

	{
	  string output_dir = ouput_base_directory + "\\libCore\\Source";
	  SourceEmitter helper_output_hpp (output_dir + "/root_type_helpers.hpp");
	  SourceEmitter helper_output_cpp (output_dir + "/root_type_helpers.cpp");

	  helper_output_cpp.include_file("root_type_helpers.hpp");

	  helper_output_hpp.start_line() << "#ifndef __root_type_helpers__" << endl;
	  helper_output_hpp.start_line() << "#define __root_type_helpers__" << endl;
	  CPPNetTypeMapper::instance()->write_out_clr_type_support_files(helper_output_hpp);
	  helper_output_hpp.start_line() << "#endif" << endl;

	  helper_output_cpp.close();
	  helper_output_hpp.close();
	}
	///
	/// We want all internal symbols visibile to all the projects we are building right now. Sort
	/// of like a nice sandbox. :-) Create the "visibility" file. The source file that will give
	/// permission to every other assembly to look into this one.
	///

	string assembly_export_file = ouput_base_directory + "\\make_visible_to_all.cpp";
	{
		ofstream output (assembly_export_file.c_str());
		output << "/// Generated by wrapper utilities -- make all assemblies visible to each other" << endl;
		output << "using namespace System::Runtime::CompilerServices;" << endl;
		for_each (files_by_library.begin(), files_by_library.end(), make_assembly_as_friend(output));
		output.close();
	}

	///
	/// Next, build a solution for VS for each project we are looking at.
	/// Root has some funny dependencies in it. For example, Hist seems to depend on Matrix. So, we'll just force
	/// the issue to remain safe.
	///

	libraries.push_back("libMatrix");
	vector<string> extra_files;
	create_project_files proj_maker(ouput_base_directory, translator, libraries, extra_files);
	create_project_files result(for_each (files_by_library.begin(), files_by_library.end(), proj_maker));
	
	for_each(files_by_library.begin(), files_by_library.end(), fill_in_project_references(ouput_base_directory, translator, result.ProjectGuids()));

	make_solution_file (ouput_base_directory, result.ProjectGuids());

	///
	/// Finally, dump the error summary.
	///

	ofstream errors_out ("conversion_errors.txt");
	ConverterErrorLog::dump(errors_out);
	errors_out << endl << endl << "In order of least frequent to most frequent: " << endl;
	ConverterErrorLog::dump_by_error_order(errors_out);
	errors_out.close();
	ConverterErrorLog::dump(cout);
	cout << endl << endl << "In order of least frequent to most frequent: " << endl;
	ConverterErrorLog::dump_by_error_order(cout);

	return 0;
}

/// Check to see that a directory exists. If not, create it.
void check_dir (const string &dir_name)
{
	const int dir_size = 400;
	char b_current_dir[dir_size];
	int err = GetCurrentDirectoryA(dir_size, b_current_dir);
	if (err == 0) {
		throw runtime_error ("Failed to get current working directory!");
	}

	string full_dir = string(b_current_dir) + "\\" + dir_name;

	err = SHCreateDirectoryExA (NULL, full_dir.c_str(), NULL);
	if (err != ERROR_SUCCESS && err != ERROR_ALREADY_EXISTS && err != ERROR_FILE_EXISTS) {
		throw runtime_error ("Error trying to create directory " + dir_name);
	}
}

class add_class_file_reference
{
public:
	inline add_class_file_reference (const std::string &filetype, const string &command, ostream &output)
		: _filetype (filetype), _output(output), _command(command)
	{}

	void operator() (const string &class_name);
private:
	const string _filetype, _command;
	ostream &_output;
};

/// Add a reference to a fully formed file in the output.
class add_file_reference
{
public:
	inline add_file_reference (ostream &output)
		: _output(output)
	{}

	void operator() (const string &class_name);
private:
	ostream &_output;
};

class emit_include_for_project {
public:
		inline emit_include_for_project (ostream &output)
			: _output (output), _first_dependent (true)
		{}
		void operator() (const string &project_name);
private:
	ostream &_output;
	bool _first_dependent;
};
class emit_link_dependency {
public:
		inline emit_link_dependency (ostream &output)
			: _output (output)
		{}
		void operator() (const string &project_name);
private:
	ostream &_output;
};

/// Create a project file
void create_project_files::operator ()(const pair<string,vector<string> > &library_classes)
{
	const string library_name = library_classes.first;
	const vector<string> &classes = library_classes.second;

	///
	/// Open the input template and the output solution file
	///

	string project_path = _base_dir + "\\" + library_name + "\\" + library_name + ".vcxproj-temp";
	ofstream output (project_path.c_str());
	ifstream input ("cpp_template_project.vcxproj");

	///
	/// Now, read one to the other!
	///

	while (!input.fail()) {
		string line;
		getline (input, line);

		if (line.find("<!-- CPP Files -->") != line.npos) {
			for_each (classes.begin(), classes.end(), add_class_file_reference ("cpp", "ClCompile", output));
			for_each (_extra_files.begin(), _extra_files.end(), add_file_reference (output));
		} else if (line.find("<!-- HPP Files -->") != line.npos) {
			for_each (classes.begin(), classes.end(), add_class_file_reference ("hpp", "ClInclude", output));
		} else if (line.find("<!-- ProjectGUID -->") != line.npos) {
			GUID guid;
			CoCreateGuid (&guid);
			RPC_CSTR g_string;
			UuidToStringA (&guid, &g_string);
			output << "    <ProjectGuid>{" << g_string << "}</ProjectGuid>" << endl;
			_project_guid.push_back(make_pair(library_name, string((const char*)g_string)));
			RpcStringFreeA(&g_string);
		} else if (line.find("<!-- Name -->") != line.npos) {
			output << "Name=\"" << library_name << "Wrapper\"" << endl;
		} else if (line.find("<!-- RootNameSpace -->") != line.npos) {
			output << "<RootNamespace>" << library_name << "</RootNamespace>" << endl;
		} else if (line.find("<!-- ADDIncludes -->") != line.npos) {
			vector<string> dependent_libraries = _translator.get_dependent_libraries(library_name);
		} else if (line.find("<!-- ADDLinkLibraries -->") != line.npos) {
			vector<string> dependent_libraries = _translator.get_dependent_libraries(library_name);
			copy (_global_link_dependencies.begin(), _global_link_dependencies.end(), back_inserter(dependent_libraries));
			for_each (dependent_libraries.begin(), dependent_libraries.end(), emit_link_dependency(output));
		} else {
			output << line << endl;
		}
	}

	input.close();
	output.close();
}

/// Create the map between projects and GUIDs for later reference.
fill_in_project_references::fill_in_project_references (const string &proj_dir, const ClassTranslator &trans, vector<pair<string, string> > project_guids)
: _base_dir (proj_dir), _translator(trans)
{
	for (unsigned int i = 0; i < project_guids.size(); i++) {
		_project_guids[project_guids[i].first] = project_guids[i].second;
	}
}

class write_project_reference {
public:
	inline write_project_reference (ostream &output, const map<string, string> &project_guids)
		: _output (output), _project_guids(project_guids)
	{}
	void operator() (const string &library_name);
private:
	const map<string, string> &_project_guids;
	ostream &_output;
};
class write_using_statement {
public:
	inline write_using_statement (ostream &output)
		: _output (output)
	{}
	void operator() (const string &library_name);
private:
	ostream &_output;
};

/// Fill in the references sections of the files.
/// The refernecs is the .vcxproj get the build order right. But we need to use the #using in order
/// to declare the other guys as friends. So we have to write a seperate project includer file.
void fill_in_project_references::operator ()(const pair<string,vector<string> > &library_classes)
{
	const string library_name = library_classes.first;

	///
	/// Open the previously created vcxproj file and the final file.
	///

	string temp_project_path = _base_dir + "\\" + library_name + "\\" + library_name + ".vcxproj-temp";
	ifstream input (temp_project_path.c_str());

	string project_path = _base_dir + "\\" + library_name + "\\" + library_name + "Wrapper.vcxproj";
	ofstream output (project_path.c_str());

	string assembly_loader_header = _base_dir + "\\" + library_name + "\\assembly_loader.hpp";
	ofstream ass_loader (assembly_loader_header.c_str());

	///
	/// Now, read one to the other!
	///

	while (!input.fail()) {
		string line;
		getline (input, line);

		if (line.find("<!-- ReferenceProjects -->") != line.npos) {
			vector<string> dependent_libraries = _translator.get_dependent_libraries(library_name);
			for_each (dependent_libraries.begin(), dependent_libraries.end(), write_project_reference(output, _project_guids));
			//for_each (dependent_libraries.begin(), dependent_libraries.end(), write_using_statement(ass_loader));
		} else {
			output << line << endl;
		}
	}

	input.close();
	output.close();
	ass_loader.close();
}

void write_using_statement::operator () (const std::string &lib_name)
{
	_output << "#using \"" << lib_name << "Wrapper.dll\" as_friend" << endl;
}

void write_project_reference::operator ()(const string &library_name)
{
	map<string, string>::const_iterator itr (_project_guids.find(library_name));
	if (itr == _project_guids.end()) {
		return;
	}

	_output << "    <ProjectReference";
	_output << " Include=\"..\\" << library_name << "\\" << library_name << "Wrapper.vcxproj" << "\">" << endl;
	_output << "      <Project>{" << itr->second << "}</Project>" << endl;
	_output << "    </ProjectReference>" << endl;
}

/// Output a quoted project include file string.
void emit_include_for_project::operator () (const string &project_name)
{
	if (!_first_dependent) {
		_output << ";";
	} else {
		_first_dependent = false;
	}

	_output << "&quot;$(SolutionDir)\\" << project_name << "\\Source&quot;";
}

/// Output a quoted project include file string.
void emit_link_dependency::operator () (const string &project_name)
{
	_output << " " << project_name << ".lib;";
}

void add_file_reference::operator() (const string &file_name)
{
	_output << "<File RelativePath=\"" << file_name << "\"></File>" << endl;
}

/// Helper class to define a few lines of code for the project def in a solution file
class output_project_defines
{
public:
	inline output_project_defines (ostream &output)
		: _output(output)
	{}
	void operator() (const pair<string, string> &item);
private:
	ostream &_output;
};

/// Out put the active configurations for our projects!
class output_project_configs
{
public:
	inline output_project_configs (ostream &output)
		: _output(output)
	{}
	void operator() (const pair<string, string> &item);
private:
	ostream &_output;
};

/// Create a solution file
void make_solution_file (const string &output_dir, const vector<pair<string, string> > &project_guids)
{
	string project_path = output_dir + "\\ROOT.NET Lib.sln";
	SourceEmitter output (project_path);
	ifstream input ("solution_template.sln");

	while (!input.fail()) {
		string line;
		getline (input, line);

		if (line.find("<!-- DefProjects -->") != line.npos) {
			for_each (project_guids.begin(), project_guids.end(),
				output_project_defines(output()));
		} else if (line.find("<!-- ProjectConfig -->") != line.npos) {
			for_each (project_guids.begin(), project_guids.end(),
				output_project_configs(output()));
		} else {
			output() << line << endl;
		}
	}

	output.close();
	input.close();
}

char mytoupper (char c)
{
  return (char) toupper(c);
}

string upcase(const string &str)
{
  string result;
  transform (str.begin(), str.end(), inserter(result, result.end()), &mytoupper);
  return result;
}

/// Print out a project define
void output_project_defines::operator() (const pair<string, string> &item)
{
	const string &project_name = item.first;
	const string guid = upcase(item.second);

	_output << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" << project_name
		<< "\", \".\\" << project_name << "\\" << project_name << "Wrapper.vcxproj\", \"{"
		<< guid << "}\"" << endl;
	_output << "EndProject" << endl;
}



/// Print out a project define
void output_project_configs::operator() (const pair<string, string> &item)
{
	const string &project_name = item.first;
	const string guid = upcase(item.second);

	_output << "\t\t{" << guid << "}.Release|Win32.ActiveCfg = Release|Win32" << endl;
	_output << "\t\t{" << guid << "}.Release|Win32.Build.0 = Release|Win32" << endl;
	//_output << "\t\t{" << guid << "}.Debug|Win32.ActiveCfg = Debug|Win32" << endl;
	//_output << "\t\t{" << guid << "}.Debug|Win32.Build.0 = Debug|Win32" << endl;
}

/// If this class has an inherrited class that isn't in the proper library, bounce it!
void find_bad_inheritance_classes::operator ()(const std::string &class_name)
{
  RootClassInfo &master_class (RootClassInfoCollection::GetRootClassInfo(class_name));
	if (WrapperConfigurationInfo::IsLibraryLinkRestricted(master_class.LibraryName())) {
		vector<string> inher_classes (master_class.GetInheritedClassesDeep());
		vector<string> bad_classes (WrapperConfigurationInfo::BadClassLibraryCrossReference(master_class.LibraryName(), inher_classes));
		if (bad_classes.size() > 0) {
			_bad_classes.push_back(class_name);
		}
	}
}

#endif
