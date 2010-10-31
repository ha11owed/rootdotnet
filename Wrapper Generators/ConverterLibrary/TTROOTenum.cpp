///
/// Map between enum's in C++ and NET
///
#include "TTROOTenum.hpp"
#include "SourceEmitter.hpp"

using std::string;
using std::endl;

TTROOTenum::TTROOTenum(const string &enum_name)
: TypeTranslator ("EButtonState", "EButtonState")
{
}

TTROOTenum::~TTROOTenum(void)
{
}

///
/// Always make sure we refer to the global namespace for the enums here...
///
string TTROOTenum::cpp_code_typename() const
{
	return "::" + cpp_typename();
}

///
/// Given a .NET name, translate the guy to CPP.
///
void TTROOTenum::translate_to_cpp(const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter) const
{
	emitter.start_line() << "::" << cpp_typename() << " " << name_cpp << " = (::" << cpp_typename() << ") " << name_net << ";" << endl;
}

///
/// Given a CPP name, translate back to a NET guy
///
void TTROOTenum::translate_to_net(const std::string &name_net, const std::string &name_cpp, SourceEmitter &emitter) const
{
	emitter.start_line() << net_interface_name() << " " << name_net << " = (" << net_typename() << ") " << name_cpp << ";" << endl;
}
