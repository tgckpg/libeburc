#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

EBSubbook::EBSubbook( EBBook^ Book )
{
	ParentBook = Book;
	word_alphabet = ref new EBSearch();
	word_asis = ref new EBSearch();
	word_kana = ref new EBSearch();
	endword_alphabet = ref new EBSearch();
	endword_asis = ref new EBSearch();
	endword_kana = ref new EBSearch();
	keyword = ref new EBSearch();
	menu = ref new EBSearch();
	image_menu = ref new EBSearch();
	cross = ref new EBSearch();
	copyright = ref new EBSearch();
	text = ref new EBSearch();
	sound = ref new EBSearch();
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

void EBSubbook::SetEPWING()
{
	if ( !initialized )
	{
		try
		{
			/*
			 * Adjust directory names.
			 */
			DataDir = FileName::eb_fix_directory( DirRoot, EB_DIRECTORY_NAME_DATA );
			GaijiDir = FileName::eb_fix_directory( DirRoot, EB_DIRECTORY_NAME_GAIJI );
			MovieDir = FileName::eb_fix_directory( DirRoot, EB_DIRECTORY_NAME_MOVIE );
		}
		catch ( Exception^ ex )
		{

		}
	}

	/*
	 * Open a text file if exists.
	 *
	 * If a subbook has stream data only, its index_page has been set
	 * to 0.  In this case, we must not try to open a text file of
	 * the subbook, since the text file may be for another subbook.
	 * Remember that subbooks can share a `data' sub-directory.
	 */
	ZioCode text_zio_code = ZioCode::ZIO_INVALID;

	if ( initialized )
	{
		if ( TextZio->Code != ZioCode::ZIO_INVALID )
			text_zio_code = ZioCode::ZIO_REOPEN;
	}
	else if ( IndexPage > 0 )
	{
		TextFile = FileName::eb_find_file_name( DataDir, Utils::ToWStr( text_file_name ) );
		text_zio_code = FileName::eb_path_name_zio_code( TextFile, text_zio_code );
	}

	if ( text_zio_code != ZioCode::ZIO_INVALID )
	{
		TextZio = ref new Zio( TextFile, text_zio_code );
		text_zio_code = TextZio->Code;;
	}

	/*
	 * Open a graphic file if exists.
	 */
	ZioCode graphic_zio_code = ZioCode::ZIO_INVALID;

	if ( initialized )
	{
		if ( GraphicZio->Code!= ZioCode::ZIO_INVALID )
			graphic_zio_code = ZioCode::ZIO_REOPEN;
	}
	// text_zio_code in graphzio? Typo in original source?
	else if ( text_zio_code != ZioCode::ZIO_INVALID )
	{
		GraphFile = FileName::eb_find_file_name( DataDir, Utils::ToWStr( graphic_file_name ) );
		graphic_zio_code = FileName::eb_path_name_zio_code( GraphFile, graphic_zio_code );
	}

	if ( graphic_zio_code != ZioCode::ZIO_INVALID )
	{
		GraphicZio = ref new Zio( GraphFile, graphic_zio_code );
		graphic_zio_code = GraphicZio->Code;
	}

	/*
	 * Open a sound file if exists.
	 */
	ZioCode sound_zio_code = ZioCode::ZIO_INVALID;

	if ( initialized )
	{
		if ( SoundZio->Code != ZioCode::ZIO_INVALID )
			sound_zio_code = ZioCode::ZIO_REOPEN;
	}
	// text_zio_code in soundzio? Typo in original source?
	else if ( text_zio_code != ZioCode::ZIO_INVALID )
	{
		SoundFile = FileName::eb_find_file_name( DataDir, Utils::ToWStr( sound_file_name ) );
		sound_zio_code = FileName::eb_path_name_zio_code( SoundFile, sound_zio_code );
	}

	if ( sound_zio_code != ZioCode::ZIO_INVALID )
	{
		SoundZio = ref new Zio( SoundFile, sound_zio_code );
		sound_zio_code = GraphicZio->Code;
	}
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

	Array<byte>^ buff = ref new Array<byte>( buffer_size );
	TextZio->Read( buffer_size, buff );
	table_buffer = buff->Data;

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

