#include "pch.h"
#include "EBBook.h"

using namespace libeburc;

using namespace std;
using namespace concurrency;
using namespace Platform;
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
}

void EBBook::LoadLanguage()
{
	Zio^ zio = ref new Zio();
	ZioCode zio_code;
	char language_path_name[ EB_MAX_PATH_LENGTH + 1 ];
	char language_file_name[ EB_MAX_FILE_NAME_LENGTH + 1 ];
	char buffer[ 16 ];

	CharCode = EBCharacterCode::EB_CHARCODE_JISX0208;

	/*
	 * Open the language file.
	 */
	try
	{
		StorageFile^ language_file = FileName::eb_find_file_name( DirRoot, L"language" );

		ZioCode ZCode = FileName::eb_path_name_zio_code( language_file, ZioCode::ZIO_PLAIN );

		IAsyncOperation<Zio^>^ Ziop = Zio::Open( language_file, ZCode );
		/*
		if ( zio_open( &zio, language_path_name, zio_code ) < 0 )
			goto failed;
		*/

			/*
			 * Get a character code of the book, and get the number of langueages
			 * in the file.
			if (zio_read(&zio, buffer, 16) != 16)
				goto failed;

			book->character_code = eb_uint2(buffer);
			if (book->character_code != EB_CHARCODE_ISO8859_1
				&& book->character_code != EB_CHARCODE_JISX0208
				&& book->character_code != EB_CHARCODE_JISX0208_GB2312) {
				goto failed;
			}

			zio_close(&zio);
		 */

	}
	catch ( Exception^ ex )
	{
		/*
		 * An error occurs...
		 */

	}
}