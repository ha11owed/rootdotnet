
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

#pragma make_public(TObject)
#pragma make_public(TClass)


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

			auto classSpec = GetClassInfo();
			if (classSpec == nullptr)
				throw gcnew System::InvalidOperationException("Attempt to call method on ROOT object that has no class info - impossible!");

			//
			// Get the call site setup
			//

		    ROOTNET::Utility::NetStringToConstCPP method_name(binder->Name);
			auto caller = DynamicHelpers::GetFunctionCaller(classSpec, (string) method_name, args);
			if (caller == nullptr)
				return false;
			void *ptr = GetTObjectPointer();
			if (ptr == nullptr)
				ptr = GetVoidPointer();
			return caller->Call(ptr, args, result);

		}
	}
}
