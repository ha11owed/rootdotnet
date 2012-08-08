///
/// Manage all the resources required by a tree as we iterate through it.
///

#pragma once

namespace ROOTNET
{
	namespace Utility
	{

		class TreeLeafExecutor;

		ref class TreeManager
		{
		public:
			TreeManager(void);

			// Return an executor that will fetch the given leaf and return its value as a Root Dot Net object
			// (or some sort of simple type).
			TreeLeafExecutor *get_executor (System::Dynamic::GetMemberBinder ^binder);
		};
	}
}
