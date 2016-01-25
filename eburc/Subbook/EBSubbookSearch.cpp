#include "pch.h"
#include "eburc/Book/EBBook.h"
#include "eburc/Subbook/EBSubbook.h"

using namespace libeburc;

/*
 * Page-ID macros.
 */
#define PAGE_ID_IS_LEAF_LAYER(page_id)		(((page_id) & 0x80) == 0x80)
#define PAGE_ID_IS_LAYER_START(page_id)		(((page_id) & 0x40) == 0x40)
#define PAGE_ID_IS_LAYER_END(page_id)		(((page_id) & 0x20) == 0x20)
#define PAGE_ID_HAVE_GROUP_ENTRY(page_id)	(((page_id) & 0x10) == 0x10)
/*
 * Book-code of the book in which you want to search a word.
 */
static EBBookCode cache_book_code = EB_BOOK_NONE;

/*
 * Cache buffer for the current page.
 */
static char cache_buffer[ EB_SIZE_PAGE ];

/*
 * Cache buffer for the current page.
 */
static int cache_page;
/*
 * The maximum number of hit entries for tomporary hit lists.
 * This is used in eb_hit_list().
 */
#define EB_TMP_MAX_HITS		64

void EBSubbook::LoadIndexes()
{
	/*
     * Read the index table in the subbook.
     */
	TextZio->LSeek( ( ( off_t ) IndexPage - 1 ) * EB_SIZE_PAGE, SEEK_SET );
	String^ str = TextZio->SrcFile->Name;

	Array<byte>^ buff = ref new Array<byte>( EB_SIZE_PAGE );
	TextZio->Read( EB_SIZE_PAGE, buff );
	byte* buffer = buff->Data;

	/*
     * Get start page numbers of the indexes in the subbook.
     */
	int index_count = eb_uint1( buffer + 1 );
	if ( EB_SIZE_PAGE / 16 - 1 <= index_count )
	{
		EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
	}

	/*
     * Get availavility flag of the index information.
     */
	int global_availability = eb_uint1( buffer + 4 );
	if ( 0x02 < global_availability )
		global_availability = 0;

	/*
     * Set each search method information.
     */
	int i;
	byte* buffer_p;
	EBSearch^ sebxa_zip_index;
    EBSearch^ sebxa_zip_text;

	for ( i = 0, buffer_p = buffer + 16; i < index_count; i++, buffer_p += 16 )
	{
		/*
		 * Set index style.
		 */
		EBSearch^ search = ref new EBSearch();

		search->index_id = eb_uint1( buffer_p );
		search->start_page = eb_uint4( buffer_p + 2 );
		search->end_page = search->start_page + eb_uint4( buffer_p + 6 ) - 1;

		/*
		 * Set canonicalization flags.
		 */
		int availability = eb_uint1( buffer_p + 10 );
		if ( ( global_availability == 0x00 && availability == 0x02 )
			|| global_availability == 0x02 )
		{
			unsigned int flags;

			flags = eb_uint3( buffer_p + 11 );
			search->katakana = ( EBIndexStyleCode ) ( ( flags & 0xc00000 ) >> 22 );
			search->lower = ( EBIndexStyleCode ) ( ( flags & 0x300000 ) >> 20 );
			if ( ( flags & 0x0c0000 ) >> 18 == 0 )
				search->mark = EBIndexStyleCode::EB_INDEX_STYLE_DELETE;
			else
				search->mark = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
			search->long_vowel = ( EBIndexStyleCode ) ( ( flags & 0x030000 ) >> 16 );
			search->double_consonant = ( EBIndexStyleCode ) ( ( flags & 0x00c000 ) >> 14 );
			search->contracted_sound = ( EBIndexStyleCode ) ( ( flags & 0x003000 ) >> 12 );
			search->small_vowel = ( EBIndexStyleCode ) ( ( flags & 0x000c00 ) >> 10 );
			search->voiced_consonant = ( EBIndexStyleCode ) ( ( flags & 0x000300 ) >> 8 );
			search->p_sound = ( EBIndexStyleCode ) ( ( flags & 0x0000c0 ) >> 6 );

		}
		else if ( search->index_id == 0x70 || search->index_id == 0x90 )
		{
			search->katakana = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
			search->lower = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
			search->mark = EBIndexStyleCode::EB_INDEX_STYLE_DELETE;
			search->long_vowel = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
			search->double_consonant = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
			search->contracted_sound = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
			search->small_vowel = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
			search->voiced_consonant = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
			search->p_sound = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
		}
		else
		{
			search->katakana = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
			search->lower = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
			search->mark = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
			search->long_vowel = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
			search->double_consonant = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
			search->contracted_sound = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
			search->small_vowel = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
			search->voiced_consonant = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
			search->p_sound = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
		}

		if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_ISO8859_1
			|| search->index_id == 0x72
			|| search->index_id == 0x92 )
		{
			search->space = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
		}
		else
		{
			search->space = EBIndexStyleCode::EB_INDEX_STYLE_DELETE;
		}

		/*
		 * Identify search method.
		 */
		switch ( search->index_id )
		{
		case 0x00:
			if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
				|| ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				text = search;
			}
			break;
		case 0x01:
			if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
				|| ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				menu = search;
			}
			break;
		case 0x02:
			if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
				|| ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				copyright = search;
			}
			break;
		case 0x10:
			image_menu = search;
			break;
		case 0x16:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EPWING )
				search_title_page = search->start_page;
			break;
		case 0x21:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB
				&& TextZio->Code == ZioCode::ZIO_PLAIN )
				sebxa_zip_text = search;
			break;
		case 0x22:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB
				&& TextZio->Code == ZioCode::ZIO_PLAIN )
				sebxa_zip_index = search;
			break;
		case 0x70:
			if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
				|| ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				endword_kana = search;
			}
			break;
		case 0x71:
			if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
				|| ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				endword_asis = search;
			}
			break;
		case 0x72:
			endword_alphabet = search;
			break;
		case 0x80:
			if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
				|| ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				keyword = search;
			}
			break;
		case 0x81:
			if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
				|| ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				cross = search;
			}
			break;
		case 0x90:
			if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
				|| ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				word_kana = search;
			}
			break;
		case 0x91:
			if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
				|| ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				word_asis = search;
			}
			break;
		case 0x92:
			word_alphabet = search;
			break;
		case 0xc0:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EPWING
				&& ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				text = search;
			}
			break;
		case 0xc1:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EPWING
				&& ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				menu = search;
			}
			break;
		case 0xc2:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EPWING
				&& ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				copyright = search;
			}
			break;
		case 0xc6:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EPWING
				&& ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				word_asis = search;
			}
			break;
		case 0xc7:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EPWING
				&& ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				endword_asis = search;
			}
			break;
		case 0xc8:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EPWING
				&& ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				keyword = search;
			}
			break;
		case 0xc9:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EPWING
				&& ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				cross = search;
			}
			break;
		case 0xca:
			if ( multi_count < EB_MAX_MULTI_SEARCHES
				&& ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				EBMultiSearch^ mSearch = ref new EBMultiSearch();
				mSearch->search = search;
				multis[ multi_count++ ] = mSearch;
			}
			break;
		case 0xcb:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EPWING
				&& ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				table_page = search->start_page;
				table_size = search->end_page - search->start_page + 1;
				LoadUTF8Table();
			}
			break;
		case 0xd8:
			sound = search;
			break;
		case 0xf1:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBWideFont( this );
				font->page = search->start_page;
				font->font_code = EB_FONT_16;
				wide_fonts[ EB_FONT_16 ] = font;
			}
			break;
		case 0xf2:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBNarrowFont( this );
				font->page = search->start_page;
				font->font_code = EB_FONT_16;
				narrow_fonts[ EB_FONT_16 ] = font;
			}
			break;
		case 0xf3:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBWideFont( this );
				font->page = search->start_page;
				font->font_code = EB_FONT_24;
				wide_fonts[ EB_FONT_24 ] = font;
			}
			break;
		case 0xf4:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBNarrowFont( this );
				font->page = search->start_page;
				font->font_code = EB_FONT_24;
				narrow_fonts[ EB_FONT_24 ] = font;
			}
			break;
		case 0xf5:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBWideFont( this );
				font->page = search->start_page;
				font->font_code = EB_FONT_30;
				wide_fonts[ EB_FONT_30 ] = font;
			}
			break;
		case 0xf6:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBNarrowFont( this );
				font->page = search->start_page;
				font->font_code = EB_FONT_30;
				narrow_fonts[ EB_FONT_30 ] = font;
			}
			break;
		case 0xf7:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBWideFont( this );
				font->page = search->start_page;
				font->font_code = EB_FONT_48;
				wide_fonts[ EB_FONT_48 ] = font;
			}
			break;
		case 0xf8:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBNarrowFont( this );
				font->page = search->start_page;
				font->font_code = EB_FONT_48;
				narrow_fonts[ EB_FONT_48 ] = font;
			}
			break;
		case 0xff:
			if ( multi_count < EB_MAX_MULTI_SEARCHES
				&& ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				EBMultiSearch^ mSearch = ref new EBMultiSearch();
				mSearch->search = search;
				multis[ multi_count++ ] = mSearch;
			}
			break;
		}
	}

	// the function there was comment as *Nothing to be done*
	// So leaving it here for future reference
	// eb_finalize_search( &sebxa_zip_index );
	// eb_finalize_search( &sebxa_zip_text );
}

