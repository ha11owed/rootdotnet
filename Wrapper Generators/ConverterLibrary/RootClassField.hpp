///
/// Hold onto info for a field
///
#pragma once

#include <string>
#include <vector>

class TDataMember;
class RootClassInfo;

class RootClassField
{
public:
	RootClassField(void);
	RootClassField(TDataMember *f);

	/// The CPP safe name of the field
	std::string CPPName(void) const;

	/// The name of the property
	std::string NETName (void) const;

	/// The .NET type
	std::string NETType(void) const;

	/// The CPP Type
	std::string CPPType(void) const;

	/// Return the class where this is actually defined
	std::string ClassOfFieldDefinition(void) const;

	/// Is left < right?
	bool is_less_than (const RootClassField &right) const;

	/// Reset the paraent class (the class that owns this field).
	void ResetParent (const RootClassInfo *parent);

	/// Return true if we can possibly translate this guy
	bool can_be_translated (void) const;

	/// Return a list of all the classes that this field will need
	/// to reference in its translation.
	std::vector<std::string> get_all_referenced_raw_types (void) const;

	~RootClassField(void);

private:
	TDataMember *_root_field;
};

