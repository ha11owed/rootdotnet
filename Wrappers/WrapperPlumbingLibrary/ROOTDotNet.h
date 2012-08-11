// ROOTDotNet.h
// Some services for use by everyone else. Very low level.
//
#pragma once

#include <string>

class TObject;
#pragma make_public(TObject)

class TClass;
#pragma make_public(TClass)

namespace ROOTNET
{
	namespace Utility
	{
		ref class ROOTDOTNETBaseTObject;

		public ref class ROOTObjectServices
		{
		public:
			static void DoIt () {}
			// Return the best wrapper we can find for an object that has TObject in its inherritance list.
			// TODO: Make sure it returns an old wrapper if we've already got one!
			generic<class T> where T: ref class
				static T GetBestObject (const ::TObject *obj);

			// Return the best wrapper we can find for an object that doesn't have TObject in its inherritance list.
			// TODO: Make sure it returns an old wrapper if we've already got one!
			static ROOTDOTNETBaseTObject ^ GetBestNonTObjectObject (const void *obj, ::TClass *class_info);

		};
	}
}