void EBSubbook::LoadMultiSearches()
{
	int index_count;
	int index_id;

	for ( int i = 0; i < multi_count; i++ )
	{
		EBMultiSearch^ multi = multis[ i ];
		/*
		 * Read the index table page of the multi search.
		 */
		TextZio->LSeek( ( ( off_t ) multi->search->start_page - 1 ) * EB_SIZE_PAGE, SEEK_SET );

		Array<byte>^ buff = ref new Array<byte>( EB_SIZE_PAGE );
		TextZio->Read( EB_SIZE_PAGE, buff );
		byte* buffer = buff->Data;

		/*
		 * Get the number of entries in this multi search.
		 */
		multi->entry_count = eb_uint2( buffer );
		if ( EB_MAX_MULTI_SEARCHES <= multi->entry_count )
		{
			EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
		}

		byte* buffer_p = buffer + 16;
		for ( int j = 0; j < multi->entry_count; j++ )
		{
			EBSearch^ entry = ref new EBSearch();
			/*
			 * Get the number of indexes in this entry, and title
			 * of this entry.
			 */
			index_count = eb_uint1( buffer_p );

			strncpy_s( entry->label, ( char * ) buffer_p + 2, EB_MAX_MULTI_LABEL_LENGTH );
			entry->label[ EB_MAX_MULTI_LABEL_LENGTH ] = '\0';
			JACode::eb_jisx0208_to_euc( entry->label, entry->label );
			buffer_p += EB_MAX_MULTI_LABEL_LENGTH + 2;

			/*
			 * Initialize index page information of the entry.
			 */
			for ( int k = 0; k < index_count; k++ )
			{
				/*
				 * Get the index page information of the entry.
				 */
				index_id = eb_uint1( buffer_p );
				switch ( index_id )
				{
				case 0x71:
				case 0x91:
				case 0xa1:
					if ( entry->start_page != 0 && entry->index_id != 0x71 )
						break;
					entry->start_page = eb_uint4( buffer_p + 2 );
					entry->end_page = entry->start_page
						+ eb_uint4( buffer_p + 6 ) - 1;
					entry->index_id = index_id;
					entry->katakana = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
					entry->lower = EBIndexStyleCode::EB_INDEX_STYLE_CONVERT;
					entry->mark = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
					entry->long_vowel = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
					entry->double_consonant = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
					entry->contracted_sound = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
					entry->voiced_consonant = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
					entry->small_vowel = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
					entry->p_sound = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
					entry->space = EBIndexStyleCode::EB_INDEX_STYLE_ASIS;
					break;
				case 0x01:
					entry->candidates_page = eb_uint4( buffer_p + 2 );
					break;
				}
				buffer_p += 16;
			}

			multi->entries[ j ] = entry;
		}
	}
}


/*
 * Default multi search titles (written in JIS X 0208).
 */
static const char *default_multi_titles_jisx0208[] = {
	"J#9g8!:w#1",    /* Multi search 1. */
	"J#9g8!:w#2",    /* Multi search 2. */
	"J#9g8!:w#3",    /* Multi search 3. */
	"J#9g8!:w#4",    /* Multi search 4. */
	"J#9g8!:w#5",    /* Multi search 5. */
	"J#9g8!:w#6",    /* Multi search 6. */
	"J#9g8!:w#7",    /* Multi search 7. */
	"J#9g8!:w#8",    /* Multi search 8. */
	"J#9g8!:w#9",    /* Multi search 9. */
	"J#9g8!:w#1#0",  /* Multi search 10. */
};

