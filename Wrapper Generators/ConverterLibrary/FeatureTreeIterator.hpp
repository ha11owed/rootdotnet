///
/// Add a feature that will make a TTree iterable, and allow access to its leaves via
/// the dynamic interface.
///
#pragma once
#include "FeatureBase.hpp"

class FeatureTreeIterator :
	public FeatureBase
{
public:
	FeatureTreeIterator(void);
	~FeatureTreeIterator(void);

	bool is_applicable (const RootClassInfo &info);
	std::vector<std::string> get_additional_interfaces (const RootClassInfo &info);

	void emit_header_method_definitions (const RootClassInfo &info, SourceEmitter &emitter);

	void emit_class_methods (const RootClassInfo &info, SourceEmitter &emitter);
};
