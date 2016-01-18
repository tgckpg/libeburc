#include "pch.h"
#include "EBBook.h"

using namespace libeburc;

using namespace std;
using namespace concurrency;
using namespace Platform;
using namespace Microsoft::WRL;
using namespace Windows::Foundation::Collections;

static EBBookCode BookCounter = 0;

EBBook::EBBook( IStorageFolder^ BookDir )
{
	DirRoot = BookDir;
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
	Code = EB_BOOK_NONE;
	Code = BookCounter++;
	LoadLanguage();
	LoadCatalog();
}

void EBBook::LoadCatalog()
{
	EBErrorCode ErrCode;

	IStorageFile^ CatalogFile;
	DiscCode = EBDiscCode::EB_DISC_INVALID;

	/*
     * Find a catalog file.
     */
	try
	{
		CatalogFile = FileName::eb_find_file_name( DirRoot, L"catalog" );
		DiscCode = EBDiscCode::EB_DISC_EB;
	}
	catch ( Exception^ ex ) { }

	if( DiscCode == EBDiscCode::EB_DISC_INVALID )
	try
	{
		CatalogFile = FileName::eb_find_file_name( DirRoot, L"catalogs" );
		DiscCode = EBDiscCode::EB_DISC_EPWING;
	}
	catch ( Exception^ ex ) { }

	if ( DiscCode == EBDiscCode::EB_DISC_INVALID )
	{
		EBException::Throw( EBErrorCode::EB_ERR_FAIL_OPEN_CAT );
	}

	/*
     * Load the catalog file.
     */
	switch ( DiscCode )
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
	SubbookCount = eb_uint2( NumBooks );

	if ( EB_MAX_SUBBOOKS < SubbookCount )
		SubbookCount = EB_MAX_SUBBOOKS;

	if ( SubbookCount == 0 )
	{
		EBException::Throw( EBErrorCode::EB_ERR_UNEXP_CAT );
	}

	/*
     * -Allocate memories for subbook entries.-
	 * Initialize VectorView for Subbooks
     */
	Vector<EBSubbook^>^ VSubbooks = ref new Vector<EBSubbook^>( SubbookCount );

	char* space;
	for ( int i = 0; i < SubbookCount; i++ )
	{
		EBSubbook^ subbook = ref new EBSubbook();
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
		char title[ EB_MAX_TITLE_LENGTH + 1 ];
		strncpy_s( title, ( ( char* ) buffer ) + 2, EB_MAX_EB_TITLE_LENGTH );

		title[ EB_MAX_EB_TITLE_LENGTH ] = '\0';
		if ( CharCode != EBCharacterCode::EB_CHARCODE_ISO8859_1 )
			JACode::eb_jisx0208_to_euc( title, title );

		wstring wtitle = Utils::ToWStr( title );
		subbook->Title = ref new String( wtitle.c_str() );
		VSubbooks->Append( subbook );
	}

	/*
	 * Fix chachacter-code of the book.
	 */
	FixMislead();

	Subbooks = VSubbooks->GetView();
}

void EBBook::FixMislead()
{
	throw ref new NotImplementedException();
	/*
	const char * const * misleaded;
	EBSubbook^ subbook;
	int i;

	LOG( ( "in: eb_fix_misleaded_book(book=%d)", ( int ) book->code ) );

	for ( misleaded = misleaded_book_table; *misleaded != NULL; misleaded++ ) {
		if ( strcmp( book->subbooks[ 0 ].title, *misleaded ) == 0 ) {
			book->character_code = EB_CHARCODE_JISX0208;
			for ( i = 0, subbook = book->subbooks; i < book->subbook_count;
			i++, subbook++ ) {
				eb_jisx0208_to_euc( subbook->title, subbook->title );
			}
			break;
		}
	}
	*/
}

void EBBook::LoadLanguage()
{
	ZioCode zio_code;
	CharCode = EBCharacterCode::EB_CHARCODE_JISX0208;

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

		CharCode = (EBCharacterCode) eb_uint2(ByteCode);
		if ( CharCode != EBCharacterCode::EB_CHARCODE_ISO8859_1
			&& CharCode != EBCharacterCode::EB_CHARCODE_JISX0208
			&& CharCode != EBCharacterCode::EB_CHARCODE_JISX0208_GB2312 ) {

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