/*
 * Default multi search titles (written in ASCII, subset of ISO 8859-1).
 */
static const char *default_multi_titles_latin[] = {
	"Multi search 1",
	"Multi search 2",
	"Multi search 3",
	"Multi search 4",
	"Multi search 5",
	"Multi search 6",
	"Multi search 7",
	"Multi search 8",
	"Multi search 9",
	"Multi search 10",
};


void EBSubbook::LoadMultiTitles()
{
	int title_count;
	size_t offset;
	int i;
	/*
	 * Set default titles.
	 */
	if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_ISO8859_1
		|| ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
	{
		for ( i = 0; i < multi_count; i++ )
		{
			strcpy_s( multis[i]->title, default_multi_titles_latin[ i ] );
		}
	}
	else
	{
		for ( i = 0; i < multi_count; i++ )
		{
			strcpy_s( multis[ i ]->title, default_multi_titles_jisx0208[ i ] );

			char * title = multis[ i ]->title;
			JACode::eb_jisx0208_to_euc( title, title );
		}
	}

	if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING || search_title_page == 0 )
		return;

	/*
	 * Read the page of the multi search.
	 */
	TextZio->LSeek( ( ( off_t ) search_title_page - 1 ) * EB_SIZE_PAGE, SEEK_SET );
	Array<byte>^ buff = ref new Array<byte>( EB_SIZE_PAGE );
	TextZio->Read( EB_SIZE_PAGE, buff );
	byte* buffer = buff->Data;

	title_count = eb_uint2( buffer );
	if ( EB_MAX_SEARCH_TITLES < title_count )
		title_count = EB_MAX_SEARCH_TITLES;

	/*
	 * We need titles for multi searches only.
	 *     titles[ 0]: title for word and endword searches.
	 *     titles[ 1]: title for keyword search.
	 *     titles[ 2]: common title for all multi searches.
	 *                (we don't need this)
	 *     titles[ 3]: title for multi search 1.
	 *         :
	 *     titles[12]: title for multi search 10.
	 *     titles[13]: title for menu search.
	 *
	 * The offset of titles[3] is:
	 *     the number of entries(2bytes)
	 *     + reserved 1 (68bytes)
	 *     + title for word and endword searches (70bytes)
	 *     + title for keyword search (70bytes)
	 *     + common title for all multi searches (70bytes)
	 *     + reserved 2 (70bytes)
	 *     = 2 + 68 + 70 + 70 + 70 + 70 = 350
	 */
	for ( i = 4, offset = 350; i < EB_MAX_SEARCH_TITLES; i++, offset += 70 )
	{
		if ( multi_count <= i - 4 )
			break;
		if ( eb_uint2( buffer + offset ) != 0x02 )
			continue;

		/*
		 * Each titles[] consists of
		 *    parameter (2bytes)
		 *    short title (16bytes)
		 *    long title (32bytes)
		 * We get long title rather than short one.
		 */
		strncpy_s( multis[ i - 4 ]->title, ( char * ) ( buffer + offset + 2 + 16 ), EB_MAX_MULTI_TITLE_LENGTH );

		char* title = multis[ i - 4 ]->title;
		title[ EB_MAX_MULTI_TITLE_LENGTH ] = '\0';
		JACode::eb_jisx0208_to_euc( title, title );
	}
}

void EBSubbook::PreSearchWord( EBSearchContext^ context )
{
	int next_page;
	int index_depth;
	char *cache_p;

	/*
	 * Discard cache data.
	 */
	cache_book_code = EB_BOOK_NONE;

	/*
	 * Search the word in intermediate indexes.
	 * Find a page number of the leaf index page.
	 */
	for ( index_depth = 0; index_depth < EB_MAX_INDEX_DEPTH; index_depth++ )
	{
		next_page = context->page;

		/*
		 * Seek and read a page.
		 */
		TextZio->LSeek( ( ( off_t ) context->page - 1 ) * EB_SIZE_PAGE, SEEK_SET );

		Array<byte>^ buff = ref new Array<byte>( EB_SIZE_PAGE );
		TextZio->Read( EB_SIZE_PAGE, buff );
		memcpy_s( cache_buffer, EB_SIZE_PAGE, buff->Data, EB_SIZE_PAGE );

		/*
		 * Get some data from the read page.
		 */
		context->page_id = eb_uint1( cache_buffer );
		context->entry_length = eb_uint1( cache_buffer + 1 );
		if ( context->entry_length == 0 )
			context->entry_arrangement = EB_ARRANGE_VARIABLE;
		else
			context->entry_arrangement = EB_ARRANGE_FIXED;
		context->entry_count = eb_uint2( cache_buffer + 2 );
		context->offset = 4;
		cache_p = cache_buffer + 4;

		/*
		 * Exit the loop if it reached to the leaf index.
		 */
		if ( PAGE_ID_IS_LEAF_LAYER( context->page_id ) )
			break;

		/*
		 * Search a page of next level index.
		 */
		for ( context->entry_index = 0;
		context->entry_index < context->entry_count;
			context->entry_index++ )
		{
			if ( EB_SIZE_PAGE < context->offset + context->entry_length + 4 )
			{
				EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
			}
			if ( context->compare_pre( context->canonicalized_word, cache_p,
				context->entry_length ) <= 0 )
			{
				next_page = eb_uint4( cache_p + context->entry_length );
				break;
			}
			cache_p += context->entry_length + 4;
			context->offset += context->entry_length + 4;
		}
		if ( context->entry_count <= context->entry_index
			|| context->page == next_page )
		{
			context->comparison_result = -1;
			return;
		}
		context->page = next_page;
	}

	/*
	 * Check for the index depth.
	 */
	if ( index_depth == EB_MAX_INDEX_DEPTH )
	{
		EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
	}

	/*
	 * Update search context and cache information.
	 */
	context->entry_index = 0;
	context->comparison_result = 1;
	context->in_group_entry = 0;
	cache_book_code = ParentBook->code;
	cache_page = context->page;
}

/*
 * Do AND operation of hit lists.
 * and_list = hit_lists[0] AND hit_lists[1] AND ...
 */
