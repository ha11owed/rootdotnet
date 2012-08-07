///
/// Write out code to be attached to a TTree so that we can go after a
/// a tree's items.
///

#include "FeatureTreeIterator.hpp"
#include "RootClassInfo.hpp"

#include <vector>
#include <string>

using std::vector;
using std::string;

FeatureTreeIterator::FeatureTreeIterator(void)
{
}


FeatureTreeIterator::~FeatureTreeIterator(void)
{
}

///
/// We want to work on any class that is TIter or sub-class of that guy.
///
bool FeatureTreeIterator::is_applicable (const RootClassInfo &info)
{
	if (info.CPPName() == "TIter")
		return true;

	return false;
}

///
/// If this is the TIter interface we are making, then we want to add enumerability to it. Everyone
/// else will inherrit from it, so we don't need to do anything.
///
std::vector<std::string> FeatureTreeIterator::get_additional_interfaces (const RootClassInfo &info)
{
	vector<string> result;
	return result;
}

///
/// Emit the header definitions for the enumerable guys!
///
void FeatureTreeIterator::emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter)
{
}

///
/// Emit the class we are going to use along with the methods that return it
///
void FeatureTreeIterator::emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter)
{
}
