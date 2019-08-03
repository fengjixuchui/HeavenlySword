/***************************************************************************************************
*
*	DESCRIPTION		Handy functions for list manipulation
*
*	NOTES
*
***************************************************************************************************/

#ifndef _LIST_TOOLS_H
#define _LIST_TOOLS_H

/***************************************************************************************************
*
*	FUNCTION		Bubble_Sort
*
*	DESCRIPTION		Function template for sorting shennanigans.
*
***************************************************************************************************/
template<typename BidirectionalIterator, typename Compare>
void	bubble_sort( BidirectionalIterator first, BidirectionalIterator last, Compare comp )
{
	BidirectionalIterator left, right;
	while( first != last )
	{
		left = right = first;
		++right;
		while( right != last ) 
		{
			if( comp( *right, *left ) ) 
				ntstd::iter_swap( left, right );
			++right;
			++left;
		}
		--last;
	}
}

#endif // _LIST_TOOLS_H
