#include "TreeManager.hpp"
#include "NetStringToConstCPP.hpp"
#include "TreeLeafExecutor.hpp"
#include "ROOTDotNet.h"
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTDOTNETVoidObject.hpp"

#include "TreeROOTValueExecutor.h"
#include "TreeVectorValueExecutor.h"
#include "TreeSimpleValueExecutor.h"

#include "TTree.h"
#include "TLeaf.h"

#include <string>
#include <vector>
#include <sstream>

using std::string;
using std::vector;
using std::ostringstream;

class TTree;
#pragma make_public(TTree)

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{
		TreeManager::TreeManager(::TTree *tree)
			: _tree (tree)
		{
			_executors = gcnew System::Collections::Generic::Dictionary<System::String^, TreeLeafExecutor ^>();
		}

		///
		/// Our main job. Sort through everything we know about this leaf (if it exists) and attempt to find
		/// or create an executor for it that will return the object.
		///
		TreeLeafExecutor ^TreeManager::get_executor (System::Dynamic::GetMemberBinder ^binder)
		{
			//
			// Get the leaf name, see if we've already looked for one of these guys.
			//

			if (_executors->ContainsKey(binder->Name))
				return _executors[binder->Name];

			//
			// Now we are going to have to see if we can find the branch. If the branch isn't there, then
			// we are done - this is just a case of the user not knowing what the leaf name is. We let the
			// DLR deal with this error.
			//

			NetStringToConstCPP leaf_name_net (binder->Name);
			string leaf_name (leaf_name_net);
			auto branch = _tree->GetBranch(leaf_name.c_str());
			if (branch == nullptr)
				return nullptr;

			auto leaf = branch->GetLeaf(leaf_name.c_str());
			if (leaf == nullptr)
				return nullptr;
			string leaf_type (leaf->GetTypeName());

			TreeLeafExecutor ^result = nullptr;

			//
			// Next, we have to see if we can use the type to figure out what sort of executor we can use.
			//

			if (leaf_type == "UInt_t")
			{
				result = gcnew tle_simple_type<UInt_t> (new tle_simple_type_accessor<UInt_t> (branch));
			} else
			if (leaf_type == "float")
			{
				result = gcnew tle_simple_type<float> (new tle_simple_type_accessor<float> (branch));
			} else
			if (leaf_type == "vector<int>")
			{
				result = gcnew tle_vector_type (new tle_vector_type_exe<int>(branch));
			} else
			if (leaf_type == "vector<float>")
			{
				result = gcnew tle_vector_type (new tle_vector_type_exe<float>(branch));
			} else
			if (leaf_type == "vector<string>")
			{
				result = gcnew tle_vector_type (new tle_vector_string_type_exe(branch));
			} else {

				//
				// Is this something known to root?
				//

				auto cls_info = ::TClass::GetClass(leaf_type.c_str());
				if (cls_info != nullptr)
				{
					result = gcnew tle_root_object (branch, cls_info);
				}
			}

			//
			// If we couldn't match that means we have the proper leaf, but we don't know how to deal with
			// the type. So we actually have a failing of this DLR section. Pop an error so the user doesn't
			// get confused about what went wrong.
			//

			if (result == nullptr)
			{
				ostringstream err;
				err << "TTree leaf '" << leaf_name << "' has type '" << leaf_type << "'; ROOT.NET's dynamic TTree infrastructure doesn't know how to deal with that type, unfortunately!";
				throw gcnew System::InvalidOperationException(gcnew System::String(err.str().c_str()));
			}

			//
			// We have a good reader if we make it here. Cache it so next time through this is fast
			// (eventually the DLR will do the caching for us most of the time!).
			//

			_executors[binder->Name] = result;
			return result;
		}
	}
}
