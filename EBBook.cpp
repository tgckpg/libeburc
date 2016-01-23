#include "pch.h"
#include "EBSubbook.h"
#include "EBBook.h"

using namespace libeburc;

using namespace std;
using namespace Platform;

static EBBookCode BookCounter = 0;

/*
 * There are some books that EB Library sets wrong character code of
 * the book.  They are written in JIS X 0208, but the library sets
 * ISO 8859-1.
 *
 * We fix the character of the books.  The following table lists
 * titles of the first subbook in those books.
 */
static const char * const misleaded_book_table[] = {
    /* SONY DataDiskMan (DD-DR1) accessories. */
    "%;%s%A%e%j!\\%S%8%M%9!\\%/%i%&%s",

    /* Shin Eiwa Waei Chujiten (earliest edition) */
    "8&5f<R!!?71QOBCf<-E5",

    /* EB Kagakugijutsu Yougo Daijiten (YRRS-048) */
    "#E#B2J3X5;=QMQ8lBg<-E5",

    /* Nichi-Ei-Futsu Jiten (YRRS-059) */
    "#E#N#G!?#J#A#N!J!\\#F#R#E!K",

    /* Japanese-English-Spanish Jiten (YRRS-060) */
    "#E#N#G!?#J#A#N!J!\\#S#P#A!K",

     /* Panasonic KX-EBP2 accessories. */
    "%W%m%7!<%I1QOB!&OB1Q<-E5",
    NULL
};

void EBBook::Bind()
{
	code = BookCounter++;
	LoadLanguage();
	LoadCatalog();
}

void EBBook::LoadCatalog()
{
	EBErrorCode ErrCode;

	IStorageFile^ CatalogFile;
	disc_code = EBDiscCode::EB_DISC_INVALID;

	/*
     * Find a catalog file.
     */
	try
	{
		CatalogFile = FileName::eb_find_file_name( DirRoot, L"catalog" );
		disc_code = EBDiscCode::EB_DISC_EB;
	}
	catch ( Exception^ ex ) { }

	if( disc_code == EBDiscCode::EB_DISC_INVALID )
	try
	{
		CatalogFile = FileName::eb_find_file_name( DirRoot, L"catalogs" );
		disc_code = EBDiscCode::EB_DISC_EPWING;
	}
	catch ( Exception^ ex ) { }

	if ( disc_code == EBDiscCode::EB_DISC_INVALID )
	{
		EBException::Throw( EBErrorCode::EB_ERR_FAIL_OPEN_CAT );
	}

	/*
     * Load the catalog file.
     */
	switch ( disc_code )
	{
	case EBDiscCode::EB_DISC_EB:
		LoadCatalogEB( CatalogFile );
		break;
	case EBDiscCode::EB_DISC_EPWING:
		LoadCatalogEPWING( CatalogFile );
		break;
	}
}

void EBBook::LoadCatalogEB( IStorageFile^ File )
{
	ZioCode ZCode = FileName::eb_path_name_zio_code( File, ZioCode::ZIO_PLAIN );

	Zio^ ZInst = ref new Zio( File, ZCode );

	/*
	 * Get the number of subbooks in this book.
	 */
	Array<byte>^ buff = ref new Array<byte>( 16 );
	ZInst->Read( 16, buff );
	byte* NumBooks = buff->Data;
	subbook_count = eb_uint2( NumBooks );

	if ( EB_MAX_SUBBOOKS < subbook_count )
		subbook_count = EB_MAX_SUBBOOKS;

	if ( subbook_count == 0 )
	{
		EBException::Throw( EBErrorCode::EB_ERR_UNEXP_CAT );
	}

	/*
     * -Allocate memories for subbook entries.-
	 * Initialize VectorView for Subbooks
     */
	subbooks = ref new Vector<EBSubbook^>( subbook_count );

	for ( int i = 0; i < subbook_count; i++ )
	{
		EBSubbook^ subbook = ref new EBSubbook( this );
		/*
		 * Read data from the catalog file.
		 */
		Array<byte>^ buff = ref new Array<byte>( EB_SIZE_EB_CATALOG );
		ZInst->Read( EB_SIZE_EB_CATALOG, buff );
		byte* buffer = buff->Data;
		/*
		 * Set a directory name.
		 */
		strncpy_s( subbook->directory_name,
			( ( char* ) buffer ) + 2 + EB_MAX_EB_TITLE_LENGTH,
			EB_MAX_DIRECTORY_NAME_LENGTH );

		subbook->directory_name[ EB_MAX_DIRECTORY_NAME_LENGTH ] = '\0';

		char *space = strchr( subbook->directory_name, ' ' );
		if ( space != NULL ) *space = '\0';

		IStorageFolder^ Folder = FileName::eb_fix_directory( DirRoot, Utils::ToWStr( subbook->directory_name ) );
		subbook->DirRoot = Folder;

		/*
		 * Set an index page.
		 */
		subbook->IndexPage = 1;

		/*
		 * Set a title.  (Convert from JISX0208 to EUC JP)
		 */
		strncpy_s( subbook->title, ( ( char* ) buffer ) + 2, EB_MAX_EB_TITLE_LENGTH );

		subbook->title[ EB_MAX_EB_TITLE_LENGTH ] = '\0';
		if ( character_code != EBCharCode::EB_CHARCODE_ISO8859_1 )
			JACode::eb_jisx0208_to_euc( subbook->title, subbook->title );

		// Use set instead of append
		subbooks->SetAt( i, subbook );
	}

	/*
	 * Fix chachacter-code of the book.
	 */
	FixMislead();
}

