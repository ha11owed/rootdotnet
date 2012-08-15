//
// All the factories for all the leaf types (well, most) that we can
// recognize by simple pattern recognition.
//
#include "TreeLeafExecutorFactories.h"
#include "TreeLeafExecutor.hpp"
#include "TreeManager.hpp"

#include "TreeSimpleValueExecutor.h"
#include "TreeVectorValueExecutor.h"

#include "TBranch.h"
namespace {
	using namespace ROOTNET::Utility;
	/// Lots of factory objects. It would have been awsome to use lambda's here, however the
	/// ::TBranch means we can't use templates for lambda funcions! These guys also have the function
	/// of linking the generic and C++ template world together.

#define MAKE_TLEF(t)	ref class TLEF_##t : TreeLeafExecutorFactory \
	{ \
	public: \
		virtual TreeLeafExecutor ^Generate (::TBranch *branch) override \
		{ \
				return gcnew tle_simple_type<t> (new tle_simple_type_accessor<t> (branch)); \
		} \
	};
#define MAKE_UTLEF(t)	ref class TLEF_u##t : TreeLeafExecutorFactory \
	{ \
	public: \
		virtual TreeLeafExecutor ^Generate (::TBranch *branch) override \
		{ \
				return gcnew tle_simple_type<unsigned t> (new tle_simple_type_accessor<unsigned t> (branch)); \
		} \
	};
#define MAKE_TLEVF(t)	ref class TLEVF_##t : TreeLeafExecutorFactory \
	{ \
	public: \
		virtual TreeLeafExecutor ^Generate (::TBranch *branch) override \
		{ \
				return gcnew tle_vector_type (new tle_vector_type_exe<t>(branch)); \
		} \
	};
#define MAKE_UTLEVF(t)	ref class TLEVF_u##t : TreeLeafExecutorFactory \
	{ \
	public: \
		virtual TreeLeafExecutor ^Generate (::TBranch *branch) override \
		{ \
				return gcnew tle_vector_type (new tle_vector_type_exe<unsigned t>(branch)); \
		} \
	};



#define MAKE_TLEF_REF(r,t) r[#t] = gcnew TLEF_##t()
#define MAKE_UTLEF_REF(r,t) r["unsigned " #t] = gcnew TLEF_u##t()
#define MAKE_TLEVF_REF(r,t) r["vector<" #t ">"] = gcnew TLEVF_##t()
#define MAKE_UTLEVF_REF(r,t) r["vector<unsigned " #t ">"] = gcnew TLEVF_u##t()

	MAKE_TLEF(int);
	MAKE_TLEF(short);
	MAKE_TLEF(long);

	MAKE_TLEVF(int);
	MAKE_TLEVF(short);
	MAKE_TLEVF(long);

	MAKE_UTLEF(int);
	MAKE_UTLEF(short);
	MAKE_UTLEF(long);

	MAKE_UTLEVF(int);
	MAKE_UTLEVF(short);
	MAKE_UTLEVF(long);

	MAKE_TLEF(float);
	MAKE_TLEF(double);

	MAKE_TLEVF(float);
	MAKE_TLEVF(double);

	ref class TLEVF_string : TreeLeafExecutorFactory
	{
	public:
		virtual TreeLeafExecutor ^Generate (::TBranch *branch) override
		{
				return gcnew tle_vector_type (new tle_vector_string_type_exe(branch));
		}
	};
}

namespace ROOTNET {
	namespace Utility {
		System::Collections::Generic::Dictionary<System::String^, TreeLeafExecutorFactory ^> ^CreateKnownLeafFactories()
		{
			auto result = gcnew System::Collections::Generic::Dictionary<System::String^, TreeLeafExecutorFactory ^> ();
			MAKE_TLEF_REF(result, int);
			MAKE_TLEF_REF(result, short);
			MAKE_TLEF_REF(result, long);

			MAKE_TLEVF_REF(result, int);
			MAKE_TLEVF_REF(result, short);
			MAKE_TLEVF_REF(result, long);

			MAKE_UTLEF_REF(result, int);
			MAKE_UTLEF_REF(result, short);
			MAKE_UTLEF_REF(result, long);

			MAKE_UTLEVF_REF(result, int);
			MAKE_UTLEVF_REF(result, short);
			MAKE_UTLEVF_REF(result, long);

			MAKE_TLEF_REF(result, float);
			MAKE_TLEF_REF(result, double);

			MAKE_TLEVF_REF(result, float);
			MAKE_TLEVF_REF(result, double);

			result["vector<string>"] = gcnew TLEVF_string();
			return result;
		}
	}
}
