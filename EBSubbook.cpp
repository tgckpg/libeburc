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
}

void EBSubbook::Load()
{
	/*
     * Reset contexts.
     */
    // eb_reset_search_contexts(book);
    // eb_reset_text_context(book);
    // eb_reset_binary_context(book);

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
				multis[ multi_count++ ]->search = search;
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
				EBFont^ font = ref new EBFont();
				font->page = search->start_page;
				font->font_code = EB_FONT_16;
				wide_fonts[ EB_FONT_16 ] = font;
			}
			break;
		case 0xf2:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBFont();
				font->page = search->start_page;
				font->font_code = EB_FONT_16;
				narrow_fonts[ EB_FONT_16 ] = font;
			}
			break;
		case 0xf3:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBFont();
				font->page = search->start_page;
				font->font_code = EB_FONT_24;
				wide_fonts[ EB_FONT_24 ] = font;
			}
			break;
		case 0xf4:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBFont();
				font->page = search->start_page;
				font->font_code = EB_FONT_24;
				narrow_fonts[ EB_FONT_24 ] = font;
			}
			break;
		case 0xf5:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBFont();
				font->page = search->start_page;
				font->font_code = EB_FONT_30;
				wide_fonts[ EB_FONT_30 ] = font;
			}
			break;
		case 0xf6:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBFont();
				font->page = search->start_page;
				font->font_code = EB_FONT_30;
				narrow_fonts[ EB_FONT_30 ] = font;
			}
			break;
		case 0xf7:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBFont();
				font->page = search->start_page;
				font->font_code = EB_FONT_48;
				wide_fonts[ EB_FONT_48 ] = font;
			}
			break;
		case 0xf8:
			if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
			{
				EBFont^ font = ref new EBFont();
				font->page = search->start_page;
				font->font_code = EB_FONT_48;
				narrow_fonts[ EB_FONT_48 ] = font;
			}
			break;
		case 0xff:
			if ( multi_count < EB_MAX_MULTI_SEARCHES
				&& ParentBook->character_code != EBCharCode::EB_CHARCODE_UTF8 )
			{
				multis[ multi_count++ ]->search = search;
			}
			break;
		}

		// eb_finalize_search( &sebxa_zip_text );
	}
	throw ref new NotImplementedException( "LoadIndexes not impl yet" );
}

void EBSubbook::LoadMultiSearches()
{
	throw ref new NotImplementedException( "LoadMultiSearch not impl yet" );
}

void EBSubbook::LoadMultiTitles()
{
	throw ref new NotImplementedException( "LoadMultiTitle not impl yet" );
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