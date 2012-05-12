
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTObjectManager.h"
#include "NetStringToConstCPP.hpp"
#include "DynamicHelpers.h"

#include <TObject.h>
#include <TClass.h>
#include <TMethodCall.h>
#include <TFunction.h>

#include <string>

using std::string;

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
				return false;
			auto classSpec = GetTObjectPointer()->IsA();
			if (classSpec == nullptr)
				return false;

			//
			// Do the method lookup next. We have copied some of the code we are dealing with from
			// the SFRame project.
			// http://sframe.svn.sourceforge.net/viewvc/sframe/SFrame/trunk/core/src/SCycleOutput.cxx
			//

			auto prototype = DynamicHelpers::GeneratePrototype(args);
			if (prototype == "<>")
				return false;

		    ROOTNET::Utility::NetStringToConstCPP method_name(binder->Name);
			TMethodCall method;
			method.InitWithPrototype(classSpec, method_name, prototype.c_str());
			if (!method.IsValid())
				return false;

			//
			// Now build up the argument list for CINT
			//

			for each (auto arg in args)
			{
				if (arg->GetType() == int::typeid || arg->GetType() == long::typeid)
				{
					long i = (long) arg;
					method.SetParam(i);
				} else if (arg->GetType() == float::typeid || arg->GetType() == double::typeid)
				{
					double d = (double) arg;
					method.SetParam(d);
				}
			}

			//
			// Do the invocation. How we do this depends on the return type, unfortunately!
			//

			string return_type_name (resolveTypedefs(method.GetMethod()->GetReturnTypeName()));
			if (return_type_name == "double" || return_type_name == "float")
			{
				double val = 0;
				method.Execute(GetTObjectPointer(), val);
				if (return_type_name == "double") {
					result = val;
				} else {
					result = (float) val;
				}
				return true;
			} if (return_type_name == "int" || return_type_name == "long")
			{
				long val = 0;
				method.Execute(GetTObjectPointer(), val);
				if (return_type_name == "int") {
					result = (int) val;
				} else {
					result = val;
				}
				return true;
			} else {
				return false;
			}

			//
			// Translate the result back into something we know about!
			//

			return false;
		}

		//
		// Make sure all type-defs are taken care of.
		//
		string ROOTDOTNETBaseTObject::resolveTypedefs(const std::string &type)
		{
			if (type == "Int_t")
				return "int";
			if (type == "Double_t")
				return "double";
			if (type == "Float_t")
				return "float";
			if (type == "Long_t")
				return "long";
			return type;
		}
	}
}
