
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTObjectManager.h"

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
		/// Do a dynamic lookup of a method and see if we can't invoke it.
		///
		/// There are lots of limitations to this:
		///
		bool ROOTDOTNETBaseTObject::TryInvokeMember (System::Dynamic::InvokeMemberBinder ^binder, array<Object^> ^args, Object^% result)
		{
			return false;
		}
	}
}
