#pragma once

/*
 * Get an unsigned value from an octet stream buffer.
 */
#define eb_uint1(p) (*(const unsigned char *)(p))

#define eb_uint2(p) ((*(const unsigned char *)(p) << 8) \
        + (*(const unsigned char *)((p) + 1)))

#define eb_uint3(p) ((*(const unsigned char *)(p) << 16) \
        + (*(const unsigned char *)((p) + 1) << 8) \
        + (*(const unsigned char *)((p) + 2)))

#define eb_uint4(p) ((*(const unsigned char *)(p) << 24) \
        + (*(const unsigned char *)((p) + 1) << 16) \
        + (*(const unsigned char *)((p) + 2) << 8) \
        + (*(const unsigned char *)((p) + 3)))

#define eb_uint4_le(p) ((*(const unsigned char *)(p)) \
        + (*(const unsigned char *)((p) + 1) << 8) \
        + (*(const unsigned char *)((p) + 2) << 16) \
        + (*(const unsigned char *)((p) + 3) << 24))

#define eb_hexbyte(s) ( \
	( ( ( s[ 0 ] >= 'A' && s[ 0 ] <= 'Z' ) ? ( 10 + s[ 0 ] - 'A' ) : \
	( s[ 0 ] >= 'a' && s[ 0 ] <= 'z' ) ? ( 10 + s[ 0 ] - 'a' ) : \
		( s[ 0 ] >= '0' && s[ 0 ] <= '9' ) ? ( s[ 0 ] - '0' ) : 0 ) << 4 ) | \
		( ( s[ 1 ] >= 'A' && s[ 1 ] <= 'Z' ) ? ( 10 + s[ 1 ] - 'A' ) : \
	( s[ 1 ] >= 'a' && s[ 1 ] <= 'z' ) ? ( 10 + s[ 1 ] - 'a' ) : \
			( s[ 1 ] >= '0' && s[ 1 ] <= '9' ) ? ( s[ 1 ] - '0' ) : 0 ) );

/*
 * Test whether `off_t' represents a large integer.
 */
#define off_t_is_large \
    ((((off_t) 1 << 41) + ((off_t) 1 << 40) + 1) % 9999991 == 7852006)

/*
 * Data size of a book entry in a catalog file.
 */
#define EB_SIZE_EB_CATALOG		40
#define EB_SIZE_EPWING_CATALOG		164

/*
 * File names.
 */
#define EB_FILE_NAME_START      L"start"
#define EB_FILE_NAME_HONMON     "honmon"
#define EB_FILE_NAME_FUROKU     L"furoku"
#define EB_FILE_NAME_APPENDIX       L"appendix"

/*
 * Directory names.
 */
#define EB_DIRECTORY_NAME_DATA		L"data"
#define EB_DIRECTORY_NAME_GAIJI		L"gaiji"
#define EB_DIRECTORY_NAME_STREAM	"stream"
#define EB_DIRECTORY_NAME_MOVIE		L"movie"

/*
 * Maximum number of search titles.
 */
#define EB_MAX_SEARCH_TITLES		14
/*
 * Arrangement style of entries in a search index page.
 */
#define EB_ARRANGE_FIXED		0
#define EB_ARRANGE_VARIABLE		1
#define EB_ARRANGE_INVALID		-1