static void AndHitLists( EBHit^ and_list[ EB_TMP_MAX_HITS ], int *and_count,
	int max_and_count, int hit_list_count,
	EBHit^ hit_lists[ EB_NUMBER_OF_SEARCH_CONTEXTS ][ EB_TMP_MAX_HITS ],
	int hit_counts[ EB_NUMBER_OF_SEARCH_CONTEXTS ] )
{
	int hit_indexes[ EB_NUMBER_OF_SEARCH_CONTEXTS ];
	/*
	 * Initialize indexes for the hit_lists[].
	 */
	for ( int i = 0; i < hit_list_count; i++ )
		hit_indexes[ i ] = 0;

	/*
	 * Generate the new list `and_list'.
	 */
	*and_count = 0;
	while ( *and_count < max_and_count )
	{
		/*
		 * Initialize variables.
		 */
		int greatest_list = -1;
		int greatest_page = 0;
		int greatest_offset = 0;
		int current_page = 0;
		int current_offset = 0;
		int equal_count = 0;

		/*
		 * Compare the current elements of the lists.
		 */
		for ( int i = 0; i < hit_list_count; i++ )
		{
			/*
			 * If we have been reached to the tail of the hit_lists[i],
			 * skip the list.
			 */
			if ( hit_counts[ i ] <= hit_indexes[ i ] )
				continue;

			/*
			 * Compare {current_page, current_offset} and {greatest_page,
			 * greatest_offset}.
			 */
			current_page = hit_lists[ i ][ hit_indexes[ i ] ]->text->page;
			current_offset = hit_lists[ i ][ hit_indexes[ i ] ]->text->offset;

			if ( greatest_list == -1 )
			{
				greatest_page = current_page;
				greatest_offset = current_offset;
				greatest_list = i;
				equal_count++;
			}
			else if ( greatest_page < current_page )
			{
				greatest_page = current_page;
				greatest_offset = current_offset;
				greatest_list = i;
			}
			else if ( current_page == greatest_page
				&& greatest_offset < current_offset )
			{
				greatest_page = current_page;
				greatest_offset = current_offset;
				greatest_list = i;
			}
			else if ( current_page == greatest_page
				&& current_offset == greatest_offset )
			{
				equal_count++;
			}
		}

		if ( equal_count == hit_list_count )
		{
			/*
			 * All the current elements of the lists point to the same
			 * position.  This is hit element.  Increase indexes of all
			 * lists.
			 */
			and_list[ *and_count ] = ref new EBHit( hit_lists[ 0 ][ hit_indexes[ 0 ] ] );
			*and_count += 1;
			for ( int i = 0; i < hit_list_count; i++ )
			{
				if ( hit_counts[ i ] <= hit_indexes[ i ] )
					continue;
				hit_indexes[ i ]++;
			}
		}
		else
		{
			/*
			 * This is not hit element.  Increase indexes of all lists
			 * except for greatest element(s).  If there is no list
			 * whose index is incremented, our job has been completed.
			 */
			int increment_count = 0;
			for ( int i = 0; i < hit_list_count; i++ )
			{
				if ( hit_counts[ i ] <= hit_indexes[ i ] )
					continue;
				current_page = hit_lists[ i ][ hit_indexes[ i ] ]->text->page;
				current_offset = hit_lists[ i ][ hit_indexes[ i ] ]->text->offset;
				if ( current_page != greatest_page
					|| current_offset != greatest_offset )
				{
					hit_indexes[ i ]++;
					increment_count++;
				}
			}
			if ( increment_count == 0 )
				break;
		}
	}

	/*
	 * Update hit_counts[].
	 * The hit counts of the lists are set to the current indexes.
	 */
	for ( int i = 0; i < hit_list_count; i++ )
		hit_counts[ i ] = hit_indexes[ i ];
}

