#pragma once

#include <string>
#include <vector>
#include <map>

class ROOTHelpers
{
public:
	static std::vector<std::string> GetAllClassesInLibraries (const std::vector<std::string> &library_names);

	static std::string GetClassLibraryName (const std::string &class_name);
	static void ForceClassLibraryname (const std::string &class_name, const std::string &library_name);

	static bool IsClass (const std::string &class_name);
	static bool IsEnum (const std::string &enum_name);

	static std::map<std::string, std::vector<std::string> > GetAllGlobals (void);

	/// Return all enums sorted by type/value.
	static std::vector<std::string> GetAllEnums (void);
	static const std::vector<std::pair<std::string, unsigned int> > &GetEnumValues (const std::string &enum_type);

	/// Returns template arguments
	static std::vector<std::string> GetTemplateArguments (const std::string &template_type);

private:
	static std::map<std::string, std::vector<std::pair<std::string, unsigned int> > > _all_enums;
	static void LoadAllEnums();
};
