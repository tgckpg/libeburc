#pragma once
#include <pch.h>
#include <Utils.h>

#define CP_EUCJP 20932

#define LOG(x);

typedef int EBBookCode;
typedef int EBCaseCode;
typedef int EBSuffixCode;
typedef int EBFontCode;
typedef int EBSubbookCode;
typedef int EBMultiSearchCode;

/*
 * The number of text hooks.
 */
#define EB_NUMBER_OF_HOOKS		55

/*
 * Size of a page (The term `page' means `block' in JIS X 4081).
 */
#define EB_SIZE_PAGE			2048
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
 * The maximum index depth of search indexes.
 */
#define EB_MAX_INDEX_DEPTH		6

/*
 * Length of cache buffer in a binary context.
 * It must be greater than 38, size of GIF preamble.
 * It must be greater than 44, size of WAVE sound header.
 * It must be greater than 118, size of BMP header + info + 16 rgbquads.
 */
#define EB_SIZE_BINARY_CACHE_BUFFER	128

/*
 * The number of text hooks.
 */
#define EB_NUMBER_OF_HOOKS		55

/*
 * The number of search contexts required by a book.
 */
#define EB_NUMBER_OF_SEARCH_CONTEXTS	EB_MAX_MULTI_ENTRIES

#define SKIP_CODE_NONE  -1

namespace libeburc
{
	/*
	 * Search word types.
	 */
	public enum class EBWordCode
	{
		EB_WORD_ALPHABET = 0,
		EB_WORD_KANA = 1,
		EB_WORD_OTHER = 2,
		EB_WORD_INVALID = -1,
	};
	/*
	 * Hook codes.
	 * (When you add or remove a hook, update EB_NUMER_OF_HOOKS in defs.h.)
	 */
	public enum class EBHookCode
	{
		EB_HOOK_NULL = -1,
		EB_HOOK_INITIALIZE = 0,
		EB_HOOK_BEGIN_NARROW = 1,
		EB_HOOK_END_NARROW = 2,
		EB_HOOK_BEGIN_SUBSCRIPT = 3,
		EB_HOOK_END_SUBSCRIPT = 4,

		EB_HOOK_SET_INDENT = 5,
		EB_HOOK_NEWLINE = 6,
		EB_HOOK_BEGIN_SUPERSCRIPT = 7,
		EB_HOOK_END_SUPERSCRIPT = 8,
		EB_HOOK_BEGIN_NO_NEWLINE = 9,

		EB_HOOK_END_NO_NEWLINE = 10,
		EB_HOOK_BEGIN_EMPHASIS = 11,
		EB_HOOK_END_EMPHASIS = 12,
		EB_HOOK_BEGIN_CANDIDATE = 13,
		EB_HOOK_END_CANDIDATE_GROUP = 14,

		EB_HOOK_END_CANDIDATE_LEAF = 15,
		EB_HOOK_BEGIN_REFERENCE = 16,
		EB_HOOK_END_REFERENCE = 17,
		EB_HOOK_BEGIN_KEYWORD = 18,
		EB_HOOK_END_KEYWORD = 19,

		EB_HOOK_NARROW_FONT = 20,
		EB_HOOK_WIDE_FONT = 21,
		EB_HOOK_ISO8859_1 = 22,
		EB_HOOK_NARROW_JISX0208 = 23,
		EB_HOOK_WIDE_JISX0208 = 24,

		EB_HOOK_GB2312 = 25,
		EB_HOOK_BEGIN_MONO_GRAPHIC = 26,
		EB_HOOK_END_MONO_GRAPHIC = 27,
		EB_HOOK_BEGIN_GRAY_GRAPHIC = 28,
		EB_HOOK_END_GRAY_GRAPHIC = 29,

		EB_HOOK_BEGIN_COLOR_BMP = 30,
		EB_HOOK_BEGIN_COLOR_JPEG = 31,
		EB_HOOK_BEGIN_IN_COLOR_BMP = 32,
		EB_HOOK_BEGIN_IN_COLOR_JPEG = 33,
		EB_HOOK_END_COLOR_GRAPHIC = 34,

