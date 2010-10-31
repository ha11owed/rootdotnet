///
/// RootEnum
///
///  Represents the root info for an ENUM.
///
#pragma once

#include <string>
#include <vector>

class RootEnum
{
public:
	inline RootEnum(const std::string &name)
		: _name(name), _self_fill (false), _cint_inited(false)
	{}
	inline RootEnum()
	{}
	inline ~RootEnum(void)
	{}

	/// If you want to fill it yourself.
	void add (const std::string &name, const unsigned int value)
	{
		_values.push_back(std::make_pair(name, value));
		_self_fill = true;
	}

	inline const std::vector<std::pair<std::string, unsigned int> > &values() const
	{
		update_values();
		return _values;
	}

	std::string Name(void) const;

	/// Return the name of the include file in ROOT where we are defined.
	std::string include_filename (void) const;

	/// Library where the enum is usually located.
	std::string LibraryName (void) const;

private:
	std::string _name;
	bool _self_fill;

	mutable std::vector<std::pair<std::string, unsigned int> > _values;
	void update_values() const;

	void init_cint_data (void) const;
	mutable bool _cint_inited;
	mutable std::string _include_filename;
	mutable std::string _library_name;
};
