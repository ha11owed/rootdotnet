///
/// Generate a ROOT.NET translation that is:
///		1) Based on only a set of libraries already specified
///		2) All in one output .DLL/etc/ file
///		3) Possibly building on a previous translation
///

#include "LibraryConverterDriver.h"

#include <string>
#include <vector>
#include <algorithm>

using std::string;
using std::vector;
using std::for_each;

int main(int argc, char* argv[])
{
	///
	/// Command line arguments processing
	///

	string dir_of_project_folder = "C:\\Users\\gwatts\\Documents\\ATLAS\\Projects\\LINQToROOTTest";
	vector<string> dlls;
	dlls.push_back("C:\\Users\\gwatts\\Documents\\ATLAS\\Projects\\LINQToROOT\\NTupleSource\\BTagJet_cpp.dll");
	dlls.push_back("C:\\Users\\gwatts\\Documents\\ATLAS\\Projects\\LINQToROOT\\NTupleSource\\MuonInBJet_cpp.dll");

	vector<string> prev_trans_directory;
	prev_trans_directory.push_back("c:\\root\\NETNewWrappers");

	string libname ("additional");

	///
	/// If there are any other directories that contain libraries we've previously translated,
	/// pick up all the previously translated things from there
	///

	LibraryConverterDriver driver;
	for_each(prev_trans_directory.begin(), prev_trans_directory.end(), [&driver] (const string &s) { driver.add_converted_info(s);});

	///
	/// Ok, now specify the list of libraries we want translated
	///

	for_each(dlls.begin(), dlls.end(), [&driver] (const string &s) { driver.translate_classes_in_library(s); });

	///
	/// The specifics (output location, etc.)
	///

	driver.set_output_solution_directory(dir_of_project_folder);
	driver.write_solution(false);
	driver.use_class_header_locations(true);
	driver.write_all_in_single_library(libname);

	///
	/// Now, do the translation and write out the final project solutoin
	///

	driver.translate();
}