		EB_HOOK_END_IN_COLOR_GRAPHIC = 35,
		EB_HOOK_BEGIN_WAVE = 36,
		EB_HOOK_END_WAVE = 37,
		EB_HOOK_BEGIN_MPEG = 38,
		EB_HOOK_END_MPEG = 39,

		EB_HOOK_BEGIN_GRAPHIC_REFERENCE = 40,
		EB_HOOK_END_GRAPHIC_REFERENCE = 41,
		EB_HOOK_GRAPHIC_REFERENCE = 42,
		EB_HOOK_BEGIN_DECORATION = 43,
		EB_HOOK_END_DECORATION = 44,

		EB_HOOK_BEGIN_IMAGE_PAGE = 45,
		EB_HOOK_END_IMAGE_PAGE = 46,
		EB_HOOK_BEGIN_CLICKABLE_AREA = 47,
		EB_HOOK_END_CLICKABLE_AREA = 48,

		EB_HOOK_BEGIN_UNICODE = 49,
		EB_HOOK_END_UNICODE = 50,
		EB_HOOK_BEGIN_EBXAC_GAIJI = 51,
		EB_HOOK_END_EBXAC_GAIJI = 52,
		EB_HOOK_EBXAC_GAIJI = 53,
		EB_HOOK_UNICODE = 54,
	};
	/*
	 * Text-stop status.
	 */
	public enum class EBTextStatusCode
	{
		EB_TEXT_STATUS_CONTINUED = 0,
		EB_TEXT_STATUS_SOFT_STOP = 1,
		EB_TEXT_STATUS_HARD_STOP = 2,
	};
	/*
	 * Text content currently read.
	 */
	public enum class EBTextCode
	{
		EB_TEXT_INVALID = -1,
		EB_TEXT_MAIN_TEXT = 1,
		EB_TEXT_HEADING = 2,
		EB_TEXT_RAWTEXT = 3,
		EB_TEXT_OPTIONAL_TEXT = 4,
		EB_TEXT_SEEKED = 0,
	};
	/*
	 * Binary data types.
	 */
	public enum class EBBinaryCode
	{
		EB_BINARY_INVALID = -1,
		EB_BINARY_MONO_GRAPHIC = 0,
		EB_BINARY_COLOR_GRAPHIC = 1,
		EB_BINARY_WAVE = 2,
		EB_BINARY_MPEG = 3,
		EB_BINARY_GRAY_GRAPHIC = 4,
	};

	/*
	 * Search method currently processed.
	 */
	public enum class EBSearchCode
	{
		EB_SEARCH_NONE = -1,
		EB_SEARCH_EXACTWORD = 0,
		EB_SEARCH_WORD = 1,
		EB_SEARCH_ENDWORD = 2,
		EB_SEARCH_KEYWORD = 3,
		EB_SEARCH_MULTI = 4,
		EB_SEARCH_CROSS = 5,
	};

	/*
	 * Index Style flags.
	 */
	public enum class EBIndexStyleCode
	{
		EB_INDEX_STYLE_CONVERT = 0,
		EB_INDEX_STYLE_ASIS = 1,
		EB_INDEX_STYLE_REVERSED_CONVERT = 2,
		EB_INDEX_STYLE_DELETE = 2, // typo?
	};

	/*
	 * Disc code
	 */
	public enum class EBDiscCode
	{
		EB_DISC_INVALID,
		EB_DISC_EB,
		EB_DISC_EPWING,
	};

	/*
	 * Character codes.
	 */
	public enum class EBCharCode
	{
		EB_CHARCODE_INVALID = -1,
		EB_CHARCODE_ISO8859_1 = 1,
		EB_CHARCODE_JISX0208 = 2,
		EB_CHARCODE_JISX0208_GB2312 = 3,
		EB_CHARCODE_UTF8 = 4,
	};

	/*
	 * Error codes.
	 */
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