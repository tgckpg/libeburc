#include "pch.h"
#include "EBSubbook.h"
#include "EBBook.h"

using namespace libeburc;

using namespace std;
using namespace concurrency;
using namespace Platform;
using namespace Microsoft::WRL;
using namespace Windows::Foundation::Collections;

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

EBBook::EBBook( IStorageFolder^ BookDir )
{
	DirRoot = BookDir;
	code = EB_BOOK_NONE;
}

IAsyncOperation<EBBook^>^ EBBook::Parse( IStorageFolder^ BookDir )
{
	return concurrency::create_async([&] {
		EBBook^ Book = ref new EBBook( BookDir );
		Book->Bind();
		return Book;
	});
}

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
		throw ref new NotImplementedException( "EPWing Catalog Loading" );
		// LoadCatalogEBWing( CatalogFile );
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
	byte* NumBooks = ZInst->Read( 16 );
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

	char* space;
	for ( int i = 0; i < subbook_count; i++ )
	{
		EBSubbook^ subbook = ref new EBSubbook( this );
		/*
		 * Read data from the catalog file.
		 */
		byte* buffer = ZInst->Read( EB_SIZE_EB_CATALOG );
		/*
		 * Set a directory name.
		 */
		strncpy_s( subbook->directory_name,
			( ( char* ) buffer ) + 2 + EB_MAX_EB_TITLE_LENGTH,
			EB_MAX_DIRECTORY_NAME_LENGTH );

		subbook->directory_name[ EB_MAX_DIRECTORY_NAME_LENGTH ] = '\0';

		space = strchr( subbook->directory_name, ' ' );

		if ( space != NULL )
			*space = '\0';

		/*
		 * Set a directory name.
		 */
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
		byte* ByteCode = ZInst->Read( 16 );

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

IIterable<EBSubbook^>^ EBBook::Subbooks::get()
{
	return subbooks->GetView();
}