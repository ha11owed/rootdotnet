#include "CPPNetTypeMapper.hpp"
#include "ConverterErrorLog.hpp"

#include <stdexcept>
#include <algorithm>

using std::string;
using std::map;
using std::runtime_error;
using std::for_each;
using std::ostream;
using std::pair;

CPPNetTypeMapper *CPPNetTypeMapper::_mapper = 0;

CPPNetTypeMapper::CPPNetTypeMapper(void)
{
}

///
/// Return the singleton. Create if we have to.
///
CPPNetTypeMapper* CPPNetTypeMapper::instance()
{
	if (_mapper == 0) {
		_mapper = new CPPNetTypeMapper();
	}
	return _mapper;
}

///
/// Reset to zero -- delete the thing!
/// Used mostly in tests...
///
void CPPNetTypeMapper::Reset (void)
{
  delete _mapper;
  _mapper = 0;
}

///
/// Returns a .net type name. Exception if it can't find it!
///
string CPPNetTypeMapper::GetNetTypename(const std::string cpp_typename) const
{
  const TypeTranslator *t (get_translator_from_cpp(cpp_typename));
  return t->net_typename();
}

///
/// Returns a .net type name. Exception if it can't find it!
///
string CPPNetTypeMapper::GetNetInterfaceTypename(const std::string cpp_typename) const
{
	return get_translator_from_cpp(cpp_typename)->net_interface_name();
}

///
/// Add a type translator to our list of type translators...
///
void CPPNetTypeMapper::AddTypeMapper(CPPNetTypeMapper::TypeTranslator *trans)
{
	_cpp_translator_map[trans->cpp_typename()] = trans;
}

///
/// Add a typedef mapping.
///
void CPPNetTypeMapper::AddTypedefMapping(const std::string &typedef_name, const std::string &type_name)
{
	if (typedef_name != type_name) {
		_typedef_map[typedef_name] = type_name;
	}
}

///
/// Return the type transltor for a particular type
///
const CPPNetTypeMapper::TypeTranslator *CPPNetTypeMapper::get_translator_from_cpp(const std::string &cpp_typename) const
{
	map<string, TypeTranslator*>::const_iterator itr = _cpp_translator_map.find(resolve_typedefs(cpp_typename));
	if (itr == _cpp_translator_map.end()) {
		ConverterErrorLog::log_type_error (resolve_typedefs(cpp_typename), "No .NET translator available");
		throw runtime_error ("Unable to translate cpp type " + cpp_typename + " (lookup as " + resolve_typedefs(cpp_typename) + ")");
	}
	return itr->second;
}

///
/// Resolve any typedef references
///
string CPPNetTypeMapper::resolve_typedefs (const string &in_name) const
{
  string name (in_name);
  bool is_const = false;
  if (name.find("const ") == 0) {
    is_const = true;
    name = name.substr(6);
  }
  map<string, string>::const_iterator itr;
  string found_type (name);
  int count = 0;
  while ((itr = _typedef_map.find(found_type)) != _typedef_map.end()) {
    found_type = itr->second;
    count++;
    if (count > 100) {
      ConverterErrorLog::log_type_error(name, "Circular/recursive type definition detected.");
      throw runtime_error ("Typedef " + name + " has a circular typedef definition -- giving up!");
    }
  }
  if (is_const) {
    found_type = "const " + found_type;
  }

  return found_type;
}

///
/// has_mapping
///
///  Do we know about a particular mapping?
///
bool CPPNetTypeMapper::has_mapping (const string &class_name) const
{
	return _cpp_translator_map.find(resolve_typedefs(class_name)) != _cpp_translator_map.end();
}

namespace {
  class write_out_types {
  public:
    inline write_out_types (SourceEmitter &output)
      : _output (output)
    {}
    inline void operator() (const pair<string, CPPNetTypeMapper::TypeTranslator*> &item)
    {
      item.second->write_out_clr_types(_output);
    }
  private:
    SourceEmitter &_output;
  };
}

///
/// write out type support files. Sometimes the type translators need to author there
/// own support types. They can't do this inline, so we give them a chance to write
/// everything to a seperate file.
///
void CPPNetTypeMapper::write_out_clr_type_support_files(SourceEmitter &output) const
{
  for_each (_cpp_translator_map.begin(), _cpp_translator_map.end(),
    write_out_types (output));
}

///
/// Are the two given types actually the same?
///
bool CPPNetTypeMapper::AreSameType (const std::string &t1, const std::string &t2) const
{
  return resolve_typedefs(t1) == resolve_typedefs(t2);
}