void EBSubbook::HitListKeyword( EBSearchContext^ context, int max_hit_count, EBHit^ *hit_list, int *hit_count )
{
	EBHit^ *hit = hit_list;
	*hit_count = 0;

	/*
	 * Backup the text context in `book'
	 */
	EBTextContext^ text_context = ref new EBTextContext( ParentBook->text_context );

	try
	{

		/*
		 * Seek text file
		 */
		if ( context->in_group_entry && context->comparison_result == 0 )
		{
			SeekText( context->keyword_heading );
		}

		/*
		 * If the result of previous comparison is negative value, all
		 * matched entries have been found
		 */
		if ( context->comparison_result < 0 || max_hit_count <= 0 )
			goto succeeded;

		for ( ;;)
		{
			/*
			 * Read a page to search, if the page is not on the cache buffer
			 *
			 * Cache may be missed by the two reasons:
			 *   1-> the search process reaches to the end of an index page,
			 *      and tries to read the next page
			 *   2-> Someone else used the cache buffer
			 *
			 * At the case of 1, the search process reads the page and update
			 * the search context->  At the case of 2-> it reads the page but
			 * must not update the context!
			 */
			if ( cache_book_code != ParentBook->code || cache_page != context->page )
			{
				TextZio->LSeek( ( ( off_t ) context->page - 1 ) * EB_SIZE_PAGE, SEEK_SET );
				Array<byte>^ buff = ref new Array<byte>( EB_SIZE_PAGE );
				TextZio->Read( EB_SIZE_PAGE, buff );
				memcpy_s( cache_buffer, EB_SIZE_PAGE, buff->Data, EB_SIZE_PAGE );
				/*
				 * Update search context.
				 */
				if ( context->entry_index == 0 )
				{
					context->page_id = eb_uint1( cache_buffer );
					context->entry_length = eb_uint1( cache_buffer + 1 );
					if ( context->entry_length == 0 )
						context->entry_arrangement = EB_ARRANGE_VARIABLE;
					else
						context->entry_arrangement = EB_ARRANGE_FIXED;
					context->entry_count = eb_uint2( cache_buffer + 2 );
					context->entry_index = 0;
					context->offset = 4;
				}

				cache_book_code = ParentBook->code;
				cache_page = context->page;
			}

			char *cache_p = cache_buffer + context->offset;

			if ( !PAGE_ID_IS_LEAF_LAYER( context->page_id ) )
			{
				/*
				 * Not a leaf index->  It is an error
				 */
				EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
			}

			if ( !PAGE_ID_HAVE_GROUP_ENTRY( context->page_id )
				&& context->entry_arrangement == EB_ARRANGE_FIXED )
			{
				/*
				 * The leaf index doesn't have a group entry
				 * Find text and heading locations
				 */
				while ( context->entry_index < context->entry_count )
				{
					if ( EB_SIZE_PAGE
						< context->offset + context->entry_length + 12 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}

					/*
					 * Compare word and pattern
					 * If matched, add it to a hit list
					 */
					context->comparison_result
						= context->compare_single( context->word, cache_p,
							context->entry_length );
					if ( context->comparison_result == 0 )
					{
						*hit = ref new EBHit(
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 6 ),
								eb_uint2( cache_p + context->entry_length + 10 ) ),
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length ),
								eb_uint2( cache_p + context->entry_length + 4 ) )
							);
						hit++;
						*hit_count += 1;
					}
					context->entry_index++;
					context->offset += context->entry_length + 12;
					cache_p += context->entry_length + 12;

					if ( context->comparison_result < 0
						|| max_hit_count <= *hit_count )
						goto succeeded;
				}

			}
			else if ( !PAGE_ID_HAVE_GROUP_ENTRY( context->page_id )
				&& context->entry_arrangement == EB_ARRANGE_VARIABLE )
			{
				/*
				 * The leaf index doesn't have a group entry
				 * Find text and heading locations
				 */
				while ( context->entry_index < context->entry_count )
				{
					if ( EB_SIZE_PAGE < context->offset + 1 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}
					context->entry_length = eb_uint1( cache_p );
					if ( EB_SIZE_PAGE
						< context->offset + context->entry_length + 13 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}

					/*
					 * Compare word and pattern
					 * If matched, add it to a hit list
					 */
					context->comparison_result
						= context->compare_single( context->word, cache_p + 1,
							context->entry_length );
					if ( context->comparison_result == 0 )
					{
						*hit = ref new EBHit(
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 7 ),
								eb_uint2( cache_p + context->entry_length + 11 ) ),
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 1 ),
								eb_uint2( cache_p + context->entry_length + 5 ) )
							);
						hit++;
						*hit_count += 1;
					}
					context->entry_index++;
					context->offset += context->entry_length + 13;
					cache_p += context->entry_length + 13;

					if ( context->comparison_result < 0
						|| max_hit_count <= *hit_count )
						goto succeeded;
				}

			}
			else
			{
				/*
				 * The leaf index have a group entry
				 * Find text and heading locations
				 */
				while ( context->entry_index < context->entry_count )
				{
					if ( EB_SIZE_PAGE < context->offset + 2 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}
					int group_id = eb_uint1( cache_p );

					if ( group_id == 0x00 )
					{
						/*
						 * 0x00 -- Single entry
						 */
						context->entry_length = eb_uint1( cache_p + 1 );
						if ( EB_SIZE_PAGE
							< context->offset + context->entry_length + 14 )
						{
							EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
						}

						/*
						 * Compare word and pattern
						 * If matched, add it to a hit list
						 */
						context->comparison_result
							= context->compare_single( context->canonicalized_word,
								cache_p + 2, context->entry_length );
						if ( context->comparison_result == 0 )
						{
							*hit = ref new EBHit(
								ref new EBPosition(
									eb_uint4( cache_p + context->entry_length + 8 ),
									eb_uint2( cache_p + context->entry_length + 12 ) ),
								ref new EBPosition(
									eb_uint4( cache_p + context->entry_length + 2 ),
									eb_uint2( cache_p + context->entry_length + 6 ) )
								);
							hit++;
							*hit_count += 1;
						}
						context->in_group_entry = 0;
						context->offset += context->entry_length + 14;
						cache_p += context->entry_length + 14;

					}
					else if ( group_id == 0x80 )
					{
						/*
						 * 0x80 -- Start of group entry
						 */
						context->entry_length = eb_uint1( cache_p + 1 );
						if ( EB_SIZE_PAGE
							< context->offset + context->entry_length + 12 )
						{
							EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
						}
						context->comparison_result
							= context->compare_single( context->word, cache_p + 6,
								context->entry_length );
						context->keyword_heading->page
							= eb_uint4( cache_p + context->entry_length + 6 );
						context->keyword_heading->offset
							= eb_uint2( cache_p + context->entry_length + 10 );
						context->in_group_entry = 1;
						cache_p += context->entry_length + 12;
						context->offset += context->entry_length + 12;

						if ( context->comparison_result == 0 )
						{
							SeekText( context->keyword_heading );
						}

					}
					else if ( group_id == 0xc0 )
					{
						/*
						 * Element of the group entry
						 */
						if ( EB_SIZE_PAGE < context->offset + 7 )
						{
							EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
						}

						/*
						 * Compare word and pattern
						 * If matched, add it to a hit list
						 */
						if ( context->in_group_entry
							&& context->comparison_result == 0 )
						{
							context->keyword_heading = TellText();
							*hit = ref new EBHit(
								ref new EBPosition(
									context->keyword_heading->page,
									context->keyword_heading->offset ),
								ref new EBPosition(
									eb_uint4( cache_p + 1 ),
									eb_uint2( cache_p + 5 ) )
								);
							hit++;
							*hit_count += 1;
							ForwardHeading();
						}
						context->offset += 7;
						cache_p += 7;

					}
					else
					{
						/*
						 * Unknown group ID.
						 */
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}

					context->entry_index++;
					if ( context->comparison_result < 0
						|| max_hit_count <= *hit_count )
						goto succeeded;
				}
			}

			/*
			 * Go to a next page if available.
			 */
			if ( PAGE_ID_IS_LAYER_END( context->page_id ) )
			{
				context->comparison_result = -1;
				goto succeeded;
			}
			context->page++;
			context->entry_index = 0;
		}

	succeeded:
		if ( context->in_group_entry && context->comparison_result == 0 )
		{
			context->keyword_heading = TellText();
		}

		/*
		 * Restore the text context in `book'
		 */
		ParentBook->text_context = text_context;
	}
	catch ( Exception^ ex )
	{
		*hit_count = 0;
		ParentBook->text_context = text_context;
		throw ex;
	}
}

