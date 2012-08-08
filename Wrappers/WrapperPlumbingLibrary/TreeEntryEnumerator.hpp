///
/// Enumerator class that returns an tree iterator that moves through the tree.
///
#pragma once

#include "TreeEntry.hpp"

#include "TTree.h"

#ifdef nullptr
#undef nullptr
#endif

namespace ROOTNET
{
	namespace Utility
	{

		public ref class TreeEntryEnumerator : System::Collections::Generic::IEnumerator<ROOTNET::Utility::TreeEntry^>
		{
		public:
			TreeEntryEnumerator(::TTree *treePtr)
				: _tree (treePtr)
			{}

			inline ~TreeEntryEnumerator (void)
			{}

			bool MoveNext (void)
			{ return false; }

			virtual bool MoveNext2() sealed = System::Collections::IEnumerator::MoveNext
			{ return MoveNext(); }

			property ROOTNET::Utility::TreeEntry ^Current
			{
				virtual ROOTNET::Utility::TreeEntry ^get()
				{ return nullptr; }
			}

			property Object^ Current2
			{
				virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get
				{ return nullptr; }
			}

			void Reset()
			{}

			virtual void Reset2 () sealed = System::Collections::IEnumerator::Reset
			{ Reset(); }

		private:
			// Pointer to the tree we need to be looking at.
			::TTree *_tree;
		};

	}
}

