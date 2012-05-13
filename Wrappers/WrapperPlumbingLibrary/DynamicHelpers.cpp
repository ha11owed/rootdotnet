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
#include <algorithm>
#include <sstream>

#pragma make_public(TObject)

using std::string;
using std::vector;
using std::ostringstream;
using std::max;

#ifdef nullptr
#undef nullptr
#endif

namespace {
	using namespace ROOTNET::Utility;

	//
	// Deal with CINT when we have a string argument or return.
	//
	class RTCString : public ROOTTypeConverter
	{
	public:
		RTCString(bool isConst)
			: _cache (nullptr), _isConst (isConst)
		{}
		~RTCString()
		{
			delete[] _cache;
		}

		string GetArgType() const
		{
			if (_isConst)
				return "const char*";
			return "char*";
		}

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
			auto c = reinterpret_cast<char*>(func->ExecInt(ptr));
			result = gcnew System::String(c);
			return true;
		}
	private:
		// We need to make sure the string stays around...
		char *_cache;
		bool _isConst;
	};


	///// Type Traits to deal with the actual calling for simple types.

	template<typename T>
	struct RTCBasicTypeCallerTraits
	{
		static T Call (G__CallFunc *func, void *ptr);
	};

	template<>
	struct RTCBasicTypeCallerTraits<long>
	{
		static long Call (G__CallFunc *func, void *ptr)
		{
			return func->ExecInt(ptr);
		}
	};
	template<>
	struct RTCBasicTypeCallerTraits<unsigned long>
	{
		static unsigned long Call (G__CallFunc *func, void *ptr)
		{
			return (unsigned long) func->ExecInt(ptr);
		}
	};
	template<>
	struct RTCBasicTypeCallerTraits<double>
	{
		static double Call (G__CallFunc *func, void *ptr)
		{
			return func->ExecDouble(ptr);
		}
	};

	//////

	template<typename T, typename D>
	class RTCBasicType : public ROOTTypeConverter
	{
	public:
		RTCBasicType () {}
		string GetArgType() const { return typeid(T).name(); }
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func)
		{
			D r = (D) obj;
			func->SetArg(r);
		}
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			D r = RTCBasicTypeCallerTraits<D>::Call(func, ptr);
			result = (T) r;
			return true;
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

		//
		// Do the object pointer.
		// 
		// Since we handle only TObject's here, we don't have to worry about multiple inherritance,
		// and offests for various pointers. Everything has to come from a TObject, and we start from there.
		//
		void SetArg (System::Object ^obj, Cint::G__CallFunc *func)
		{

			auto rdnobj = (ROOTDOTNETBaseTObject^) obj;
			auto robj = rdnobj->GetTObjectPointer();
			void *ptr = static_cast<void*>(robj);

			func->SetArg(reinterpret_cast<long>(ptr));
		}

		//
		// Go get an object. A null pointer is ok to come back. But we can't
		// do much about its type in this dynamic area.
		// Ownership is not ours - the ROOT system (or someone else) is responsible.
		bool Call (G__CallFunc *func, void *ptr, System::Object^% result)
		{
			void *r = (void*) func->ExecInt(ptr);
			if (r == nullptr)
			{
				return true;
			}

			auto obj = reinterpret_cast<::TObject*>(r);
			auto rdnobj = ROOTObjectServices::GetBestObject<ROOTDOTNETBaseTObject^>(obj);
			result = rdnobj;
			return true;
		}
	private:
		::TClass *_cls;
	};

	// Direct object, no pointer. we own the thing now.
	class RTCROOTTypeCtorObject : public ROOTTypeConverter
	{
	public:
		inline RTCROOTTypeCtorObject (TClass *cls)
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
			fun->Exec(ptr);
			result = nullptr;
			return true;
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
			return new RTCString(true);
		if (resolvedName == "char*")
			return new RTCString(false);

		if (resolvedName == "short")
			return new RTCBasicType<short, long>();
		if (resolvedName == "int")
			return new RTCBasicType<int, long>();
		if (resolvedName == "long")
			return new RTCBasicType<long, long>();

		if (resolvedName == "unsigned short")
			return new RTCBasicType<unsigned short, long>();
		if (resolvedName == "unsigned int")
			return new RTCBasicType<unsigned int, long>();
		if (resolvedName == "unsigned long")
			return new RTCBasicType<unsigned long, long>();

		if (resolvedName == "double")
			return new RTCBasicType<double, double>();

		if (resolvedName == "float")
			return new RTCBasicType<float, double>();

		if (resolvedName == "void")
			return new RTCVoidType();

		auto cls_info = DynamicHelpers::ExtractROOTClassInfoPtr(resolvedName);
		if (cls_info != nullptr)
		{
			return new RTCROOTType(cls_info);
		}

		return nullptr;
	}

	///
	/// CTor returns require some special handling.
	///
	ROOTTypeConverter *FindConverterROOTCtor (const string &tname)
	{
		auto resolvedName = DynamicHelpers::resolveTypedefs(tname);

		auto cls_info = TClass::GetClass(resolvedName.c_str());
		if (cls_info != nullptr)
		{
			return new RTCROOTTypeCtorObject(cls_info); // Ctor return...
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

	//
	// Parse a type - removing the modifiers, etc. If the modifiers are already set, keep
	// pushing stuff on the front.
	//
	void parse_type (const string &type, string &current, string &modifiers)
	{
		int ptr_index = type.find("*");
		int ref_index = type.find("&");
		int index = max(ptr_index, ref_index);
		if (index == type.npos) {
			current = type;
			return;
		}

		current = type.substr(0, index);
		modifiers = type.substr(index) + modifiers;
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
				} else if (gt == short::typeid)
				{
					thisType = "short";
				} else if (gt == unsigned int::typeid)
				{
					thisType = "unsigned int";
				} else if (gt == unsigned long::typeid)
				{
					thisType = "unsigned long";
				} else if (gt == unsigned short::typeid)
				{
					thisType = "unsigned short";
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
		// Make sure all type-defs are taken care of. Deal with pointer info as well.
		//
		string DynamicHelpers::resolveTypedefs(const std::string &type)
		{
			string current, modifiers;
			parse_type (type, current, modifiers);
			while (true)
			{
				auto dtinfo = gROOT->GetType(current.c_str());
				if (dtinfo == nullptr)
					return current + modifiers;

				string newname = dtinfo->GetTypeName();
				if (newname == current)
					return current + modifiers;

				parse_type (newname, current, modifiers);
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
			// And the output type. We do something special for ctor's here as the resulting
			// output needs to be dealt with carefully.
			//

			ROOTTypeConverter *rtn_converter;
			if (method_name == cls_info->GetName()) {
				rtn_converter = FindConverterROOTCtor(method->GetReturnTypeName());
			} else {
				rtn_converter = FindConverter(method->GetReturnTypeName());
			}

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
					throw gcnew System::InvalidOperationException("ROOT Class we already know about we can't get the info for!");

				auto gmi = gcl->GetMethod(_method->GetName(), ArgList().c_str(), &_offset, G__ClassInfo::ExactMatch);
				if (!gmi.IsValid())
					throw gcnew System::InvalidOperationException("ROOT Method we already know about we can't get the method info for!");

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
