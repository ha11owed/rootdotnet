// This is the main DLL file.

#include "ROOTDotNet.h"

///
/// ROOT includes -- we have to dynamically walk the class list to figure out
/// what to do!
///
#include "TObject.h"
#include "TClass.h"
#include "TBranch.h"

#include "root_type_holder.hpp"

#include <string>
#include <map>

using std::map;
using std::string;
using namespace System;
using namespace System::Reflection;
using namespace System::Collections::Generic;

namespace ROOTNET {
	namespace Utility {

		///
		/// GetBestObject
		///
		///  Given a TObject pointer attempt to determine what type of object we are dealing with, and create it.
		///  Return the interface we are templated with.
		///
		///  NOTE: this only works if T is a pointer (i.e. TObject^).
		///
		generic<class T>
		where T: ref class
			T ROOTObjectServices::GetBestObject
			(const ::TObject *obj)
		{
			///
			/// Simple cases...
			///

			if (obj == 0) {
				return T();
			}

			///
			/// Determine the class type of the object that we are looking at.
			///

			TClass *cls = obj->IsA();
			if (cls == 0) {
				return T();
			}

			///
			/// Get the type that we can use to wrap this guy
			///

			Type ^class_type = root_type_holder::GetBestMatchType (cls);

			///
			/// Next find a ctor that takes only the C++ pointer as an argument. This is the ctor
			/// that is generated by the translation code. We will use this ctor to create the object.
			///

			array<ConstructorInfo^>^ ctors = class_type->GetConstructors();
			String ^class_name_ptr = class_type->Name->Substring(1) + "*";
			ConstructorInfo ^the_ctor;
			bool found_ctor = false;
			for (int i_ctor = 0; i_ctor < ctors->Length; i_ctor++) {
				array<ParameterInfo^>^ params = ctors[i_ctor]->GetParameters();
				if (params->Length != 1) {
					continue;
				}
				if (params[0]->ParameterType->FullName == class_name_ptr) {
					the_ctor = ctors[i_ctor];
					found_ctor = true;
					break;
				}
			}

			if (!found_ctor) {
				return T();
			}

			///
			/// Finally, we can box up the pointer and call the ctor.
			///

			array<Object^> ^arguments = gcnew array<Object^>(1);
			arguments[0] = Pointer::Box(const_cast<::TObject*>(obj), the_ctor->GetParameters()[0]->ParameterType);
			Object ^the_obj = the_ctor->Invoke (arguments);
			return (T) the_obj;
		}
	}
}