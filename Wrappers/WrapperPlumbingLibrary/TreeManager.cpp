#include "TreeManager.hpp"
#include "NetStringToConstCPP.hpp"
#include "TreeLeafExecutor.hpp"
#include "ROOTDotNet.h"
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTDOTNETVoidObject.hpp"
#include "TreeLeafExecutorRaw.h"

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

		class TreeLeafExecutorSingleValueRaw : public TreeLeafExecutorRaw
		{
		public:
			TreeLeafExecutorSingleValueRaw (::TBranch *b)
				: TreeLeafExecutorRaw (b)
			{}

			virtual System::Object^ execute (unsigned long entry)
			{
				update(entry);
				return value();
			}
			virtual System::Object ^value (void) = 0;
		};

		template <class ValueType>
		class tle_simple_type_accessor : public TreeLeafExecutorSingleValueRaw
		{
		public:
			tle_simple_type_accessor (::TBranch *b)
				: TreeLeafExecutorSingleValueRaw (b)
			{
				b->SetAddress(&_value);
			}

			System::Object ^value (void) {return _value;}

		public:
			ValueType _value;
		};

		generic <class ST>
		ref class tle_simple_type : public TreeLeafExecutor
		{
		public:
			// Init and take ownership of the executor
			tle_simple_type (TreeLeafExecutorSingleValueRaw *exe)
				: _exe(exe)
			{
			}

			~tle_simple_type()
			{
				delete _exe;
			}

			virtual System::Object ^execute (unsigned long entry) override
			{
				_exe->update(entry);
				return _exe->value();
			}

		private:
			TreeLeafExecutorSingleValueRaw *_exe;
		};

		///
		/// Allow for iteration through a vector array.
		///
		class TreeLeafExecutorVectorRaw : public TreeLeafExecutorRaw
		{
		public:
			TreeLeafExecutorVectorRaw (::TBranch *b)
				: TreeLeafExecutorRaw(b)
			{}
			virtual size_t size(void) = 0;
			virtual System::Object^ at (size_t index) = 0;
		};

		public ref class vector_accessor_enumerator : System::Collections::Generic::IEnumerator<System::Object^>
		{
		public:
			inline vector_accessor_enumerator (TreeLeafExecutorVectorRaw *exe)
				: _exe (exe), _index (-1)
			{
			}

			inline ~vector_accessor_enumerator (void)
			{}

			bool MoveNext (void)
			{
				_index++;
				return _index < _exe->size();
			}

			virtual bool MoveNext2() sealed = System::Collections::IEnumerator::MoveNext
			{ return MoveNext(); }

			property System::Object^ Current
			{
				virtual System::Object^ get() {
					if (_index < 0 || _index >= _exe->size())
						throw gcnew System::IndexOutOfRangeException();
					return _exe->at(_index);
				}
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
			TreeLeafExecutorVectorRaw *_exe;
			long _index;
		};

		public ref class vector_accessor_enumerable : System::Collections::Generic::IEnumerable<System::Object^>
		{
		public:
			size_t size() { return _exe->size(); }
			property System::Object ^default[int] {
				virtual System::Object ^get (int index)
				{
					if (index < 0 || index > _exe->size())
						throw gcnew System::IndexOutOfRangeException();
					return _exe->at(index);
				}
		    }

			virtual System::Collections::Generic::IEnumerator<System::Object^> ^GetEnumerator()
			{
				return gcnew vector_accessor_enumerator (_exe);
			}

			virtual System::Collections::IEnumerator ^GetEnumerator2() sealed = System::Collections::IEnumerable::GetEnumerator
			{
				 return GetEnumerator();
			}

		public protected:
			vector_accessor_enumerable (TreeLeafExecutorVectorRaw *exe)
				: _exe (exe)
			{}

		private:
			TreeLeafExecutorVectorRaw *_exe;
		};

		///
		/// Template class to deal with a vector of a simple object
		///

		template <class ValueType>
		class tle_vector_type_exe : public TreeLeafExecutorVectorRaw
		{
		public:
			tle_vector_type_exe (::TBranch *b)
				: TreeLeafExecutorVectorRaw(b), _value(nullptr)
			{
				b->SetAddress (&_value);
			}

			size_t size(void) { return _value->size(); }
			System::Object ^at (size_t index) { return _value->at(index); }

		private:
			vector<ValueType> *_value;
		};

		class tle_vector_string_type_exe : public TreeLeafExecutorVectorRaw
		{
		public:
			tle_vector_string_type_exe (::TBranch *b)
				: TreeLeafExecutorVectorRaw(b), _value(nullptr)
			{
				b->SetAddress (&_value);
			}

			size_t size(void) { return _value->size(); }
			System::Object ^at (size_t index) { return gcnew System::String(_value->at(index).c_str()); }

		private:
			vector<string> *_value;
		};

		ref class tle_vector_type : public TreeLeafExecutor
		{
		public:
			tle_vector_type (TreeLeafExecutorVectorRaw *exe)
				: _exe (exe)
			{
			}

			~tle_vector_type()
			{
				delete _exe;
			}

			///
			/// Read in the vector, and notify our accessor.
			///
			virtual System::Object ^execute (unsigned long entry) override
			{
				_exe->update(entry);
				return gcnew vector_accessor_enumerable(_exe);
			}

		private:
			TreeLeafExecutorVectorRaw *_exe;
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
				} else {
					auto obj = ROOTObjectServices::GetBestNonTObjectObject(*_value, _class_info);
					obj->SetNativePointerOwner (false); // TTree owns this!
					return obj;
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
