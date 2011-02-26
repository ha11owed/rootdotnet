
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTObjectManager.h"

namespace ROOTNET
{
	namespace Utility
	{
		ROOTDOTNETBaseTObject::ROOTDOTNETBaseTObject(void)
			: _owner(false)
		{
		}

		/// dtor calls the finalizer
		ROOTDOTNETBaseTObject::~ROOTDOTNETBaseTObject()
		{
			this->!ROOTDOTNETBaseTObject();
		}

		/// Finalizer releases unmanaged resources.
		ROOTDOTNETBaseTObject::!ROOTDOTNETBaseTObject()
		{
			DropObjectFromTables();
			if (_owner) {
				DeleteHeldObject();
			} else {
				SetNull();
			}
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
