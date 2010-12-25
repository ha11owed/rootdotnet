#pragma once
///
/// Drive the translation process for a set of libraries.
///

#include <string>
#include <vector>

class LibraryConverterDriver
{
public:
	LibraryConverterDriver(void);
	~LibraryConverterDriver(void);

	//// Configuration

	/// Add a list of previously converted objects that we can safely use in our translation.
	void add_converted_info(const std::string &dir_path);

	/// Load this library of ROOT objects, and translate everythign in it!
	void translate_classes_in_library (const std::string &dir_path);

	/// Where should we put the solutoin and the directories with the projects for
	/// each library?
	void set_output_solution_directory (const std::string &sol_path);

	/// Write all objects into a single library rather than mutlple ones.
	void write_all_in_single_library (const std::string &library_name);

	/// Write a solution file (by default)
	void write_solution (bool dowrite);

	//// Run

	void translate();

private:

	bool _write_solution;
	std::vector<std::string> _libs_to_translate;
	std::string _output_dir;
	std::string _single_library_name;
};

