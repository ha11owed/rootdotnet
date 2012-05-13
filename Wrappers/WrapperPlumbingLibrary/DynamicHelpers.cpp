#include "DynamicHelpers.h"
#include "ROOTDOTNETBaseTObject.hpp"
#include "NetStringToConstCPP.hpp"
#include "ROOTDOTNETBaseTObject.hpp"
#include "ROOTDotNet.h"

#include <Api.h>
#include <TClass.h>
#include <TList.h>
#include <TMethod.h>
#include <TMethodArg.h>
#include <TROOT.h>
#include <TDataType.h>

#include <typeinfo>
#include <sstream>

#pragma make_public(TObject)

using std::string;
using std::vector;
using std::ostringstream;

#ifdef nullptr
#undef nullptr
#endif

namespace {
	using namespace ROOTNET::Utility;

	class RTCString : public ROOTTypeConverter
	{
	public:
		RTCString()
			: _cache (nullptr)
		{}
		~RTCString()
		{
			delete[] _cache;
		}

		string GetArgType() const { return "const char*";}

		void SetArg (System::Object ^obj, Cint::G__CallFunc *func)
		{
			ROOTNET::Utility::NetStringToConstCPP carg((System::String^) obj);
			string sarg(carg);
			_cache = new char[sarg.size()+1];
			strncpy (_cache, sarg.c_str(), sarg.size());
			_cache[sarg.size()] = 0;
			func->SetArg(reinterpret_cast<long>(_cache));
		}
		
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			return false;
		}
	private:
		// We need to make sure the string stays around...
		char *_cache;
	};

	template<typename T>
	class RTCBasicType : public ROOTTypeConverter
	{
	public:
		string GetArgType() const { return typeid(T).name(); }
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func) {}
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			return false;
		}
	private:
	};

	//
	// Pointer to an object.
	//
	class RTCROOTType : public ROOTTypeConverter
	{
	public:
		inline RTCROOTType (TClass *cls)
			: _cls(cls)
		{}

		string GetArgType() const { return string(_cls->GetName()) + "*"; }
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func) {}

		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			return false;
		}
	private:
		::TClass *_cls;
	};

	// Direct object, no pointer. we own the thing now.
	class RTCROOTTypeDirectObject : public ROOTTypeConverter
	{
	public:
		inline RTCROOTTypeDirectObject (TClass *cls)
			: _cls (cls)
		{}

		string GetArgType() const { return string(_cls->GetName()); }
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func) {}

		// We own the memory we come back with.
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			void *r = (void*) func->ExecInt(ptr);
			if (r == nullptr)
				return false;

			// Assume it is a TObject (we should be safe here)
			auto obj = reinterpret_cast<::TObject*>(r);
			auto rdnobj = ROOTObjectServices::GetBestObject<ROOTDOTNETBaseTObject^>(obj);
			rdnobj->SetNativePointerOwner(true); // We track this and delete it when it is done!
			result = rdnobj;
			return true;
		}
	private:
		::TClass *_cls;
	};

	class RTCVoidType : public ROOTTypeConverter
	{
	public:

		string GetArgType() const { return "void";}
		void SetArg(System::Object ^obj, Cint::G__CallFunc *func) {}

		bool Call (G__CallFunc *fun, void *ptr, System::Object^% result)
		{
			return false;
		}
	};

	//
	// Get a type converter for the given name.
	//
	ROOTTypeConverter *FindConverter (const string &tname)
	{
		//
		// Any typedefs we need to worry about?
		//

		auto resolvedName = DynamicHelpers::resolveTypedefs(tname);

		if (resolvedName == "const char*")
			return new RTCString();

		if (resolvedName == "int")
			return new RTCBasicType<int>();

		if (resolvedName == "short")
			return new RTCBasicType<short>();

		if (resolvedName == "double")
			return new RTCBasicType<double>();

		if (resolvedName == "float")
			return new RTCBasicType<float>();

		if (resolvedName == "void")
			return new RTCVoidType();

		auto cls_info = DynamicHelpers::ExtractROOTClassInfoPtr(resolvedName);
		if (cls_info != nullptr)
		{
			return new RTCROOTType(cls_info);
		}

		cls_info = TClass::GetClass(resolvedName.c_str());
		if (cls_info != nullptr)
		{
			return new RTCROOTTypeDirectObject(cls_info); // Ctor return...
		}

		return nullptr;
	}

	//
	// Return a type converter for this argument
	//
	ROOTTypeConverter *FindConverter (TMethodArg *arg)
	{
		return FindConverter(arg->GetFullTypeName());
	}
}

namespace ROOTNET
{
	namespace Utility
	{
		DynamicHelpers::DynamicHelpers(void)
		{
		}

