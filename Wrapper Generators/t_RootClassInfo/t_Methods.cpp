#include "stdafx.h"

#include "RootClassInfo.hpp"
#include "RootClassInfoCollection.hpp"
#include "RootClassMethod.hpp"
#include "CPPNetTypeMapper.hpp"
#include "TTSimpleType.hpp"

#include "TApplication.h"
#include "TSystem.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace	Microsoft::VisualStudio::TestTools::UnitTesting;

using std::string;
using std::vector;

namespace t_RootClassInfo
{
	[TestClass]
	public ref class t_Methods
	{
	public: 
		[ClassInitialize]
		static void SetupClassTest(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ testContext)
		{
			int nargs = 2;
			char *argv[2];
			argv[0] = "ROOTWrapperGenerator.exe";
			argv[1] = "-b";
			TApplication *app = new TApplication ("ROOTWrapperGenerator", &nargs, argv);
			gSystem->Load("libHist");
			gSystem->Load("libHist");
			gSystem->Load("libRooFit");
		}

		/// Make sure the type translators are empty!
		[TestCleanup]
		[TestInitialize]
		void CleanOutTypeSystem()
		{
			CPPNetTypeMapper::Reset();
			RootClassInfoCollection::Reset();
		}

		/// Find a method
		const RootClassMethod *FindMethod(const string &mname, const vector<RootClassMethod> &protos)
		{
			for (int i = 0; i < protos.size(); i++) {
				if (protos[i].CPPName() == mname) {
					return &protos[i];
				}
			}
			return 0;
		}

		[TestMethod]
		void TestMethodIsVirtual()
		{
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("int", "int"));
			CPPNetTypeMapper::instance()->AddTypeMapper(new TTSimpleType("double", "double"));
			CPPNetTypeMapper::instance()->AddTypedefMapping("Int_t", "int");
			CPPNetTypeMapper::instance()->AddTypedefMapping("Double_t", "double");

			RootClassInfo *cinfo = new RootClassInfo ("TArray");
			Assert::IsFalse (cinfo == nullptr, "We should have this class");
			auto method = FindMethod("GetAt", cinfo->GetAllPrototypesForThisClass(true));
			Assert::IsFalse(method == nullptr, "We shoudl have a method");
			Assert::IsTrue(method->IsVirtual(), "Method should be marked virtual");
		}
	};
}
