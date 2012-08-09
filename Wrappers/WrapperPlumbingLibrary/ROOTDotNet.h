// ROOTDotNet.h

#pragma once

#include <string>

class TObject;
#pragma make_public(TObject)

class TClass;

namespace ROOTNET
{
	namespace Utility
	{
		ref class ROOTDOTNETBaseTObject;

		public ref class ROOTObjectServices
		{
		public:
			static void DoIt () {}
			generic<class T> where T: ref class
				static T GetBestObject (const ::TObject *obj);

			static ROOTDOTNETBaseTObject ^ GetBestNonTObjectObject (const void *obj, ::TClass *class_info);

		};
	}
}