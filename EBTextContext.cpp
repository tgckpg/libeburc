#include "pch.h"
#include "EBTextContext.h"

using namespace libeburc;

EBTextContext::EBTextContext() { }

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