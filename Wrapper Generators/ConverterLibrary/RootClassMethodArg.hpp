#pragma once

#include <string>

class TMethodArg;

class RootClassMethodArg
{
public:
	RootClassMethodArg(void);
	RootClassMethodArg (TMethodArg *arg);

	/// Returns the CPP type name
	std::string CPPTypeName (void) const;

	/// Returns the NET type name
	std::string NETTypeName (void) const;

	/// Returns the NET interface name
	std::string NETInterfaceTypeName (void) const;

	/// Returns the default name of the argument
	std::string get_argname (void) const;

	/// Reset arg counter - for when args have no name (!!)
	static void reset_arg_counter (void);

	  /// Return the raw type
	std::string RawCPPTypeName (void) const;

	/// Return true if we can translate this guy
	bool can_be_translated (void) const;
private:
	TMethodArg *_root_arg;
	int _index;

	static int _counter;
};