void EBSubbook::HitListMulti( EBSearchContext^ context, int max_hit_count, EBHit^ *hit_list, int *hit_count )
{
	EBHit^ *hit = hit_list;
	*hit_count = 0;

	/*
	 * If the result of previous comparison is negative value, all
	 * matched entries have been found
	 */
	if ( context->comparison_result < 0 || max_hit_count <= 0 ) return;

	try
	{

		for ( ;;)
		{
			/*
			 * Read a page to search, if the page is not on the cache buffer
			 *
			 * Cache may be missed by the two reasons:
			 *   1-> the search process reaches to the end of an index page,
			 *      and tries to read the next page
			 *   2-> Someone else used the cache buffer
			 *
			 * At the case of 1, the search process reads the page and update
			 * the search context->  At the case of 2-> it reads the page but
			 * must not update the context!
			 */
			if ( cache_book_code != ParentBook->code || cache_page != context->page )
			{
				TextZio->LSeek( ( ( off_t ) context->page - 1 ) * EB_SIZE_PAGE, SEEK_SET );
				Array<byte>^ buff = ref new Array<byte>( EB_SIZE_PAGE );
				TextZio->Read( EB_SIZE_PAGE, buff );
				memcpy_s( cache_buffer, EB_SIZE_PAGE, buff->Data, EB_SIZE_PAGE );

				/*
				 * Update search context
				 */
				if ( context->entry_index == 0 )
				{
					context->page_id = eb_uint1( cache_buffer );
					context->entry_length = eb_uint1( cache_buffer + 1 );
					if ( context->entry_length == 0 )
						context->entry_arrangement = EB_ARRANGE_VARIABLE;
					else
						context->entry_arrangement = EB_ARRANGE_FIXED;
					context->entry_count = eb_uint2( cache_buffer + 2 );
					context->entry_index = 0;
					context->offset = 4;
				}

				cache_book_code = ParentBook->code;
				cache_page = context->page;
			}

			char *cache_p = cache_buffer + context->offset;

			if ( !PAGE_ID_IS_LEAF_LAYER( context->page_id ) )
			{
				/*
				 * Not a leaf index.  It is an error
				 */
				EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );

			}

			if ( !PAGE_ID_HAVE_GROUP_ENTRY( context->page_id )
				&& context->entry_arrangement == EB_ARRANGE_FIXED )
			{
				/*
				 * The leaf index doesn't have a group entry
				 * Find text and heading locations
				 */
				while ( context->entry_index < context->entry_count )
				{
					if ( EB_SIZE_PAGE
						< context->offset + context->entry_length + 13 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );

					}

					/*
					 * Compare word and pattern
					 * If matched, add it to a hit list
					 */
					context->comparison_result
						= context->compare_single( context->word, cache_p,
							context->entry_length );
					if ( context->comparison_result == 0 )
					{
						*hit = ref new EBHit(
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 6 ),
								eb_uint2( cache_p + context->entry_length + 10 ) ),
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length ),
								eb_uint2( cache_p + context->entry_length + 4 ) )
							);
						hit++;
						*hit_count += 1;
					}
					context->entry_index++;
					context->offset += context->entry_length + 12;
					cache_p += context->entry_length + 12;

					if ( context->comparison_result < 0
						|| max_hit_count <= *hit_count )
						return;
				}

			}
			else if ( !PAGE_ID_HAVE_GROUP_ENTRY( context->page_id )
				&& context->entry_arrangement == EB_ARRANGE_VARIABLE )
			{
				/*
				 * The leaf index doesn't have a group entry
				 * Find text and heading locations
				 */
				while ( context->entry_index < context->entry_count )
				{
					if ( EB_SIZE_PAGE < context->offset + 1 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );

					}
					context->entry_length = eb_uint1( cache_p );
					if ( EB_SIZE_PAGE
						< context->offset + context->entry_length + 13 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );

					}

					/*
					 * Compare word and pattern
					 * If matched, add it to a hit list
					 */
					context->comparison_result
						= context->compare_single( context->word, cache_p + 1,
							context->entry_length );
					if ( context->comparison_result == 0 )
					{
						*hit = ref new EBHit(
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 7 ),
								eb_uint2( cache_p + context->entry_length + 11 ) ),
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 1 ),
								eb_uint2( cache_p + context->entry_length + 5 ) )
							);
						hit++;
						*hit_count += 1;
					}
					context->entry_index++;
					context->offset += context->entry_length + 13;
					cache_p += context->entry_length + 13;

					if ( context->comparison_result < 0
						|| max_hit_count <= *hit_count )
						return;
				}

			}
			else
			{
				/*
				 * The leaf index have a group entry
				 * Find text and heading locations
				 */
				while ( context->entry_index < context->entry_count )
				{
					if ( EB_SIZE_PAGE < context->offset + 2 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}
					int group_id = eb_uint1( cache_p );

					if ( group_id == 0x00 )
					{
						/*
						 * 0x00 -- Single entry
						 */
						context->entry_length = eb_uint1( cache_p + 1 );
						if ( EB_SIZE_PAGE
							< context->offset + context->entry_length + 14 )
						{
							EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
						}

						/*
						 * Compare word and pattern
						 * If matched, add it to a hit list
						 */
						context->comparison_result
							= context->compare_single( context->canonicalized_word,
								cache_p + 2, context->entry_length );
						if ( context->comparison_result == 0 )
						{
							*hit = ref new EBHit(
								ref new EBPosition(
									eb_uint4( cache_p + context->entry_length + 8 ),
									eb_uint2( cache_p + context->entry_length + 12 ) ),
								ref new EBPosition(
									eb_uint4( cache_p + context->entry_length + 2 ),
									eb_uint2( cache_p + context->entry_length + 6 ) )
								);
							hit++;
							*hit_count += 1;
						}
						context->in_group_entry = 0;
						context->offset += context->entry_length + 14;
						cache_p += context->entry_length + 14;

					}
					else if ( group_id == 0x80 )
					{
						/*
						 * 0x80 -- Start of group entry
						 */
						context->entry_length = eb_uint1( cache_p + 1 );
						if ( EB_SIZE_PAGE
							< context->offset + context->entry_length + 6 )
						{
							EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
						}
						context->comparison_result
							= context->compare_single( context->word, cache_p + 6,
								context->entry_length );
						context->in_group_entry = 1;
						cache_p += context->entry_length + 6;
						context->offset += context->entry_length + 6;

					}
					else if ( group_id == 0xc0 )
					{
						/*
						 * Element of the group entry
						 */
						if ( EB_SIZE_PAGE < context->offset + 13 )
						{
							EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
						}

						/*
						 * Compare word and pattern
						 * If matched, add it to a hit list
						 */
						if ( context->in_group_entry
							&& context->comparison_result == 0 )
						{
							*hit = ref new EBHit(
								ref new EBPosition(
									eb_uint4( cache_p + 7 ),
									eb_uint2( cache_p + 11 ) ),
								ref new EBPosition(
									eb_uint4( cache_p + 1 ),
									eb_uint2( cache_p + 5 ) )
								);
							hit++;
							*hit_count += 1;
						}
						context->offset += 13;
						cache_p += 13;

					}
					else
					{
						/*
						 * Unknown group ID
						 */
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}

					context->entry_index++;
					if ( context->comparison_result < 0 || max_hit_count <= *hit_count )
						return;
				}
			}

			/*
			 * Go to a next page if available.
			 */
			if ( PAGE_ID_IS_LAYER_END( context->page_id ) )
			{
				context->comparison_result = -1;
				return;
			}
			context->page++;
			context->entry_index = 0;
		}

	}
	catch ( Exception^ ex )
	{
		*hit_count = 0;
		throw ex;
	}
}

