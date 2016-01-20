#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

EBSubbook::EBSubbook( EBBook^ Book )
{
	ParentBook = Book;
}


IAsyncAction^ EBSubbook::OpenAsync()
{
	return create_async( [ & ] { SetAuto(); } );
}

void EBSubbook::SetAuto()
{
	/*
     * Dispatch.
     */
	if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
	{
		SetEB();
	}
	else
	{
		SetEPWING();
	}

	/*
     * Load the subbook.
     */
	Load();
	/*
     * Load font files.
     */
    LoadFontHeaders();
}

void EBSubbook::Load()
{
	/*
     * Reset contexts.
     */
	ParentBook->ResetSearchContext();
	ParentBook->ResetTextContext();
	ParentBook->ResetBinaryContext();

	/*
	 * If the subbook has already initialized, return immediately.
	 */
	if ( initialized ) return;

	if ( TextZio->Code != ZioCode::ZIO_INVALID )
	{
		/*
		 * Read index information.
		 */
		LoadIndexes();

		/*
		 * Read mutli search information.
		 */
		LoadMultiSearches();
		LoadMultiTitles();

		/*
		 * Rewind the file descriptor of the start file.
		 */
		TextZio->LSeekRaw( ( ( off_t ) IndexPage - 1 ) * EB_SIZE_PAGE );
	}
}


void EBSubbook::LoadIndexes()
{
	/*
     * Read the index table in the subbook.
     */
	TextZio->LSeekRaw( ( ( off_t ) IndexPage - 1 ) * EB_SIZE_PAGE );
	String^ str = TextZio->SrcFile->Name;
	byte* buffer = TextZio->Read( EB_SIZE_PAGE );

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
		TextZio->LSeekRaw( ( ( off_t ) multi->search->start_page - 1 ) * EB_SIZE_PAGE );
		byte* buffer = TextZio->Read( EB_SIZE_PAGE );

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
			EBSearch^ entry = multi->entries[ j ];
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
	TextZio->LSeekRaw( ( ( off_t ) search_title_page - 1 ) * EB_SIZE_PAGE );
	byte* buffer = TextZio->Read( EB_SIZE_PAGE );

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

void EBSubbook::SetEPWING()
{
	throw ref new NotImplementedException( "Subbook SetEPWing is not impl yet" );
}

void EBSubbook::SetEB()
{
	/*
	 * Open a text file if exists.
	 */
	ZioCode TextZCode = ZioCode::ZIO_INVALID;

	if ( initialized )
	{
		if ( TextZio->Code != ZioCode::ZIO_INVALID )
			TextZCode = ZioCode::ZIO_REOPEN;
	}
	else
	{
		try
		{
			TextFile = FileName::eb_find_file_name( DirRoot, EB_FILE_NAME_START );
			TextZCode = FileName::eb_path_name_zio_code( TextFile, ZioCode::ZIO_PLAIN );
		}
		catch ( Exception^ ex )
		{

		}
	}

	if ( TextZCode != ZioCode::ZIO_INVALID )
	{
		TextZio = ref new Zio( TextFile, TextZCode );
		TextZCode = TextZio->Code;
	}

	IStorageFile^ GraphicFile;
	/*
	 * Open a graphic file if exists.
	 */
	ZioCode GraphZCode = ZioCode::ZIO_INVALID;
	if ( initialized )
	{
		if ( GraphicZio->Code != ZioCode::ZIO_INVALID )
			GraphZCode = ZioCode::ZIO_REOPEN;
	}
	else if ( TextZCode != ZioCode::ZIO_INVALID )
	{
		GraphicFile = TextFile;
		GraphZCode = TextZCode;
	}

	/* The following code does not make sense
	 * Before the Subbook is initialize, the graph_file_name
	 * can only be initialized via text_file_name
	 * and there were no handling for last ELSE case for graphic file
	if ( graphic_zio_code != ZIO_INVALID )
	{
		eb_compose_path_name2( book->path, subbook->directory_name,
			subbook->graphic_file_name, graphic_path_name );
		if ( zio_open( &subbook->graphic_zio, graphic_path_name,
			graphic_zio_code ) < 0 )
		{
			error_code = EB_ERR_FAIL_OPEN_BINARY;
			goto failed;
		}
		graphic_zio_code = zio_mode( &subbook->graphic_zio );
	}
	* Since we are using file reference now
	* we just use the GraphicFile ( i.e. TextFile ) instead
	* Notice GrapZCode can only be valid where TextFile is valid
	*/
	if ( GraphZCode != ZioCode::ZIO_INVALID )
	{
		GraphicZio = ref new Zio( GraphicFile, GraphZCode );
	}
}

void EBSubbook::LoadUTF8Table()
{
	/*
	 * If table is already loaded, do nothing.
	 */
	if ( table_buffer ) return;

	/*
	 * Read normalization table.
	 */
	size_t buffer_size = table_size * EB_SIZE_PAGE;
	TextZio->LSeekRaw( ( ( off_t ) table_page - 1 ) * EB_SIZE_PAGE );
	table_buffer = TextZio->Read( buffer_size );

	byte* buffer = table_buffer;
	/*
	 * Check version and etc.
	 */
	if ( eb_uint4( buffer ) != 1 )
	{
		EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
	}

	table_count = eb_uint4( buffer + 4 );
	if ( table_count > 0x10ffff )
	{
		EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
	}

	table = ref new Vector<EBUTF8Table^>( table_count );

	char* read = ( char* ) ( buffer + 8 );
	for ( unsigned int i = 0; i < table_count; i++ )
	{
		EBUTF8Table^ zistable = ref new EBUTF8Table();
		read += Utils::ReadUtf8( read, &zistable->code );
		if ( zistable->code == 0 )
		{
			EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
		}
		zistable->string = read;
		read += strlen( read ) + 1;
	}
}

void InitFont( EBFont^ nfont )
{
	if ( !nfont ) return;

	if ( nfont->font_code == EB_FONT_INVALID
		|| nfont->initialized )
		return;

	try
	{
		nfont->Open();
		nfont->LoadHeaders();
	}
	catch ( Exception^ ex )
	{
		nfont->font_code = EB_FONT_INVALID;
	}

	nfont->initialized = 1;
}

void EBSubbook::LoadFontHeaders()
{
	/*
	 * Load narrow font headers.
	 */
	for ( EBFontCode i = 0; i < EB_MAX_FONTS; i++ )
	{
		InitFont( narrow_fonts[ i ] );
	}

	/*
	 * Load wide font header.
	 */
	for ( EBFontCode i = 0; i < EB_MAX_FONTS; i++ )
	{
		InitFont( wide_fonts[ i ] );
	}
}