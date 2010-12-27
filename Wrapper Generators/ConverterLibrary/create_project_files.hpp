#pragma once

///
/// Helper class to do the work of generating the project
/// files.
///

#include <string>
#include <vector>
#include <map>

class ClassTranslator;

class create_project_files
{
public:
	inline create_project_files (const std::string &proj_dir, const ClassTranslator &trans,
		const std::vector<std::string> &global_link_dep, 
		const std::vector<std::string> &extra_files,
		const std::vector<std::string> &extra_include_dirs,
		const std::vector<std::string> &link_libs)
		: _base_dir (proj_dir), _translator(trans), _global_link_dependencies(global_link_dep), _extra_files (extra_files),
		_extra_include_dirs(extra_include_dirs), _link_libs(link_libs)
	{}
	void operator() (const std::pair<std::string, std::vector<std::string> > &library_classes);
	std::vector<std::pair<std::string, std::string> > ProjectGuids (void) const {return _project_guid;}
private:
	const std::string _base_dir;
	const std::vector<std::string> &_extra_files;
	std::vector<std::pair<std::string, std::string> > _project_guid;
	const ClassTranslator &_translator;
	const std::vector<std::string> &_global_link_dependencies;
	const std::vector<std::string> &_extra_include_dirs;
	const std::vector<std::string> &_link_libs;
};

