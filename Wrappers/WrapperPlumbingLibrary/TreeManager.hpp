///
/// Manage all the resources required by a tree as we iterate through it.
///

#pragma once

#include <map>
#include <string>
#include <vector>

class TTree;

namespace ROOTNET
{
	namespace Utility
	{

		class TreeLeafExecutor;

		struct TreeManagerData
		{
			std::map<std::string, TreeLeafExecutor *> _executors;
		};

		ref class TreeManager
		{
		public:
			TreeManager(::TTree *tree);

			// Return an executor that will fetch the given leaf and return its value as a Root Dot Net object
			// (or some sort of simple type).
			TreeLeafExecutor *get_executor (System::Dynamic::GetMemberBinder ^binder);

		private:
			// Keep most of our data in here. We have to do this b/c this is a managed class.
			TreeManagerData *_data;

			// Track the tree.
			::TTree *_tree;
		};
	}
}
