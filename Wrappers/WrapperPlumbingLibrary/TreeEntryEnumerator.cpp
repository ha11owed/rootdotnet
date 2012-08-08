#include "TreeEntryEnumerator.hpp"

class TTree;
#pragma make_public(TTree)


namespace ROOTNET
{
	namespace Utility
	{
			TreeEntryEnumerator::TreeEntryEnumerator(::TTree *treePtr)
				: _tree (treePtr), _current_entry (-1)
			{
				// We explicity read in each leaf as we need it.
				_tree->SetBranchStatus ("*", false);
			}

			bool TreeEntryEnumerator::MoveNext (void)
			{
				_current_entry++;
				return _current_entry < _tree->GetEntries();
			}
	}
}
