//
// Helper functions for dealing with dynamic infrastructure and the DLR
//
#pragma once

#include <string>

namespace ROOTNET
{
	namespace Utility
	{
		class DynamicHelpers
		{
		public:
			DynamicHelpers(void);

			// Given a list of arguments, generate an argument list.
			static std::string GeneratePrototype(array<System::Object^> ^args);
		};
	}
}

