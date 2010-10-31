#include "TTROOTClass.hpp"
#include "SourceEmitter.hpp"
#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"

#include <string>
#include <sstream>
#include <stdexcept>

using std::string;
using std::ostringstream;
using std::endl;
using std::runtime_error;

string build_lookup_typename (const string &class_name, const string modifiers, bool is_const)
{
	string base_class_name = class_name;
	if (is_const) {
		base_class_name = "const " + base_class_name;
	}
	return base_class_name + modifiers;
}

///
/// We will translate a root class. Yes!
TTROOTClass::TTROOTClass(const string &class_name, bool is_from_tobject, const string modifiers, bool is_const)
: _class_name (class_name), _modifiers(modifiers),
_inherits_from_TObject(is_from_tobject), _is_const(is_const),
TypeTranslator (RootClassInfoCollection::GetRootClassInfo(class_name).NETName() + " ^", build_lookup_typename(class_name, modifiers, is_const))
{
}

TTROOTClass::~TTROOTClass(void)
{
}

///
/// Switch from .NET to CPP.
///
/// This doesn't really make sense if they ask for an object with no modifier, but
/// we will just do it. This means the object will get copied twice -- and won't
/// be very efficient!!! Sorry!
///
/// The key to understanding this code is that all the .NET objects hold is a pointer.
/// So "TObject" means pass a copy of the object!
///
void TTROOTClass::translate_to_cpp (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
	emitter.start_line() << "::" << _class_name;
	if (_modifiers == "*&") { // Not clear what this means to me!!
		emitter() << "*";
	} else {
		emitter() << _modifiers;
	}
	emitter() << " " << cpp_name << " = ";
	if (_modifiers == "") {
		emitter() << "*"; // Deref the pointer to make it a full blown object.
	} else if (_modifiers == "&") {
		emitter() << "*"; // Again, to assign a reference
	} else if (_modifiers == "*" || _modifiers == "*&") {
	} else {
		throw runtime_error ("Unknown set of modifiers - " + _modifiers);
	}
	emitter() << net_name << "->CPP_Instance_" << _class_name << "();" << endl;
}

///
/// cpp_code_typename
///
///  Return a cpp name -- built for code. Mostly that means putting a "::" in front of it!
///
string TTROOTClass::cpp_code_typename (void) const
{
	ostringstream result;
	if (_is_const) {
		result << "const ";
	}
	result << "::" << _class_name << _modifiers;
	return result.str();
}

///
/// cpp_core_typename
///
///  The core class, without any extra stuff in it.
///
string TTROOTClass::cpp_core_typename() const
{
	return _class_name;
}

///
/// Switch from CPP world to the .NET world (i.e. for a return type from
/// some sort of call.
///
/// WARNING: If this object doesn't inherrit from TObject yet, then 
/// this won't correctly identify the same object that comes through twice!!
/// this means that it is possible to have two wrapper objects pointing to the
/// same real object, and when both try to do the delete... BOOM!
///
/// This conversion is a bit tricky because we need to
/// translate from the TObject type world into a good representative object
/// in our work. For example, TFile::Get("hi") might return a TH1F -- but it
/// will look a lot like a TObject from the signature. For this to be useful
/// we need to create a TH1F object, and return the interface to TObject from
/// Get. Then the user can re-cast the object as they expect.
///
void TTROOTClass::translate_to_net (const std::string &net_name, const std::string &cpp_name, SourceEmitter &emitter) const
{
	RootClassInfo info (RootClassInfoCollection::GetRootClassInfo(_class_name));
	emitter.start_line() << "ROOTNET::Interface::" << info.NETName() << " ^"
		<< net_name;

	if (_inherits_from_TObject) {
		emitter() << " = ROOTNET::Utility::ROOTObjectServices::GetBestObject<ROOTNET::Interface::" <<  info.NETName() <<"^>"
			<< "(";

		if (_modifiers == "&") {
			emitter() << "&" << cpp_name << ");" << endl;
		} else if (_modifiers == "") {
			emitter() << "new ::" << _class_name << "(";
			emitter() << cpp_name << "));" << endl;
		} else {
			emitter() << cpp_name << ");" << endl;
		}
	} else {
		emitter() << " = gcnew ROOTNET::" << info.NETName() << " (";
		if (_modifiers == "&") {
		  if (_is_const) {
		    emitter() << "const_cast<::" << _class_name << "*>(";
		  } else {
		    emitter() << "(";
		  }
		  emitter() << "&" << cpp_name << "));" << endl;
		} else if (_modifiers == "") {
		  emitter() << "new ::" << _class_name << "(";
		  emitter() << cpp_name << "));" << endl;
		} else {
		  if (_is_const) {
		    emitter() << "const_cast<::" << _class_name << _modifiers << ">(" << cpp_name << "));" << endl;
		  } else {
		    emitter() << cpp_name << ");" << endl;
		  }
		}		
	}
}
