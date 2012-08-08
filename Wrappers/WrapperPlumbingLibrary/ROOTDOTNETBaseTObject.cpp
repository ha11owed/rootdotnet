
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTObjectManager.h"
#include "NetStringToConstCPP.hpp"
#include "DynamicHelpers.h"
#include "ROOTDynamicException.h"
#include "ROOTDotNet.h"

#include <TObject.h>
#include <TClass.h>
#include <TMethodCall.h>
#include <TFunction.h>
#include <TROOT.h>
#include <TDataType.h>
#include <sstream>
#include <string>

using std::string;
using std::ostringstream;

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{
		ROOTDOTNETBaseTObject::ROOTDOTNETBaseTObject(void)
			: _owner(false), _whyNull(ReasonPointerNullEnum::kNothingHappenedYet)
		{
		}

		/// dtor calls the finalizer
		/// This is called only if we manually delete the object
		/// (in a C# program, this would never get called).
		ROOTDOTNETBaseTObject::~ROOTDOTNETBaseTObject()
		{
			this->!ROOTDOTNETBaseTObject();
		}

		/// Finalizer
		/// This is called by the GC as it is being cleaned up. This is the 
		/// normal path to deletion in C#, and is run on a seperate thread
		/// (the finalizer thread).
		ROOTDOTNETBaseTObject::!ROOTDOTNETBaseTObject()
		{
			DropObjectFromTables();
			if (_owner) {
				DeleteHeldObject();
			} else {
				SetNull();
			}
			SetNullReason(ReasonPointerNullEnum::kObjectFinalized);
		}

		///
		/// Change (perhaps) who is the owner. It would be nice
		void ROOTDOTNETBaseTObject::SetNativePointerOwner(bool newIsOwner)
		{
			_owner = newIsOwner;
		}

		///
		/// Do a dynamic lookup of a method and see if we can't invoke it, and then use CINT to do the invokation.
		/// This is a fairly expensive operation, but on the other hand, it means not having to link against
		/// anything.
		///
		/// This version has lots of limitations which will chagne over time
		///  - Only basic types are supported (int, float, short, etc.).
		///
		bool ROOTDOTNETBaseTObject::TryInvokeMember (System::Dynamic::InvokeMemberBinder ^binder, array<Object^> ^args, Object^% result)
		{
			//
			// Get the TClass for our class pointer
			//

			if (GetTObjectPointer() == nullptr)
				throw gcnew ROOTDynamicException("Attempt to call method on null ptr object!");
			auto classSpec = GetTObjectPointer()->IsA();
			if (classSpec == nullptr)
				throw gcnew System::InvalidOperationException("Attempt to call method on ROOT object that has no class info - impossible!");

			//
			// Get the call site setup
			//

		    ROOTNET::Utility::NetStringToConstCPP method_name(binder->Name);
			auto caller = DynamicHelpers::GetFunctionCaller(classSpec, (string) method_name, args);
			if (caller == nullptr)
				return false;
			return caller->Call(GetTObjectPointer(), args, result);
		}

		///
		/// Property access. Here is how we work. First, we look for
		///  GetXXX with no arguments. If that fails, then we look for "XXX" with no
		/// arguments. If all of that fails, then we fail. Otherwise, we call that method
		/// to get the result.
		///
		bool ROOTDOTNETBaseTObject::TryGetMember (System::Dynamic::GetMemberBinder ^binder, Object^% result)
		{
			//
			// Get the TClass for our class pointer
			//

			if (GetTObjectPointer() == nullptr)
				throw gcnew ROOTDynamicException("Attempt to call method on null ptr object!");
			auto classSpec = GetTObjectPointer()->IsA();
			if (classSpec == nullptr)
				throw gcnew System::InvalidOperationException("Attempt to call method on ROOT object that has no class info - impossible!");

			//
			// See if we can find a method call "GetXXX()". If that fails, attempt to call "XXX()".
			//

		    ROOTNET::Utility::NetStringToConstCPP method_name_net(binder->Name);

			string method_name ("Get" + method_name_net);
			array<Object^> ^empty_args = gcnew array<Object^>(0);
			auto caller = DynamicHelpers::GetFunctionCaller(classSpec, (string) method_name, empty_args);
			if (caller == nullptr)
			{
				string method_name (method_name_net);
				auto caller = DynamicHelpers::GetFunctionCaller(classSpec, (string) method_name, empty_args);
			}
			if (caller == nullptr)
				return false;

			return caller->Call(GetTObjectPointer(), empty_args, result);
		}
	}
}