		//
		// Given the list of arguments, generate a prototype string
		// that CINT can understand for argument lookup.
		//
		string DynamicHelpers::GeneratePrototype(array<System::Object^> ^args)
		{
			string result = "";

			for (int index = 0; index < args->Length; index++)
			{
				auto arg = args[index];
				string thisType = "";
				auto gt = arg->GetType();
				if (gt == int::typeid)
				{
					thisType = "int";
				} else if (gt == long::typeid)
				{
					thisType = "long";
				} else if (gt == float::typeid)
				{
					thisType = "float";
				} else if (gt == double::typeid)
				{
					thisType = "double";
				} else if (gt == System::String::typeid)
				{
					thisType = "const char*";
				} else {
					// See if this is a class ptr that is part of the ROOT system.
					if (gt->IsSubclassOf(ROOTNET::Utility::ROOTDOTNETBaseTObject::typeid))
					{
						auto robj = static_cast<ROOTNET::Utility::ROOTDOTNETBaseTObject^>(arg);
						::TObject *tobj = robj->GetTObjectPointer();
						string rootname = string(tobj->IsA()->GetName());
						thisType = rootname + "*";
					} else {
						return "<>"; // Can't do it!
					}
				}

				if (result.size() == 0) {
					result = thisType;
				} else {
					result += "," + thisType;
				}
			}

			return result;
		}

		///
		/// Look for a root pointer
		///
		::TClass *DynamicHelpers::ExtractROOTClassInfoPtr (const string &tname)
		{
			int ptr = tname.rfind("*");
			if (ptr == tname.npos)
				return nullptr;

			auto nameonly = tname.substr(0, ptr);
			return ::TClass::GetClass(nameonly.c_str());
		}

		//
		// Make sure all type-defs are taken care of.
		//
		string DynamicHelpers::resolveTypedefs(const std::string &type)
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

		///
		/// We will put together a caller for this list of arguments.
		///
		DynamicCaller *DynamicHelpers::GetFunctionCaller(::TClass *cls_info, const std::string &method_name, array<System::Object^> ^args)
		{
			//
			// See if we can get the method that we will be calling for this function.
			//

			auto proto = DynamicHelpers::GeneratePrototype(args);
			auto method = cls_info->GetMethodWithPrototype(method_name.c_str(), proto.c_str());
			if (method == nullptr)
				return nullptr;

			//
			// Now, we can get a list of converters for the input types.
			//

			auto arg_list = method->GetListOfMethodArgs();
			TIter next(arg_list);
			::TObject *obj;
			vector<ROOTTypeConverter*> converters;
			while((obj = next()) != nullptr)
			{
				converters.push_back(FindConverter(static_cast<TMethodArg*>(obj)));
			}

			//
			// And the output type
			//

			ROOTTypeConverter *rtn_converter = FindConverter(method->GetReturnTypeName());

			//
			// ANd create the guy for that will do the actual work with the above information. Make sure it is
			// valid. If not, trash it, and return null.
			//

			auto caller = new DynamicCaller(converters, rtn_converter, method);
			if (caller->IsValid()) {
				return caller;
			}
			delete caller;
			return nullptr;
		}

		///
		/// Create a holder that will keep converters, etc., for a function.
		///
		DynamicCaller::DynamicCaller (std::vector<ROOTTypeConverter*> &converters, ROOTTypeConverter* rtn_converter, ::TMethod *method)
			: _arg_converters(converters), _rtn_converter(rtn_converter), _method(method), _methodCall (nullptr)
		{
		}

		///
		/// Clean up everything.
		///
		DynamicCaller::~DynamicCaller(void)
		{
			delete _rtn_converter;
			for (int i = 0; i < _arg_converters.size(); i++) {
				delete _arg_converters[i];
			}

			delete _methodCall;
		}

		//
		// Make sure everything is cool to go!
		//
		bool DynamicCaller::IsValid() const
		{
			if (_rtn_converter == nullptr || _method == nullptr)
				return false;

			for each (ROOTTypeConverter*o in _arg_converters)
			{
				if (o == nullptr)
					return false;
			}

			return true;
		}

		///
		/// Do the call
		///  Code is basically copied from ConstructorHolder in pyroot.
		///
		System::Object^ DynamicCaller::CallCtor(::TClass *clsInfo, array<System::Object^> ^args)
		{
			//
			// Allocate space for this object, and set everything up.
			//

			System::Object ^result = nullptr;
			Call(clsInfo, args, result);

			//
			// If that fails, then see if we can do something else.
			//

			return result;
		}

		//
		// Do a call for a compiled in class.
		//
		bool DynamicCaller::Call(::TObject *ptr, array<System::Object^> ^args, System::Object^% result)
		{
			///
			/// Get the calling function
			///
			if (_methodCall == nullptr)
			{
				G__ClassInfo *gcl = static_cast<G__ClassInfo*>(_method->GetClass()->GetClassInfo());
				if (gcl == nullptr)
					return false;

				auto gmi = gcl->GetMethod(_method->GetName(), ArgList().c_str(), &_offset, G__ClassInfo::ExactMatch);
				if (!gmi.IsValid())
					return false;

				_methodCall = new G__CallFunc();
				_methodCall->Init();
				_methodCall->SetFunc(gmi);
			}

			//
			// Setup the arguments
			//

			_methodCall->ResetArg();
			for (int i_arg = 0; i_arg < args->Length; i_arg++)
			{
				_arg_converters[i_arg]->SetArg(args[i_arg], _methodCall);
			}

			//
			// Now execute the thing.
			//

			return _rtn_converter->Call (_methodCall, (void*)((long)ptr + _offset), result);
		}

		//
		// Get a prototype list for everyone...
		//

		string DynamicCaller::ArgList() const
		{
			ostringstream args;
			bool isFirst = true;
			for each (ROOTTypeConverter *c in _arg_converters)
			{
				if (!isFirst)
					args << ",";
				isFirst = false;

				args << c->GetArgType();
			}

			return args.str();
		}
	}
}
