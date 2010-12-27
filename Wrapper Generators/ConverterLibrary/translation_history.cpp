///
/// Implementation of the history gathering code - writes out the history
/// of a job and also reads it back in
///

#include "translation_history.hpp"



#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <stdexcept>

using std::ofstream;
using std::ifstream;
using std::for_each;
using std::string;
using std::endl;
using std::runtime_error;
using std::getline;

translation_history::translation_history(void)
{
}


translation_history::~translation_history(void)
{
}


///
/// Write out the history of this translation job
///
void translation_history::save_history (const std::string &dir,
		const std::vector<std::string> &classes_translated,
		const std::vector<std::string> &enums_translated)
{
	ofstream converted_info ((dir + "\\converted_items.txt").c_str());

	for_each(classes_translated.begin(), classes_translated.end(),
		[&converted_info] (const string &s) { converted_info << "class " << s << endl; });
	for_each(enums_translated.begin(), enums_translated.end(),
		[&converted_info] (const string &s) { converted_info << "enum " << s << endl; });

	converted_info.close();
}

///
/// load_history - loads the history in. Given a directory, load it back.
///
void translation_history::load_history (const std::string &dir)
{
	ifstream input ((dir + "\\converted_items.txt").c_str());
	if (!input.good()) {
		throw runtime_error("Error opening the converted_items.txt file in " + dir + "!");
	}

	while (!input.eof()) {
		string line;
		getline(input, line);

		if (line.find("class ") == 0) {
			auto c = line.substr(6);
			_classes.insert(c);
		} else if (line.find("enum ") == 0) {
			auto e = line.substr(5);
			_enums.insert(e);
		}
	}
}
