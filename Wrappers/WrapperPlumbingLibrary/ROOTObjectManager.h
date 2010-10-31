#pragma once

///
/// Code to manage the connection between base C++ objects and .NET objects.

using namespace System::Collections::Generic;

#include "ROOTObjectInfo.h"

/// The base ROOT object...
class TObject;
#pragma make_public(TObject)

namespace ROOTNET {
	namespace Utility {

		/// A few forward declares...
		ref class ROOTDOTNETBaseTObject;
		class ROOTCleanupCallback;

		///
		/// Keep track of the connectionb between the ROOT TObjet and any .NET front-class we create.
		///
		/// NOTE: If the main table ever gets too large that it affects performance, we could split
		/// things by object type.
		///
		public ref class ROOTObjectManager
		{
		public:
			/// This is a singleton...
			static ROOTObjectManager ^instance(void);

			/// Find an object in the table. null if not there now
			ROOTObjectInfo ^FindROOTObject (const ::TObject *obj);

			/// Similar, but do it with an int. :-)
			ROOTObjectInfo ^FindROOTObject (int obj_ptr);

			/// Save a TObject mapping in our internal table. Do it only for TObject's for now.
			/// we are owner is set to true if we shoudl dtor this object. If set to false then
			/// we don't dtor it.
			void RegisterObject (::TObject *obj,
				ROOTDOTNETBaseTObject ^root_obj);

			/// Do we know about a particular object?
			bool KnowAboutROOTObject (const ::TObject *obj);

			/// Cleanup references to this particular object -- this is when the ::TObject is being deleted and
			/// we need to deal with it.
			void ROOTObjectRemoval (::TObject *obj);

			/// Drop an object from our table, but don't do anything to it in the process
			void ForgetAboutObject (::TObject *obj);

			/// If we are goign to be banished, and shut down, disconnect ourselves from root!
			void CleanupManager(void);

			/// When this goes away, make sure we interact with the ROOT subsystem
			/// correctly.
			~ROOTObjectManager (void);
			!ROOTObjectManager (void);

			/// How many objects are we tracking?
			int ObjectCount (void) {return _root_mapping->Count;}

		private:
			Dictionary<int, ROOTObjectInfo^> ^_root_mapping;
			System::Threading::ReaderWriterLock ^_reader_writer_root_map_lock;
			static ROOTObjectManager ^_instance = nullptr;
			ROOTCleanupCallback *_callback;

		protected:
			/// Singleton. Keep others out! :-)
			ROOTObjectManager(void);


		};
	}
}