#include "DynamicHelpers.h"
#include "ROOTDOTNETBaseTObject.hpp"
#include "NetStringToConstCPP.hpp"

#include <TClass.h>
#pragma make_public(TObject)

using std::string;

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{
		DynamicHelpers::DynamicHelpers(void)
		{
		}

		//
		// Given the list of arguments, generate a prototype string
		// that CINT can understand for argument lookup.
		//
		string DynamicHelpers::GeneratePrototype(array<System::Object^> ^args)
		{
			string result = "";

			for (int index = 0; index < args->Length; index++)
			{
				auto arg = args[index];
				string thisType = "";
				auto gt = arg->GetType();
				if (gt == int::typeid)
				{
					thisType = "int";
				} else if (gt == long::typeid)
				{
					thisType = "long";
				} else if (gt == float::typeid)
				{
					thisType = "float";
				} else if (gt == double::typeid)
				{
					thisType = "double";
				} else if (gt == System::String::typeid)
				{
					thisType = "const char*";
				} else {
					// See if this is a class ptr that is part of the ROOT system.
					if (gt->IsSubclassOf(ROOTNET::Utility::ROOTDOTNETBaseTObject::typeid))
					{
						auto robj = static_cast<ROOTNET::Utility::ROOTDOTNETBaseTObject^>(arg);
						::TObject *tobj = robj->GetTObjectPointer();
						string rootname = string(tobj->IsA()->GetName());
						thisType = rootname + "*";
					} else {
						return "<>"; // Can't do it!
					}
				}

				if (result.size() == 0) {
					result = thisType;
				} else {
					result += "," + thisType;
				}
			}

			return result;
		}

		///
		/// Look for a root pointer
		///
		::TClass *DynamicHelpers::ExtractROOTClassInfoPtr (const string &tname)
		{
			int ptr = tname.rfind("*");
			if (ptr == tname.npos)
				return nullptr;

			auto nameonly = tname.substr(0, ptr);
			return ::TClass::GetClass(nameonly.c_str());
		}
	}
}
