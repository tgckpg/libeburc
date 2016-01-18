#pragma once
#include <pch.h>
#include <Utils.h>

#define LOG(x);

typedef int EBBookCode;
typedef int EBCaseCode;
typedef int EBSuffixCode;
typedef int EBFontCode;
typedef int EBWordCode;
typedef int EBSubbookCode;
typedef int EBIndexStyleCode;
typedef int EBSearchCode;
typedef int EBTextCode;
typedef int EBTextStatusCode;
typedef int EBMultiSearchCode;
typedef int EBHookCode;
typedef int EBBinaryCode;

/*
 * Special book ID for cache to represent "no cache data for any book".
 */
#define EB_BOOK_NONE            -1
/*
 * Maximum length of a word to be searched.
 */
#define EB_MAX_WORD_LENGTH             255

/*
 * Maximum length of an EB* book title.
 */
#define EB_MAX_EB_TITLE_LENGTH		30

/*
 * Maximum length of an EPWING book title.
 */
#define EB_MAX_EPWING_TITLE_LENGTH	80

/*
 * Maximum length of a book title.
 */
#define EB_MAX_TITLE_LENGTH		80

/*
 * Maximum length of a word to be searched.
 */
#if defined(PATH_MAX)
#define EB_MAX_PATH_LENGTH		PATH_MAX
#elif defined(MAXPATHLEN)
#define EB_MAX_PATH_LENGTH		MAXPATHLEN
#else
#define EB_MAX_PATH_LENGTH		1024
#endif

/*
 * Maximum length of a directory name.
 */
#define EB_MAX_DIRECTORY_NAME_LENGTH	8

/*
 * Maximum length of a file name under a certain directory.
 * prefix(8 chars) + '.' + suffix(3 chars) + ';' + digit(1 char)
 */
#define EB_MAX_FILE_NAME_LENGTH		14

/*
 * Maximum length of a label for multi-search entry.
 */
#define EB_MAX_MULTI_LABEL_LENGTH	30

/*
 * Maximum length of alternation text string for a private character.
 */
#define EB_MAX_ALTERNATION_TEXT_LENGTH	31

/*
 * Maximum length of title for multi search.
 */
#define EB_MAX_MULTI_TITLE_LENGTH	32

/*
 * Maximum number of font heights in a subbok.
 */
#define EB_MAX_FONTS			4

/*
 * Maximum number of subbooks in a book.
 */
#define EB_MAX_SUBBOOKS			50

/*
 * Maximum number of multi-search types in a subbook.
 */
#define EB_MAX_MULTI_SEARCHES		10

/*
 * Maximum number of entries in a multi-search.
 */
#define EB_MAX_MULTI_ENTRIES		5

/*
 * Maximum number of entries in a keyword search.
 */
#define EB_MAX_KEYWORDS			EB_MAX_MULTI_ENTRIES

/*
 * Maximum number of entries in a cross search.
 */
#define EB_MAX_CROSS_ENTRIES		EB_MAX_MULTI_ENTRIES

/*
 * Maximum number of characters for alternation cache.
 */
#define EB_MAX_ALTERNATION_CACHE	16

/*
 * The number of text hooks.
 */
#define EB_NUMBER_OF_HOOKS		55

/*
 * The number of search contexts required by a book.
 */
#define EB_NUMBER_OF_SEARCH_CONTEXTS	EB_MAX_MULTI_ENTRIES

namespace libeburc
{
	public enum class EBDiscCode
	{
		EB_DISC_INVALID,
		EB_DISC_EB,
		EB_DISC_EPWING,
	};

	public enum class EBCharacterCode
	{
		EB_CHARCODE_INVALID = -1,
		EB_CHARCODE_ISO8859_1 = 1,
		EB_CHARCODE_JISX0208 = 2,
		EB_CHARCODE_JISX0208_GB2312 = 3,
		EB_CHARCODE_UTF8 = 4,
	};

