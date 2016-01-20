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