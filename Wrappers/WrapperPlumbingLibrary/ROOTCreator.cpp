///
/// ROOTCreator
///
///  This class enables easily creating new ROOT objects dynamically.
///

#include "ROOTCreator.h"
#include "NetStringToConstCPP.hpp"
#include "DynamicHelpers.h"
#include "ROOTDotNet.h"
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTDynamicException.h"

#include <TClass.h>
#include <TMethodCall.h>
#include <TApplication.h>

#pragma make_public(TObject)

#include <string>
using std::string;

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{

		ROOTCreator::ROOTCreator(void)
		{
		}

		///
		/// Create an object, but do it by name rather than using the DLR... when an object name contains template arguments, etc.
		/// It should be cast to a dynamic type by the calling guy.
		///
		System::Object^ ROOTCreator::CreateByName(System::String^ root_object_name, array<Object^> ^args)
		{
			//
			// Some basic init. Very quick if it has been done already
			//

			TApplication::CreateApplication();

			//
			// Check out the class that we are going to create. Even possible?
			//

			ROOTNET::Utility::NetStringToConstCPP class_name(root_object_name);
			auto c = TClass::GetClass(class_name);
			if (c == nullptr)
				throw gcnew ROOTDynamicException("ROOT does not know about the class");

			if (!c->InheritsFrom("TObject"))
				throw gcnew ROOTDynamicException("The class does not inherrit from TObject and so can't be handled by ROOT.NET");

			//
			// Next parse through the ctor arguments, and see if we can find the constructor method.
			//

			auto proto = DynamicHelpers::GeneratePrototype(args);

			//
			// If this is the special case of a trivial ctor, then we can use the cls.New call.
			//

			if (proto.size() == 0)
			{
				auto obj = static_cast<::TObject*>(c->New());
				return ROOTObjectServices::GetBestObject<ROOTDOTNETBaseTObject^>(obj);
			}

			return nullptr;
#ifdef notyet
			TMethodCall method;
			method.InitWithPrototype(c, class_name, proto.c_str());
			if (!method.IsValid())
				return false;

			//
			// Do the method lookup next. We have copied some of the code we are dealing with from
			// the SFRame project.
			// http://sframe.svn.sourceforge.net/viewvc/sframe/SFrame/trunk/core/src/SCycleOutput.cxx
			//

			auto prototype = GeneratePrototype(args);
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
#endif
		}

		///
		/// Called to create a new ROOT object. The method name is the name of the object we want to create.
		/// The args are the ctor arguments.
		/// And the result will be the new objet!
		///
		/// Class must come from TObject for now...
		///
		bool ROOTCreator::TryInvokeMember (System::Dynamic::InvokeMemberBinder ^binder, array<Object^> ^args, Object^% result)
		{
			result = CreateByName(binder->Name, args);
			return result != nullptr;
		}
	}
}
