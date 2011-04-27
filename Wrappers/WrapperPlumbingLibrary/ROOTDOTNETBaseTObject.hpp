#pragma once

class TObject;

namespace ROOTNET
{
	namespace Utility
	{
		///
		/// The base-base-base-base object that is used for all objects that are wrapped by TObject.
		/// Actually, the main reason this is hear is so that we can cleanly factor the plumbing library code
		/// and the actual generated classes. :-)
		///
		/// Also includes simple Dynamic implementation. Note that the DLR will use any static implemenations first,
		/// and then call one of the Tryxxx methods that we override here.
		///
		public ref class ROOTDOTNETBaseTObject abstract : System::Dynamic::DynamicObject
		{
		public:
			ROOTDOTNETBaseTObject(void);
			/// Set the held object pointer to null, but don't delete it.
			virtual void SetNull (void) = 0;
			/// Delete the object and set the pointer to null.
			virtual void DeleteHeldObject (void) = 0;
			/// Drop the object from our internal tables. This is done just before it is to be deleted!
			virtual void DropObjectFromTables (void) = 0;
			/// Reset ownership to be not us so we don't delete it. Used when ROOT takes over ownership because of something we do.
			void SetNativePointerOwner(bool owner) { _owner = owner; }

			/// Dynamic implementions.
			virtual bool TryInvokeMember (System::Dynamic::InvokeMemberBinder ^binder, array<Object^> ^args, Object^% result) override;

			/// Destructors and finalizers. The finalizer will delete the ROOT memory.
			/// The dtor will call the finalizer (so it doesn't happen twice!). Probably no
			/// other language besides C++ will ever use the dtor.
			~ROOTDOTNETBaseTObject(void);
			!ROOTDOTNETBaseTObject(void);
		protected:
			bool _owner;
		};
	}
}