void EBBook::LoadCatalogEPWING( IStorageFile^ File )
{

	/*
	 * Open a catalog file.
	 */
	ZioCode zio_code = FileName::eb_path_name_zio_code( File, ZioCode::ZIO_PLAIN );
	Zio^ zio = ref new Zio( File, zio_code );

	/*
	 * Get the number of subbooks in this book.
	 */
	Array<byte>^ buff = ref new Array<byte>( 16 );
	zio->Read( 16, buff );
	byte* buffer = buff->Data;


	subbook_count = eb_uint2( buffer );
	if ( EB_MAX_SUBBOOKS < subbook_count )
		subbook_count = EB_MAX_SUBBOOKS;

	if ( subbook_count == 0 )
	{
		EBException::Throw( EBErrorCode::EB_ERR_UNEXP_CAT );
	}

	int epwing_version = eb_uint2( buffer + 2 );

	if ( epwing_version >= 10 )
		character_code = EBCharCode::EB_CHARCODE_UTF8;

	/*
	 * Allocate memories for subbook entries.
	 */
	subbooks = ref new Vector<EBSubbook^>( subbook_count );

	/*
	 * Read information about subbook.
	 */
	for ( int i = 0; i < subbook_count; i++ )
	{
		EBSubbook^ subbook = ref new EBSubbook( this );
		/*
		 * Read data from the catalog file.
		 */
		Array<byte>^ buff = ref new Array<byte>( EB_SIZE_EPWING_CATALOG );
		zio->Read( EB_SIZE_EPWING_CATALOG, buff );
		char* buffer = ( char* ) buff->Data;

		/*
		 * Set a directory name.
		 */
		strncpy_s( subbook->directory_name,
			buffer + 2 + EB_MAX_EPWING_TITLE_LENGTH,
			EB_MAX_DIRECTORY_NAME_LENGTH );
		subbook->directory_name[ EB_MAX_DIRECTORY_NAME_LENGTH ] = '\0';

		char *space = strchr( subbook->directory_name, ' ' );
		if ( space != NULL ) *space = '\0';

		IStorageFolder^ Folder = FileName::eb_fix_directory( DirRoot, Utils::ToWStr( subbook->directory_name ) );
		subbook->DirRoot = Folder;

		/*
		 * Set an index page.
		 */
		subbook->IndexPage = eb_uint2( buffer + 2 + EB_MAX_EPWING_TITLE_LENGTH
			+ EB_MAX_DIRECTORY_NAME_LENGTH + 4 );

		/*
		 * Set a title.  (Convert from JISX0208 to EUC JP)
		 */
		strncpy_s( subbook->title, ( char* ) buffer + 2, EB_MAX_EPWING_TITLE_LENGTH );
		subbook->title[ EB_MAX_EPWING_TITLE_LENGTH ] = '\0';

		if ( character_code != EBCharCode::EB_CHARCODE_ISO8859_1 )
			JACode::eb_jisx0208_to_euc( subbook->title, subbook->title );

		/*
		 * Narrow font file names.
		 */
		char *buffer_p = ( char* ) buffer + 2 + EB_MAX_EPWING_TITLE_LENGTH + 50;
		for ( int j = 0; j < EB_MAX_FONTS; j++ )
		{
			/*
			 * Skip this entry if the first character of the file name
			 * is not valid.
			 */
			if ( *buffer_p == '\0' || 0x80 <= *( ( unsigned char * ) buffer_p ) )
			{
				buffer_p += EB_MAX_DIRECTORY_NAME_LENGTH;
				continue;
			}

			EBFont^ font = ref new EBNarrowFont( subbook );
			strncpy_s( font->file_name, buffer_p, EB_MAX_DIRECTORY_NAME_LENGTH );
			font->file_name[ EB_MAX_DIRECTORY_NAME_LENGTH ] = '\0';
			font->font_code = j;
			font->page = 1;
			space = strchr( font->file_name, ' ' );
			if ( space != NULL )
				*space = '\0';
			buffer_p += EB_MAX_DIRECTORY_NAME_LENGTH;

			subbook->narrow_fonts[ j ] = font;
		}

		/*
		 * Wide font file names.
		 */
		buffer_p = buffer + 2 + EB_MAX_EPWING_TITLE_LENGTH + 18;
		for ( int j = 0; j < EB_MAX_FONTS; j++ )
		{
			/*
			 * Skip this entry if the first character of the file name
			 * is not valid.
			 */
			if ( *buffer_p == '\0' || 0x80 <= *( ( unsigned char * ) buffer_p ) )
			{
				buffer_p += EB_MAX_DIRECTORY_NAME_LENGTH;
				continue;
			}

			EBFont^ font = ref new EBWideFont( subbook );
			strncpy_s( font->file_name, buffer_p, EB_MAX_DIRECTORY_NAME_LENGTH );
			font->file_name[ EB_MAX_DIRECTORY_NAME_LENGTH ] = '\0';
			font->font_code = j;
			font->page = 1;
			space = strchr( font->file_name, ' ' );
			if ( space != NULL )
				*space = '\0';
			buffer_p += EB_MAX_DIRECTORY_NAME_LENGTH;

			subbook->wide_fonts[ j ] = font;
		}

		subbook->initialized = 0;
		subbook->code = i;

		/*
		 * Set default file names and compression types.
		 */
		strcpy_s( subbook->text_file_name, EB_FILE_NAME_HONMON );
		strcpy_s( subbook->graphic_file_name, EB_FILE_NAME_HONMON );
		strcpy_s( subbook->sound_file_name, EB_FILE_NAME_HONMON );
		subbook->text_hint_zio_code = ZioCode::ZIO_PLAIN;
		subbook->graphic_hint_zio_code = ZioCode::ZIO_PLAIN;
		subbook->sound_hint_zio_code = ZioCode::ZIO_PLAIN;

		subbooks->SetAt( i, subbook );
	}

	if ( epwing_version == 1 ) return;

	/*
	 * Read extra information about subbook.
	  */
	for ( int i = 0; i < subbook_count; i++ )
	{
		EBSubbook^ subbook = subbooks->GetAt( i );
		/*
		 * Read data from the catalog file.
		 *
		 * We don't complain about unexpected EOF.  In that case, we
		 * return EB_SUCCESS.
		 */
		Array<byte>^ buff = ref new Array<byte>( EB_SIZE_EPWING_CATALOG );
		zio->Read( EB_SIZE_EPWING_CATALOG, buff );
		char *buffer = ( char* ) buff->Data;
		
		if ( *( buffer + 4 ) == '\0' )
			continue;

		/*
		 * Set a text file name and its compression hint.
		 */
		strncpy_s( subbook->text_file_name, buffer + 4, EB_MAX_DIRECTORY_NAME_LENGTH );
		subbook->text_file_name[ EB_MAX_DIRECTORY_NAME_LENGTH ] = '\0';

		char* space = strchr( subbook->text_file_name, ' ' );
		if ( space != NULL ) *space = '\0';

		subbook->text_hint_zio_code = Zio::Hint( eb_uint1( buffer + 55 ) );
		if ( subbook->text_hint_zio_code == ZioCode::ZIO_INVALID )
		{
			EBException::Throw( EBErrorCode::EB_ERR_UNEXP_CAT );
		}

		int data_types = eb_uint2( buffer + 41 );

		/*
		 * Set a graphic file name and its compression hint.
		 */
		*( subbook->graphic_file_name ) = '\0';
		if ( ( data_types & 0x03 ) == 0x02 )
		{
			strncpy_s( subbook->graphic_file_name, buffer + 44, EB_MAX_DIRECTORY_NAME_LENGTH );
			subbook->graphic_hint_zio_code = Zio::Hint( eb_uint1( buffer + 54 ) );
		}
		else if ( ( ( data_types >> 8 ) & 0x03 ) == 0x02 )
		{
			strncpy_s( subbook->graphic_file_name, buffer + 56, EB_MAX_DIRECTORY_NAME_LENGTH );
			subbook->graphic_hint_zio_code = Zio::Hint( eb_uint1( buffer + 53 ) );
		}
		subbook->graphic_file_name[ EB_MAX_DIRECTORY_NAME_LENGTH ] = '\0';

		space = strchr( subbook->graphic_file_name, ' ' );
		if ( space != NULL ) *space = '\0';

		if ( *( subbook->graphic_file_name ) == '\0' )
		{
			strcpy_s( subbook->graphic_file_name, subbook->text_file_name );
			subbook->graphic_hint_zio_code = subbook->text_hint_zio_code;
		}

		if ( subbook->graphic_hint_zio_code == ZioCode::ZIO_INVALID )
		{
			EBException::Throw( EBErrorCode::EB_ERR_UNEXP_CAT );
		}

		/*
		 * Set a sound file name and its compression hint.
		 */
		*( subbook->sound_file_name ) = '\0';
		if ( ( data_types & 0x03 ) == 0x01 )
		{
			strncpy_s( subbook->sound_file_name, buffer + 44, EB_MAX_DIRECTORY_NAME_LENGTH );
			subbook->sound_hint_zio_code = Zio::Hint( eb_uint1( buffer + 54 ) );
		}
		else if ( ( ( data_types >> 8 ) & 0x03 ) == 0x01 )
		{
			strncpy_s( subbook->sound_file_name, buffer + 56, EB_MAX_DIRECTORY_NAME_LENGTH );
			subbook->sound_hint_zio_code = Zio::Hint( eb_uint1( buffer + 53 ) );
		}
		subbook->sound_file_name[ EB_MAX_DIRECTORY_NAME_LENGTH ] = '\0';

		space = strchr( subbook->sound_file_name, ' ' );
		if ( space != NULL ) *space = '\0';

		if ( *( subbook->sound_file_name ) == '\0' )
		{
			strcpy_s( subbook->sound_file_name, subbook->text_file_name );
			subbook->sound_hint_zio_code = subbook->text_hint_zio_code;
		}

		if ( subbook->sound_hint_zio_code == ZioCode::ZIO_INVALID )
		{
			EBException::Throw( EBErrorCode::EB_ERR_UNEXP_CAT );
		}
	}
}

