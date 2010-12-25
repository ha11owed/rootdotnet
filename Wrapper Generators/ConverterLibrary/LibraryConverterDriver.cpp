///
/// Main driver routine that takes a set of libraries, finds the classes
/// in them, and then does the translation.
///

#include "LibraryConverterDriver.h"

#include "WrapperConfigurationInfo.hpp"
#include "RootClassInfoCollection.hpp"
#include "ROOTHelpers.h"
#include "ConverterErrorLog.hpp"
#include "ClassInterfaceRepositoryState.hpp"
#include "LibraryClassTranslator.hpp"
#include "create_project_files.hpp"

#include <TApplication.h>
#include <TSystem.h>

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <fstream>
#include <iterator>

#include "shlobj.h"

using std::vector;
using std::map;
using std::string;
using std::cout;
using std::endl;
using std::for_each;
using std::runtime_error;
using std::ofstream;
using std::pair;
using std::inserter;

LibraryConverterDriver::LibraryConverterDriver(void)
	: _write_solution(true)
{
}


LibraryConverterDriver::~LibraryConverterDriver(void)
{
}

void LibraryConverterDriver::add_converted_info(const std::string &dir_path)
{
}

///
/// we will scan all classes in this library and translate them!
///
void LibraryConverterDriver::translate_classes_in_library (const std::string &dir_path)
{
	///
	/// We can't have paths - well, we can, but it really confuses everything - so
	/// we do some fancy footwork here to get around the issue.
	///

	string libname (dir_path);
	auto last_seperator = dir_path.find_last_of("\\");
	if (last_seperator != dir_path.npos) {
		string dir = dir_path.substr(0, last_seperator);
		gSystem->AddDynamicPath(dir.c_str());
		libname = dir_path.substr(last_seperator+1);
	}

	_libs_to_translate.push_back(libname);
}

void LibraryConverterDriver::set_output_solution_directory (const std::string &sol_path)
{
	_output_dir = sol_path;
}

void LibraryConverterDriver::write_all_in_single_library (const std::string &library_name)
{
	_single_library_name = library_name;
}

///
/// Should we write out a solution file?
///
void LibraryConverterDriver::write_solution (bool dowrite)
{
	_write_solution = dowrite;
}

namespace {
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

	class write_project_reference {
	public:
		inline write_project_reference (ostream &output, const map<string, string> &project_guids)
			: _output (output), _project_guids(project_guids)
		{}
		void operator() (const string &library_name)
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
	private:
		const map<string, string> &_project_guids;
		ostream &_output;
	};

	/// Projects depend on one and the other -- we will "fill them in" -- this has to be done after we are all finished, unfortunately.
	class fill_in_project_references
	{
	public:
		fill_in_project_references (const string &proj_dir, const ClassTranslator &trans, vector<pair<string, string> > project_guids)
			: _base_dir (proj_dir), _translator(trans)
		{
			for (unsigned int i = 0; i < project_guids.size(); i++) {
				_project_guids[project_guids[i].first] = project_guids[i].second;
			}
		}

		void operator() (const pair<string, vector<string> > &library_classes)
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
		vector<pair<string, string> > ProjectGuids (void) const {return _project_guid;}
	private:
		const string _base_dir;
		vector<pair<string, string> > _project_guid;
		const ClassTranslator &_translator;
		map<string, string> _project_guids;
	};

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