	public enum class EBErrorCode
	{
		/* 0 -- 4 */
		EB_SUCCESS,
		EB_ERR_MEMORY_EXHAUSTED,
		EB_ERR_EMPTY_FILE_NAME,
		EB_ERR_TOO_LONG_FILE_NAME,
		EB_ERR_BAD_FILE_NAME,

		/* 5 -- 9 */
		EB_ERR_BAD_DIR_NAME,
		EB_ERR_TOO_LONG_WORD,
		EB_ERR_BAD_WORD,
		EB_ERR_EMPTY_WORD,
		EB_ERR_FAIL_GETCWD,

		/* 10 -- 14 */
		EB_ERR_FAIL_OPEN_CAT,
		EB_ERR_FAIL_OPEN_CATAPP,
		EB_ERR_FAIL_OPEN_TEXT,
		EB_ERR_FAIL_OPEN_FONT,
		EB_ERR_FAIL_OPEN_APP,

		/* 15 -- 19 */
		EB_ERR_FAIL_OPEN_BINARY,
		EB_ERR_FAIL_READ_CAT,
		EB_ERR_FAIL_READ_CATAPP,
		EB_ERR_FAIL_READ_TEXT,
		EB_ERR_FAIL_READ_FONT,

		/* 20 -- 24 */
		EB_ERR_FAIL_READ_APP,
		EB_ERR_FAIL_READ_BINARY,
		EB_ERR_FAIL_SEEK_CAT,
		EB_ERR_FAIL_SEEK_CATAPP,
		EB_ERR_FAIL_SEEK_TEXT,

		/* 25 -- 29 */
		EB_ERR_FAIL_SEEK_FONT,
		EB_ERR_FAIL_SEEK_APP,
		EB_ERR_FAIL_SEEK_BINARY,
		EB_ERR_UNEXP_CAT,
		EB_ERR_UNEXP_CATAPP,

		/* 30 -- 34 */
		EB_ERR_UNEXP_TEXT,
		EB_ERR_UNEXP_FONT,
		EB_ERR_UNEXP_APP,
		EB_ERR_UNEXP_BINARY,
		EB_ERR_UNBOUND_BOOK,

		/* 35 -- 39 */
		EB_ERR_UNBOUND_APP,
		EB_ERR_NO_SUB,
		EB_ERR_NO_APPSUB,
		EB_ERR_NO_FONT,
		EB_ERR_NO_TEXT,

		/* 40 -- 44 */
		EB_ERR_NO_STOPCODE,
		EB_ERR_NO_ALT,
		EB_ERR_NO_CUR_SUB,
		EB_ERR_NO_CUR_APPSUB,
		EB_ERR_NO_CUR_FONT,

		/* 45 -- 49 */
		EB_ERR_NO_CUR_BINARY,
		EB_ERR_NO_SUCH_SUB,
		EB_ERR_NO_SUCH_APPSUB,
		EB_ERR_NO_SUCH_FONT,
		EB_ERR_NO_SUCH_CHAR_BMP,

		/* 50 -- 54 */
		EB_ERR_NO_SUCH_CHAR_TEXT,
		EB_ERR_NO_SUCH_SEARCH,
		EB_ERR_NO_SUCH_HOOK,
		EB_ERR_NO_SUCH_BINARY,
		EB_ERR_DIFF_CONTENT,

		/* 55 -- 59 */
		EB_ERR_NO_PREV_SEARCH,
		EB_ERR_NO_SUCH_MULTI_ID,
		EB_ERR_NO_SUCH_ENTRY_ID,
		EB_ERR_TOO_MANY_WORDS,
		EB_ERR_NO_WORD,

		/* 60 -- 64 */
		EB_ERR_NO_CANDIDATES,
		EB_ERR_END_OF_CONTENT,
		EB_ERR_NO_PREV_SEEK,
		EB_ERR_EBNET_UNSUPPORTED,
		EB_ERR_EBNET_FAIL_CONNECT,

		/* 65 -- 69 */
		EB_ERR_EBNET_SERVER_BUSY,
		EB_ERR_EBNET_NO_PERMISSION,
		EB_ERR_UNBOUND_BOOKLIST,
		EB_ERR_NO_SUCH_BOOK,
	};

}