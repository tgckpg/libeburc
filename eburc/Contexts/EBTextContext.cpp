#include "pch.h"
#include "eburc/Contexts/EBTextContext.h"

using namespace libeburc;

EBTextContext::EBTextContext() { }

EBTextContext::EBTextContext( EBTextContext^ Context )
{
	code = Context->code;
	out = Context->out;
	out_rest_length = Context->out_rest_length;
	unprocessed = Context->unprocessed;
	unprocessed_size = Context->unprocessed_size;
	out_step = Context->out_step;
	narrow_flag = Context->narrow_flag;
	printable_count = Context->printable_count;
	file_end_flag = Context->file_end_flag;
	text_status = Context->text_status;
	skip_code = Context->skip_code;
	auto_stop_code = Context->auto_stop_code;

	size_t csize = strlen( Context->candidate );
	if( 0 < csize ) strcpy_s( candidate, csize + 1, Context->candidate );

	is_candidate = Context->is_candidate;
	ebxac_gaiji_flag = Context->ebxac_gaiji_flag;
}

void EBTextContext::Reset()
{
	out = NULL;
    out_rest_length = 0;
    unprocessed = NULL;
    unprocessed_size = 0;
    out_step = 0;
    narrow_flag = 0;
    printable_count = 0;
    file_end_flag = 0;
    text_status = EBTextStatusCode::EB_TEXT_STATUS_CONTINUED;
    skip_code = SKIP_CODE_NONE;
    auto_stop_code = -1;
    candidate[0] = '\0';
    is_candidate = 0;
    ebxac_gaiji_flag = 0;
}