	/// Helper class to define a few lines of code for the project def in a solution file
	class output_project_defines
	{
	public:
		inline output_project_defines (ostream &output)
			: _output(output)
		{}
		void operator() (const pair<string, string> &item)
		{
			const string &project_name = item.first;
			const string guid = upcase(item.second);

			_output << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" << project_name
				<< "\", \".\\" << project_name << "\\" << project_name << "Wrapper.vcxproj\", \"{"
				<< guid << "}\"" << endl;
			_output << "EndProject" << endl;
		}
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
		void operator() (const pair<string, string> &item)
		{
			const string &project_name = item.first;
			const string guid = upcase(item.second);

			_output << "\t\t{" << guid << "}.Release|Win32.ActiveCfg = Release|Win32" << endl;
			_output << "\t\t{" << guid << "}.Release|Win32.Build.0 = Release|Win32" << endl;
			//_output << "\t\t{" << guid << "}.Debug|Win32.ActiveCfg = Debug|Win32" << endl;
			//_output << "\t\t{" << guid << "}.Debug|Win32.Build.0 = Debug|Win32" << endl;
		}
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

}

void LibraryConverterDriver::translate(void)
{
	///
	/// Start up ROOT
	///

	int nargs = 2;
	char *myargv[2];
	myargv[0] = "ROOT.NET Library Converter.exe";
	myargv[1] = "-b";
	TApplication *app = new TApplication ("ROOT.NET Library Converter", &nargs, myargv);

	///
	/// Get the type system up and running. This is mostly things like "int", "long long",
	/// and the key "char*" stuff (the last of which requires real work!).
	///

	WrapperConfigurationInfo::InitTypeTranslators();
	RootClassInfoCollection::SetBadMethods(WrapperConfigurationInfo::GetListOfBadMethods());

	///
	/// Get all the classes we are going to do now...
	///

	_libs_to_translate = WrapperConfigurationInfo::RemoveBadLibraries(_libs_to_translate);
	vector<string> all_classes = ROOTHelpers::GetAllClassesInLibraries (_libs_to_translate);

	///
	/// Get all the enums that we are going to do as well
	///

	vector<string> all_enums = ROOTHelpers::GetAllEnums();

	///
	/// Next, remove any classes that are in the bad list.
	///

	all_classes = WrapperConfigurationInfo::RemoveBrokenClasses(all_classes);

	if (all_classes.size() == 0) {
		cout << "ERROR: No classes were requested for translation!" << endl;
		return;
	}

	///
	/// And the last thing is to look to see if this trandslation has been done
	/// before, and remove things from the class and enum list if that has been
	/// the case.
	///

	cout << "Must still remove the extra enum guys!" << endl;

	///
	/// Remove classes that violate the library linkages. This is not an issue
	/// if we are pushing everything out to a single library. Log them as classes
	/// we can't translate in the end.
	///

	if (_single_library_name.size() == 0) {
		vector<string> bad_inherit_classes;
		for_each (all_classes.begin(), all_classes.end(),
			[&bad_inherit_classes] (const string &class_name) {
				RootClassInfo &master_class (RootClassInfoCollection::GetRootClassInfo(class_name));
				if (WrapperConfigurationInfo::IsLibraryLinkRestricted(master_class.LibraryName())) {
					vector<string> inher_classes (master_class.GetInheritedClassesDeep());
					vector<string> bad_classes (WrapperConfigurationInfo::BadClassLibraryCrossReference(master_class.LibraryName(), inher_classes));
					if (bad_classes.size() > 0) {
						bad_inherit_classes.push_back(class_name);
					}
				}
		});

		for (unsigned int i = 0; i < bad_inherit_classes.size(); i++) {
			vector<string>::iterator itr;
			ConverterErrorLog::log_type_error (bad_inherit_classes[i], "Can't wrap this class because one of its inherited classes will cause a circular library dependency.");
			while ((itr = find(all_classes.begin(), all_classes.end(), bad_inherit_classes[i])) != all_classes.end())
			{
				all_classes.erase(itr);
			}
		}
	}

	cout << "Going to convert " << all_classes.size() << " classes." << endl;

	///
	/// Create the holder that tracks all classes and enums we are translating.
	/// Queue up the ones we need to translate, and also the ones we should assume
	/// we already know about.
	///

	ClassInterfaceRepositoryState rep_state;
	for_each(all_classes.begin(), all_classes.end(),
		[&rep_state] (string &s) { rep_state.request_class_translation(s); });
	for_each(all_enums.begin(), all_enums.end(),
		[&rep_state] (string &s) { rep_state.request_enum_translation(s); });

	///
	/// Now translate the enums. Unfortunately, we can't tell what libraries they
	/// are actually in. The good news is that they are dependency free, so we can
	/// put them where we like. When translating everything we put them in the core
	/// library. If we are doing just one library then... well, it is easy. :-)
	///

	LibraryClassTranslator translator (_output_dir);
	map<string, vector<string> > files_by_library;

	while (rep_state.enums_to_translate()) {
		string enum_name = rep_state.next_enum();
		cout << "Translating enum " << enum_name << endl;
		RootEnum info (enum_name);
		string libName = info.LibraryName();
		if (_single_library_name.size() > 0) {
			libName = _single_library_name;
		}
		string output_dir = _output_dir + "\\" + libName + "\\Source";
		files_by_library[libName].push_back(info.Name());
		check_dir (output_dir);
		translator.SetOutputDir (output_dir);
		translator.translate (info);
	}

	///
	/// Now translate the classes. Same deal applies when we are targeting a
	/// single class!
	///

	while (rep_state.classes_to_translate()) {
		string class_name = rep_state.next_class();
		cout << "Translating " << class_name << endl;
		RootClassInfo &info (RootClassInfoCollection::GetRootClassInfo(class_name));
		string libName = info.LibraryName();
		if (_single_library_name.size() > 0) {
			libName = _single_library_name;
		}
		string output_dir = _output_dir + "\\" + libName + "\\Source";
		files_by_library[libName].push_back("N" + class_name);
		check_dir (output_dir);
		translator.SetOutputDir (output_dir);
		translator.translate (info);
	}

	///
	/// One trick is that we have to make sure that everyone is sharing the same
	/// C++ types - this is b/c we may need to access the C++ pointer types in one
	/// library from the other. The only way to do this is to export the types
	/// and make them exposed.
	///

	translator.finalize_make_publics();


	///
	/// Write out any .hpp files that are going to get required... Again, this
	/// is done only to deal with multiple projects.
	///

	if (files_by_library.size() > 1) {
	  string output_dir = _output_dir + "\\libCore\\Source";
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

	if (files_by_library.size() > 1) {
		string assembly_export_file = _output_dir + "\\make_visible_to_all.cpp";
		ofstream output (assembly_export_file.c_str());
		output << "/// Generated by wrapper utilities -- make all assemblies visible to each other" << endl;
		output << "using namespace System::Runtime::CompilerServices;" << endl;
		for_each (files_by_library.begin(), files_by_library.end(),
			[&output] (const pair<string, vector<string> > &item) { output << "[assembly:InternalsVisibleTo(\"" << item.first << "Wrapper\")];" << endl;});
		output.close();
	}

	///
	/// Next, build a solution for VS for each project we are looking at.
	/// Root has some funny dependencies in it. For example, Hist seems to depend on Matrix. So, we'll just force
	/// the issue to remain safe.
	///

	//_libs_to_translate.push_back("libMatrix");
	vector<string> extra_files;

	create_project_files proj_maker(_output_dir, translator, _libs_to_translate, extra_files);
	create_project_files result(for_each (files_by_library.begin(), files_by_library.end(), proj_maker));
	
	for_each(files_by_library.begin(), files_by_library.end(), fill_in_project_references(_output_dir, translator, result.ProjectGuids()));

	if (_write_solution) {
		make_solution_file (_output_dir, result.ProjectGuids());
	}

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

}