void EBBook::FixMislead()
{
	const char * const * misleaded;
	EBSubbook^ subbook = subbooks->GetAt( 0 );

	for ( misleaded = misleaded_book_table; *misleaded != NULL; misleaded++ )
	{
		if ( strcmp( subbook->title, *misleaded ) == 0 )
		{
			character_code = EBCharCode::EB_CHARCODE_JISX0208;

			for_each( begin( subbooks ), end( subbooks ), [] ( EBSubbook^ subbook )
			{
				JACode::eb_jisx0208_to_euc( subbook->title, subbook->title );
			} );

			break;
		}
	}
}

void EBBook::LoadLanguage()
{
	ZioCode zio_code;
	character_code = EBCharCode::EB_CHARCODE_JISX0208;

	/*
	 * Open the language file.
	 */
	try
	{
		IStorageFile^ language_file = FileName::eb_find_file_name( DirRoot, L"language" );

		ZioCode ZCode = FileName::eb_path_name_zio_code( language_file, ZioCode::ZIO_PLAIN );

		Zio^ ZInst = ref new Zio( language_file, ZCode );

		if ( ZInst->Code == ZioCode::ZIO_INVALID )
			throw ref new COMException( -1, "Invalid Zio" );

		/*
		 * Get a character code of the book, and get the number of langueages
		 * in the file.
		 */
		Array<byte>^ buff = ref new Array<byte>( 16 );
		ZInst->Read( 16, buff );
		byte* ByteCode = buff->Data;

		character_code = (EBCharCode) eb_uint2(ByteCode);
		if ( character_code != EBCharCode::EB_CHARCODE_ISO8859_1
			&& character_code != EBCharCode::EB_CHARCODE_JISX0208
			&& character_code != EBCharCode::EB_CHARCODE_JISX0208_GB2312 ) {

			throw ref new COMException( -1, "Invalid charactor code, assuming JISX0208" );
		}
	}
	catch ( Exception^ ex )
	{
		/*
		 * An error occurs...
		 */
	}
}

void EBBook::ResetSearchContext()
{
	search_contexts = ref new Vector<EBSearchContext^>();
	search_contexts->Append( ref new EBSearchContext() );
}

void EBBook::ResetBinaryContext()
{
	binary_context = ref new EBBinaryContext();
}

void EBBook::ResetTextContext()
{
	if( !text_context )
		text_context = ref new EBTextContext();
	else text_context->Reset();
}

void EBBook::InvalidateTextContext()
{
	ResetTextContext();
	text_context->code = EBTextCode::EB_TEXT_INVALID;
}