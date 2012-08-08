#include "TreeManager.hpp"
#include "NetStringToConstCPP.hpp"
#include "TreeLeafExecutor.hpp"

#include "TTree.h"
#include "TLeaf.h"

#include <string>

using std::string;

class TTree;
#pragma make_public(TTree)

namespace ROOTNET
{
	namespace Utility
	{
		TreeManager::TreeManager(::TTree *tree)
			: _tree (tree), _data (new TreeManagerData())
		{
		}

		///
		/// Template class to deal with a very simple type of object
		///
		template <class ST>
		class tle_simple_type : public TreeLeafExecutor
		{
		public:
			tle_simple_type (::TBranch *b)
				:_value (0), _branch(b)
			{
				_branch->SetAddress(&_value);
			}

			System::Object ^execute (unsigned long entry)
			{
				_branch->GetEntry(entry);
				return _value;
			}

		private:
			ST _value;
			::TBranch *_branch;
		};

		///
		/// Our main job. Sort through everything we know about this leaf (if it exists) and attempt to find
		/// or create an executor for it that will return the object.
		///
		TreeLeafExecutor *TreeManager::get_executor (System::Dynamic::GetMemberBinder ^binder)
		{
			//
			// Get the leaf name, see if we've already looked for one of these guys.
			//

			NetStringToConstCPP leaf_name_net (binder->Name);
			string leaf_name (leaf_name_net);
			auto itr = _data->_executors.find(leaf_name);
			if (itr != _data->_executors.end())
				return itr->second;

			//
			// Now we are going to have to see if we can find the branch. If the branch isn't there, then
			// we are done!
			//

			auto branch = _tree->GetBranch(leaf_name.c_str());
			if (branch == nullptr)
				return nullptr;

			auto leaf = branch->GetLeaf(leaf_name.c_str());
			if (leaf == nullptr)
				return nullptr;
			string leaf_type (leaf->GetTypeName());

			TreeLeafExecutor *result = nullptr;

			//
			// Next, we have to see if we can use the type to figure out what sort of executor we can use.
			//

			if (leaf_type == "UInt_t")
			{
				result = new tle_simple_type<UInt_t> (branch);
			}

			// Cache it if it was good.
			if (result != nullptr)
				_data->_executors[leaf_name] = result;

			return result;
		}
	}
}