void EBSubbook::HitList( int max_hit_count, EBHit^ *hit_list, int *hit_count )
{
	EBHit^ temporary_hit_lists[ EB_NUMBER_OF_SEARCH_CONTEXTS ][ EB_TMP_MAX_HITS ];
	int temporary_hit_counts[ EB_NUMBER_OF_SEARCH_CONTEXTS ];
	int more_hit_count;
	int i;


	if ( max_hit_count == 0 ) return;

	*hit_count = 0;

	/*
	 * Get a list of hit entries.
	 */
	switch ( ParentBook->search_contexts->GetAt(0)->code )
	{
	case EBSearchCode::EB_SEARCH_EXACTWORD:
	case EBSearchCode::EB_SEARCH_WORD:
	case EBSearchCode::EB_SEARCH_ENDWORD:
		/*
		 * In case of exactword, word of endword search.
		 */
		HitListWord( max_hit_count, hit_list, hit_count );
		break;

	case EBSearchCode::EB_SEARCH_KEYWORD:
	case EBSearchCode::EB_SEARCH_CROSS:
		/*
		 * In case of keyword or cross search.
		 */
		for ( ;;)
		{
			int search_is_over = 0;

			for ( i = 0; i < EB_MAX_KEYWORDS; i++ )
			{
				EBSearchContext^ search_context = ParentBook->search_contexts->GetAt( i );
				if ( search_context->code != EBSearchCode::EB_SEARCH_KEYWORD
					&& search_context->code != EBSearchCode::EB_SEARCH_CROSS )
					break;
				EBSearchContext^ temporary_context = ref new EBSearchContext( search_context );

				HitListKeyword( temporary_context,
					EB_TMP_MAX_HITS, temporary_hit_lists[ i ],
					temporary_hit_counts + i );

				if ( temporary_hit_counts[ i ] == 0 )
				{
					search_is_over = 1;
					break;
				}
			}
			if ( search_is_over )
				break;

			AndHitLists( hit_list + *hit_count, &more_hit_count,
				max_hit_count - *hit_count, i, temporary_hit_lists,
				temporary_hit_counts );

			for ( i = 0; i < EB_MAX_MULTI_ENTRIES; i++ )
			{
				EBSearchContext^ search_context = ParentBook->search_contexts->GetAt( i );
				if ( search_context->code != EBSearchCode::EB_SEARCH_KEYWORD
					&& search_context->code != EBSearchCode::EB_SEARCH_CROSS )
					break;

				HitListKeyword(
					search_context, temporary_hit_counts[ i ],
					temporary_hit_lists[ i ], temporary_hit_counts + i );
			}

			*hit_count += more_hit_count;
			if ( max_hit_count <= *hit_count )
				break;
		}
		break;

	case EBSearchCode::EB_SEARCH_MULTI:
		/*
		 * In case of multi search.
		 */
		for ( ;;)
		{
			int search_is_over = 0;

			for ( i = 0; i < EB_MAX_MULTI_ENTRIES; i++ )
			{
				EBSearchContext^ search_context = ParentBook->search_contexts->GetAt( i );
				if ( search_context->code != EBSearchCode::EB_SEARCH_MULTI )
					break;
				EBSearchContext^ temporary_context = ref new EBSearchContext( search_context );

				HitListMulti( temporary_context,
					EB_TMP_MAX_HITS, temporary_hit_lists[ i ],
					temporary_hit_counts + i );

				if ( temporary_hit_counts[ i ] == 0 )
				{
					search_is_over = 1;
					break;
				}
			}
			if ( search_is_over )
				break;

			AndHitLists( hit_list + *hit_count, &more_hit_count,
				max_hit_count - *hit_count, i, temporary_hit_lists,
				temporary_hit_counts );

			for ( i = 0; i < EB_MAX_MULTI_ENTRIES; i++ )
			{
				EBSearchContext^ search_context = ParentBook->search_contexts->GetAt( i );
				if ( search_context->code != EBSearchCode::EB_SEARCH_MULTI )
					break;

				HitListMulti( search_context, temporary_hit_counts[ i ],
					temporary_hit_lists[ i ], temporary_hit_counts + i );
			}

			*hit_count += more_hit_count;
			if ( max_hit_count <= *hit_count )
				break;
		}
		break;

	default:
		/* not reached */
		EBException::Throw( EBErrorCode::EB_ERR_NO_PREV_SEARCH );
	}
}

