#pragma once


namespace ROOTNET
{
	namespace Utility
	{
		///
		/// This class is used to create ROOT objects when
		/// it has to be done dynamically.
		///
		public ref class ROOTCreator :
		public System::Dynamic::DynamicObject
		{
		public:
			ROOTCreator(void);

			// Called by doing the object name, and argumets for its ctor
			virtual bool TryInvokeMember (System::Dynamic::InvokeMemberBinder ^binder, array<Object^> ^args, Object^% result) override;

			// Craete an object given just a name. Should be cast to a dynamic type in C#!
			static System::Object ^ CreateByName(System::String^ ROOTObjectName, array<Object^> ^args);
		};
	}
}
