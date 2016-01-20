#pragma once

#include <defs.h>
#include <EBPosition.h>
#include <build-post.h>

namespace libeburc
{
	ref class EBSearchContext sealed
	{
		/*
		 * Current search method type.
		 * The context is not active, if this code is EB_SEARCH_NONE.
		 */
		EBSearchCode code;

		/*
		 * Function which compares word to search and pattern in an index page.
		 */
		int( *compare_pre )( const char *word, const char *pattern,
			size_t length );
		int( *compare_single )( const char *word, const char *pattern,
			size_t length );
		int( *compare_group )( const char *word, const char *pattern,
			size_t length );

		/*
		 * Result of comparison by `compare'.
		 */
		int comparison_result;

		/*
		 * Word to search.
		 */
		char word[ EB_MAX_WORD_LENGTH + 1 ];

		/*
		 * Canonicalized word to search.
		 */
		char canonicalized_word[ EB_MAX_WORD_LENGTH + 1 ];

		/*
		 * Page which is searched currently.
		 */
		int page;

		/*
		 * Offset which is searched currently in the page.
		 */
		int offset;

		/*
		 * Page ID of the current page.
		 */
		int page_id;

		/*
		 * How many entries in the current page.
		 */
		int entry_count;

		/*
		 * Entry index pointer.
		 */
		int entry_index;

		/*
		 * Length of the current entry.
		 */
		int entry_length;

		/*
		 * Arrangement style of entries in the current page (fixed or variable).
		 */
		int entry_arrangement;

		/*
		 * In a group entry or not.
		 */
		int in_group_entry;

		/*
		 * Current heading position (for keyword search).
		 */
		EBPosition^ keyword_heading;
	public:
		EBSearchContext();
	};
}
