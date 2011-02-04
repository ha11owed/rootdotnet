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
