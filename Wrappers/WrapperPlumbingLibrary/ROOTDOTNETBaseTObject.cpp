
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

namespace {
	//
	// return the tclass pointer if this is a strict pointer to a root class.
	//  TAxis* for example, but not TAxis&.
	//
	::TClass *ExtractROOTClassInfoPtr (const string &tname)
	{
		int ptr = tname.rfind("*");
		if (ptr == tname.npos)
			return nullptr;

		auto nameonly = tname.substr(0, ptr);
		return ::TClass::GetClass(nameonly.c_str());
	}
}

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

			ostringstream arg_builder;
			bool first_arg = true;
			for each (auto arg in args)
			{
				if (!first_arg)
					arg_builder << ",";
				first_arg = false;

				auto gt = arg->GetType();
				if (gt == int::typeid || gt == long::typeid || gt == short::typeid)
				{
					long i = (long) arg;
					arg_builder << i;
				} else if (gt == float::typeid || gt == double::typeid)
				{
					double d = (double) arg;
					arg_builder << d;
				} else if (gt == System::String::typeid)
				{
					ROOTNET::Utility::NetStringToConstCPP carg((System::String^) arg);
					arg_builder << "\"" << carg << "\"";
				} else
				{
					throw gcnew System::InvalidOperationException("Legal prototype on method call, but no converstion - what the heck!");
				}
			}

			string arg_list = arg_builder.str();

			//
			// Do the invocation. How we do this depends on the return type, unfortunately!
			//

			string return_type_name (resolveTypedefs(method.GetMethod()->GetReturnTypeName()));
			if (return_type_name == "double" || return_type_name == "float")
			{
				double val = 0;
				method.Execute(GetTObjectPointer(), arg_list.c_str(), val);
				if (return_type_name == "double") {
					result = val;
				} else {
					result = (float) val;
				}
				return true;
			} if (return_type_name == "int" || return_type_name == "long" || return_type_name == "short")
			{
				long val = 0;
				method.Execute(GetTObjectPointer(), arg_list.c_str(), val);
				if (return_type_name == "int") {
					result = (int) val;
				} else {
					result = val;
				}
				return true;
			} else if (return_type_name == "void") {
				method.Execute(GetTObjectPointer(), arg_list.c_str());
				result = nullptr;
				return true;
			} else if (return_type_name == "const char*") {
				char *r;
				method.Execute(GetTObjectPointer(), arg_list.c_str(), &r);
				result = gcnew ::System::String(r);
				return true;
			} else {
				// Check to see if this is a ROOT class pointer
				auto cls = ExtractROOTClassInfoPtr(return_type_name);
				if (cls == nullptr)
					return false;
				if (!cls->InheritsFrom("TObject"))
					return false;

				// It is a pointer. Do the work!
				char *ptr;
				method.Execute(GetTObjectPointer(), arg_list.c_str(), &ptr);
				TObject *obj = reinterpret_cast<TObject*>(ptr); // Danger - but this is the way of CINT! :(
				result = ROOTObjectServices::GetBestObject<ROOTDOTNETBaseTObject^>(obj);
				return true;
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
			string current (type);
			while (true)
			{
				auto dtinfo = gROOT->GetType(current.c_str());
				if (dtinfo == nullptr)
					return current;

				string newname = dtinfo->GetTypeName();
				if (newname == current)
					return current;
				current = newname;
			}
		}
	}
}
