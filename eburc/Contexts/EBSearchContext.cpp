#include "pch.h"
#include "EBSearchContext.h"

using namespace libeburc;

EBSearchContext::EBSearchContext()
{
	code = EBSearchCode::EB_SEARCH_NONE;
	compare_pre = NULL;
	compare_single = NULL;
	compare_group = NULL;
	comparison_result = -1;
	word[0] = '\0';
	canonicalized_word[0] = '\0';
	page = 0;
	offset = 0;
	page_id = 0;
	entry_count = 0;
	entry_index = 0;
	entry_length = 0;
	entry_arrangement = EB_ARRANGE_INVALID;
	in_group_entry = 0;
	keyword_heading = ref new EBPosition();
}

EBSearchContext::EBSearchContext( EBSearchContext^ Context )
{
	code = Context->code;
	compare_pre = Context->compare_pre;
	compare_single = Context->compare_single;
	compare_group = Context->compare_group;
	comparison_result = Context->comparison_result;

	size_t csize = strlen( Context->word );
	if ( 0 < csize ) strcpy_s( word, csize + 1, Context->word );

	csize = strlen( Context->canonicalized_word );
	if ( 0 < csize ) strcpy_s( canonicalized_word, csize + 1, Context->canonicalized_word );

	page = Context->page;
	offset = Context->offset;
	page_id = Context->page_id;
	entry_count = Context->entry_count;
	entry_index = Context->entry_index;
	entry_length = Context->entry_length;
	entry_arrangement = Context->entry_arrangement;
	in_group_entry = Context->in_group_entry;
	keyword_heading = Context->keyword_heading;
}