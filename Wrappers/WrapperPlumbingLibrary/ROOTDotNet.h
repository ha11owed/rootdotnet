// ROOTDotNet.h

#pragma once

class TObject;
#pragma make_public(TObject)

namespace ROOTNET
{
	namespace Utility
	{
		public ref class ROOTObjectServices
		{
		public:
			static void DoIt () {}
			generic<class T> where T: ref class
				static T GetBestObject (const ::TObject *obj);

		};
	}
}