void EBSubbook::HitListWord( int max_hit_count, EBHit^ *hit_list, int *hit_count )
{
	EBHit^ *hit = hit_list;
	*hit_count = 0;

	EBSearchContext^ context = ParentBook->search_contexts->GetAt( 0 );
	/*
	 * If the result of previous comparison is negative value, all
	 * matched entries have been found.
	 */
	if ( context->comparison_result < 0 || max_hit_count <= 0 ) return;

	for ( ;;)
	{
		/*
		 * Read a page to search, if the page is not on the cache buffer.
		 *
		 * Cache may be missed by the two reasons:
		 *   1. the search process reaches to the end of an index page,
		 *      and tries to read the next page.
		 *   2. Someone else used the cache buffer.
		 *
		 * At the case of 1, the search process reads the page and update
		 * the search context.  At the case of 2. it reads the page but
		 * must not update the context!
		 */
		if ( cache_book_code != ParentBook->code || cache_page != context->page )
		{
			TextZio->LSeek( ( ( off_t ) context->page - 1 ) * EB_SIZE_PAGE, SEEK_SET );
			Array<byte>^ buff = ref new Array<byte>( EB_SIZE_PAGE );
			TextZio->Read( EB_SIZE_PAGE, buff );
			memcpy_s( cache_buffer, EB_SIZE_PAGE, buff->Data, EB_SIZE_PAGE );

			/*
			 * Update search context.
			 */
			if ( context->entry_index == 0 )
			{
				context->page_id = eb_uint1( cache_buffer );
				context->entry_length = eb_uint1( cache_buffer + 1 );
				if ( context->entry_length == 0 )
					context->entry_arrangement = EB_ARRANGE_VARIABLE;
				else
					context->entry_arrangement = EB_ARRANGE_FIXED;
				context->entry_count = eb_uint2( cache_buffer + 2 );
				context->entry_index = 0;
				context->offset = 4;
			}

			cache_book_code = ParentBook->code;
			cache_page = context->page;
		}

		char *cache_p = cache_buffer + context->offset;

		if ( !PAGE_ID_IS_LEAF_LAYER( context->page_id ) )
		{
			/*
			 * Not a leaf index.  It is an error.
			 */
			EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
		}

		if ( !PAGE_ID_HAVE_GROUP_ENTRY( context->page_id )
			&& context->entry_arrangement == EB_ARRANGE_FIXED )
		{
			/*
			 * The leaf index doesn't have a group entry.
			 * Find text and heading locations.
			 */
			while ( context->entry_index < context->entry_count )
			{
				if ( EB_SIZE_PAGE
					< context->offset + context->entry_length + 12 )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
				}

				/*
				 * Compare word and pattern.
				 * If matched, add it to a hit list.
				 */
				context->comparison_result
					= context->compare_single( context->word, cache_p,
						context->entry_length );
				if ( context->comparison_result == 0 )
				{
					*hit = ref new EBHit(
						ref new EBPosition(
							eb_uint4( cache_p + context->entry_length + 6 ),
							eb_uint2( cache_p + context->entry_length + 10 ) ),
						ref new EBPosition(
							eb_uint4( cache_p + context->entry_length ),
							eb_uint2( cache_p + context->entry_length + 4 ) )
						);
					hit++;
					*hit_count += 1;
				}
				context->entry_index++;
				context->offset += context->entry_length + 12;
				cache_p += context->entry_length + 12;

				if ( context->comparison_result < 0 || max_hit_count <= *hit_count )
					return;
			}

		}
		else if ( !PAGE_ID_HAVE_GROUP_ENTRY( context->page_id )
			&& context->entry_arrangement == EB_ARRANGE_VARIABLE )
		{

			/*
			 * The leaf index doesn't have a group entry.
			 * Find text and heading locations.
			 */
			while ( context->entry_index < context->entry_count )
			{
				if ( EB_SIZE_PAGE < context->offset + 1 )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
				}
				context->entry_length = eb_uint1( cache_p );
				if ( EB_SIZE_PAGE
					< context->offset + context->entry_length + 13 )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
				}

				/*
				 * Compare word and pattern.
				 * If matched, add it to a hit list.
				 */
				context->comparison_result
					= context->compare_single( context->word, cache_p + 1,
						context->entry_length );
				if ( context->comparison_result == 0 )
				{
					*hit = ref new EBHit(
						ref new EBPosition(
							eb_uint4( cache_p + context->entry_length + 7 ),
							eb_uint2( cache_p + context->entry_length + 11 ) ),
						ref new EBPosition(
							eb_uint4( cache_p + context->entry_length + 1 ),
							eb_uint2( cache_p + context->entry_length + 5 ) )
						);
					hit++;
					*hit_count += 1;
				}
				context->entry_index++;
				context->offset += context->entry_length + 13;
				cache_p += context->entry_length + 13;

				if ( context->comparison_result < 0 || max_hit_count <= *hit_count ) return;
			}

		}
		else
		{
			/*
			 * The leaf index have a group entry.
			 * Find text and heading locations.
			 */
			while ( context->entry_index < context->entry_count )
			{
				if ( EB_SIZE_PAGE < context->offset + 2 )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
				}
				int group_id = eb_uint1( cache_p );

				if ( group_id == 0x00 )
				{
					/*
					 * 0x00 -- Single entry.
					 */
					context->entry_length = eb_uint1( cache_p + 1 );
					if ( EB_SIZE_PAGE
						< context->offset + context->entry_length + 14 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}

					/*
					 * Compare word and pattern.
					 * If matched, add it to a hit list.
					 */
					context->comparison_result
						= context->compare_single( context->canonicalized_word,
							cache_p + 2, context->entry_length );
					if ( context->comparison_result == 0 )
					{
						*hit = ref new EBHit(
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 8 ),
								eb_uint2( cache_p + context->entry_length + 12 ) ),
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 2 ),
								eb_uint2( cache_p + context->entry_length + 6 ) )
							);
						hit++;
						*hit_count += 1;
					}
					context->in_group_entry = 0;
					context->offset += context->entry_length + 14;
					cache_p += context->entry_length + 14;

				}
				else if ( group_id == 0x80 )
				{
					/*
					 * 0x80 -- Start of group entry.
					 */
					context->entry_length = eb_uint1( cache_p + 1 );
					if ( EB_SIZE_PAGE
						< context->offset + context->entry_length + 4 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}
					context->comparison_result
						= context->compare_single( context->canonicalized_word,
							cache_p + 4, context->entry_length );
					context->in_group_entry = 1;
					cache_p += context->entry_length + 4;
					context->offset += context->entry_length + 4;

				}
				else if ( group_id == 0xc0 )
				{
					/*
					 * Element of the group entry
					 */
					context->entry_length = eb_uint1( cache_p + 1 );
					if ( EB_SIZE_PAGE < context->offset + 14 )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					}

					/*
					 * Compare word and pattern.
					 * If matched, add it to a hit list.
					 */
					if ( context->comparison_result == 0
						&& context->in_group_entry
						&& context->compare_group( context->word, cache_p + 2,
							context->entry_length ) == 0 )
					{
						*hit = ref new EBHit(
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 8 ),
								eb_uint2( cache_p + context->entry_length + 12 ) ),
							ref new EBPosition(
								eb_uint4( cache_p + context->entry_length + 2 ),
								eb_uint2( cache_p + context->entry_length + 6 ) )
							);
						hit++;
						*hit_count += 1;
					}
					context->offset += context->entry_length + 14;
					cache_p += context->entry_length + 14;
				}
				else
				{
					/*
					 * Unknown group ID.
					 */
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
				}

				context->entry_index++;
				if ( context->comparison_result < 0 || max_hit_count <= *hit_count )
					return;
			}
		}

		/*
		 * Go to a next page if available.
		 */
		if ( PAGE_ID_IS_LAYER_END( context->page_id ) )
		{
			context->comparison_result = -1;
			return;
		}
		context->page++;
		context->entry_index = 0;
	}
}

