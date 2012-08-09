#include "TreeManager.hpp"
#include "NetStringToConstCPP.hpp"
#include "TreeLeafExecutor.hpp"
#include "ROOTDotNet.h"
#include "ROOTDOTNETBaseTObject.hpp"

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

		///
		/// Allow for iteration through a vector array.
		///
		public ref class vector_accessor_enumerator : System::Collections::Generic::IEnumerator<int>
		{
		public:
			inline vector_accessor_enumerator (vector<int> *ar)
				: _array(ar), _index(-1)
			{
			}

			inline ~vector_accessor_enumerator (void)
			{}

			bool MoveNext (void)
			{
				_index++;
				return _index < _array->size();
			}

			virtual bool MoveNext2() sealed = System::Collections::IEnumerator::MoveNext
			{ return MoveNext(); }

			property int Current
			{
				virtual int get()
				{ return _array->at(_index); }
			}

			property Object^ Current2
			{
				virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get
				{ return Current; }
			}

			void Reset()
			{ _index = -1; }

			virtual void Reset2 () sealed = System::Collections::IEnumerator::Reset
			{ Reset(); }
		private:
			vector<int> *_array;
			long _index;
		};

		public ref class vector_accessor : System::Collections::Generic::IEnumerable<int>
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

			virtual System::Collections::Generic::IEnumerator<int> ^GetEnumerator()
			{
				return gcnew vector_accessor_enumerator (_array);
			}

			virtual System::Collections::IEnumerator ^GetEnumerator2() sealed = System::Collections::IEnumerable::GetEnumerator
			{
				 return GetEnumerator();
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

		public ref class vector_accessor_enumerator_string : System::Collections::Generic::IEnumerator<System::String ^>
		{
		public:
			inline vector_accessor_enumerator_string (vector<string> *ar)
				: _array(ar), _index(-1)
			{
			}

			inline ~vector_accessor_enumerator_string (void)
			{}

			bool MoveNext (void)
			{
				_index++;
				return _index < _array->size();
			}

			virtual bool MoveNext2() sealed = System::Collections::IEnumerator::MoveNext
			{ return MoveNext(); }

			property System::String ^Current
			{
				virtual System::String ^get()
				{ return gcnew System::String(_array->at(_index).c_str()); }
			}

			property Object^ Current2
			{
				virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get
				{ return Current; }
			}

			void Reset()
			{ _index = -1; }

			virtual void Reset2 () sealed = System::Collections::IEnumerator::Reset
			{ Reset(); }
		private:
			vector<string> *_array;
			long _index;
		};

		public ref class vector_accessor_string : System::Collections::Generic::IEnumerable<System::String^>
		{
		public:
			size_t size() { return _array->size(); }
			property System::String ^default[int] {
				virtual System::String ^get (int index)
				{
					if (index < 0 || index > _array->size())
						throw gcnew System::IndexOutOfRangeException();
					return gcnew System::String ((*_array)[index].c_str());
				}
		    }

			virtual System::Collections::Generic::IEnumerator<System::String^> ^GetEnumerator()
			{
				return gcnew vector_accessor_enumerator_string (_array);
			}

			virtual System::Collections::IEnumerator ^GetEnumerator2() sealed = System::Collections::IEnumerable::GetEnumerator
			{
				 return GetEnumerator();
			}

		public protected:
			vector_accessor_string ()
				: _array(nullptr)
			{}
			void set_pointer (vector<string> *ar)
			{
				_array = ar;
			}

		private:
			vector<string> *_array;
		};

		///
		/// branch that is a vector of strings
		///
		ref class tle_vector_string : public TreeLeafExecutor
		{
		public:
			tle_vector_string (::TBranch *b)
				:_value (0), _branch(b), _last_entry (-1)
			{
				_value = new vector<string>*();
				_branch->SetAddress(_value);
				_accessor = gcnew vector_accessor_string();
			}

			~tle_vector_string()
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
			vector<string> **_value;
			vector_accessor_string ^_accessor;
			::TBranch *_branch;
			unsigned long _last_entry;
		};

		///
		/// No special wrapper - but this class is known to ROOT's dictionary, so, we'll just
		/// do our best to use a wrapper!
		///
		ref class tle_root_object : public TreeLeafExecutor
		{
		public:
			tle_root_object (::TBranch *b, ::TClass *classInfo)
				:_value (0), _branch(b), _last_entry (-1), _class_info(classInfo)
			{
				_value = new void*();
				_branch->SetAddress(_value);
				_is_tobject = classInfo->InheritsFrom("TObject");
			}

			~tle_root_object()
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
				}

				if (_is_tobject) {
					auto obj = ROOTObjectServices::GetBestObject<ROOTDOTNETBaseTObject^>(reinterpret_cast<::TObject*>(*_value));
					obj->SetNativePointerOwner (false); // TTree owns this!
					return obj;
#ifdef notyet
					// TODO: fix this up as soon as we merge the branches
				} else {
					auto obj = ROOTObjectServices::GetBestVoidObject<ROOTDOTNETVoidObject^>(obj);
					obj.SetOwner (false); // TTree owns this!
					return obj;
#endif
				}
			}

		private:
			void **_value;
			::TBranch *_branch;
			::TClass *_class_info;
			unsigned long _last_entry;
			bool _is_tobject;
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
				result = gcnew tle_simple_type<UInt_t> (branch);
			}
			if (leaf_type == "vector<int>")
			{
				result = gcnew tle_vector_type<int> (branch);
			}
			if (leaf_type == "vector<string>")
			{
				result = gcnew tle_vector_string (branch);
			}

			//
			// Is this something known to root?
			//

			auto cls_info = ::TClass::GetClass(leaf_type.c_str());
			if (cls_info != nullptr)
			{
				result = gcnew tle_root_object (branch, cls_info);
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
