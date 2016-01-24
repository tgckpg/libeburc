#include "pch.h"
#include "EBException.h"

using namespace libeburc;

static const wchar_t * const error_messages[] = {
	/* 0 -- 4 */
	L"no error",
	L"memory exhausted",
	L"an empty file name",
	L"too long file name",
	L"bad file name",

	/* 5 -- 9 */
	L"bad directory name",
	L"too long word",
	L"a word contains bad character",
	L"an empty word",
	L"failed to get the current working directory",

	/* 10 -- 14 */
	L"failed to open a catalog file",
	L"failed to open an appendix catalog file",
	L"failed to open a text file",
	L"failed to open a font file",
	L"failed to open an appendix file",

	/* 15 -- 19 */
	L"failed to open a binary file",
	L"failed to read a catalog file",
	L"failed to read an appendix catalog file",
	L"failed to read a text file",
	L"failed to read a font file",

	/* 20 -- 24 */
	L"failed to read an appendix file",
	L"failed to read a binary file",
	L"failed to seek a catalog file",
	L"failed to seek an appendix catalog file",
	L"failed to seek a text file",

	/* 25 -- 29 */
	L"failed to seek a font file",
	L"failed to seek an appendix file",
	L"failed to seek a binary file",
	L"unexpected format in a catalog file",
	L"unexpected format in an appendix catalog file",

	/* 30 -- 34 */
	L"unexpected format in a text file",
	L"unexpected format in a font file",
	L"unexpected format in an appendix file",
	L"unexpected format in a binary file",
	L"book not bound",

	/* 35 -- 39 */
	L"appendix not bound",
	L"no subbook",
	L"no subbook in the appendix",
	L"no font",
	L"no text file",

	/* 40 -- 44 */
	L"no stop-code",
	L"no alternation string",
	L"no current subbook",
	L"no current appendix subbook",
	L"no current font",

	/* 45 -- 49 */
	L"no current binary",
	L"no such subbook",
	L"no such appendix subbook",
	L"no such font",
	L"no such character bitmap",

	/* 50 -- 54 */
	L"no such character text",
	L"no such search method",
	L"no such hook",
	L"no such binary",
	L"different content type",

	/* 55 -- 59 */
	L"no previous search",
	L"no such multi search",
	L"no such multi search entry",
	L"too many words specified",
	L"no word specified",

	/* 60 -- 64 */
	L"no candidates",
	L"end of content",
	L"no previous seek",
	L"ebnet is not supported",
	L"failed to connect to an ebnet server",

	/* 65 -- 69 */
	L"ebnet server is busy",
	L"no access permission",
	L"booklist not bound",
	L"no such book",

	/* 70 -- 74 */
	L"Failed to read ebz",

};


void EBException::Throw(EBErrorCode ErrorCode)
{
	int Code = (int)ErrorCode;
	throw ref new Platform::COMException(-Code, ref new Platform::String(error_messages[Code]));
}
