#include "DynamicHelpers.h"

using std::string;

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
				if (arg->GetType() == int::typeid)
				{
					thisType = "int";
				} else if (arg->GetType() == long::typeid)
				{
					thisType = "long";
				} else if (arg->GetType() == float::typeid)
				{
					thisType = "float";
				} else if (arg->GetType() == double::typeid)
				{
					thisType = "double";
				} else {
					return "<>"; // Can't do it!
				}

				if (result.size() == 0) {
					result = thisType;
				} else {
					result += "," + thisType;
				}
			}

			return result;
		}
	}
}
