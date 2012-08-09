#include "TreeManager.hpp"
#include "NetStringToConstCPP.hpp"
#include "TreeLeafExecutor.hpp"

#include "TTree.h"
#include "TLeaf.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

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
		/// Template class to deal with a very simple type of object
		///
		generic <class ST>
		ref class tle_simple_type : public TreeLeafExecutor
		{
		public:
			tle_simple_type (::TBranch *b)
				:_value (0), _branch(b), _last_entry (-1)
			{
				_value = new UInt_t();
				_branch->SetAddress(_value);
			}

			~tle_simple_type()
			{
				delete _value;
			}

			virtual System::Object ^execute (unsigned long entry) override
			{
				if (_last_entry != entry) {
					_branch->GetEntry(entry);
					_last_entry = entry;
				}
				return *_value;
			}

		private:
			UInt_t *_value;
			::TBranch *_branch;
			unsigned long _last_entry;
		};

		public ref class vector_accessor
		{
		public:
			size_t size() { return _array->size(); }
			property int default[int] {
				virtual int get (int index)
				{
					if (index < 0 || index > _array->size())
						throw gcnew System::IndexOutOfRangeException();
					return (*_array)[index];
				}
		    }

		public protected:
			vector_accessor ()
				: _array(nullptr)
			{}
			void set_pointer (vector<int> *ar)
			{
				_array = ar;
			}

		private:
			vector<int> *_array;
		};

		///
		/// Template class to deal with a vector of a simple object
		///
		generic<class ST>
		ref class tle_vector_type : public TreeLeafExecutor
		{
		public:
			tle_vector_type (::TBranch *b)
				:_value (0), _branch(b), _last_entry (-1)
			{
				_value = new vector<int>*();
				_branch->SetAddress(_value);
				_accessor = gcnew vector_accessor();
			}

			~tle_vector_type()
			{
				delete _value;
			}

			///
			/// Read in the vector, and notify our accessor.
			///
			virtual System::Object ^execute (unsigned long entry) override
			{
				if (_last_entry != entry) {
					_branch->GetEntry(entry);
					_last_entry = entry;
					_accessor->set_pointer(*_value);
				}
				return _accessor;
			}

		private:
			vector<int> **_value;
			vector_accessor ^_accessor;
			::TBranch *_branch;
			unsigned long _last_entry;
		};

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
			// we are done!
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
				result = gcnew tle_simple_type<UInt_t> (branch);
			}
			if (leaf_type == "vector<int>")
			{
				result = gcnew tle_vector_type<int> (branch);
			}

			// Cache it if it was good.
			if (result != nullptr)
				_executors[binder->Name] = result;

			return result;
		}
	}
}
