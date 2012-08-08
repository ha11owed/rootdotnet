#include "TreeManager.hpp"


namespace ROOTNET
{
	namespace Utility
	{
		TreeManager::TreeManager(void)
		{
		}

		///
		/// Our main job. Sort through everything we know about this leaf (if it exists) and attempt to find
		/// or create an executor for it that will return the object.
		///
		TreeLeafExecutor *TreeManager::get_executor (System::Dynamic::GetMemberBinder ^binder)
		{
			return nullptr;
		}
	}
}
