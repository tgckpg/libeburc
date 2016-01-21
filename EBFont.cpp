#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"
#include "EBFont.h"

using namespace libeburc;

EBFont::EBFont( EBSubbook^ b )
{
	subbook = b;
}

void EBFont::Open()
{
	char font_path_name[ EB_MAX_PATH_LENGTH + 1 ];
	ZioCode zio_code;

	if ( font_code == EB_FONT_INVALID )
	{
		EBException::Throw( EBErrorCode::EB_ERR_FAIL_OPEN_FONT );
	}

	if ( zio && zio->Code != ZioCode::ZIO_INVALID ) return;

	/*
	 * If the book is EBWING, open the narrow font file.
	 * (In EB books, font data are stored in the `START' file.)
	 */
	zio_code = ZioCode::ZIO_INVALID;

	EBBook^ book = subbook->ParentBook;
	if ( book->disc_code == EBDiscCode::EB_DISC_EB )
	{
		if ( initialized )
		{
			if ( zio->Code != ZioCode::ZIO_INVALID )
				zio_code = ZioCode::ZIO_REOPEN;
		}
		else
		{
			zio_code = subbook->TextZio->Code;
		}

		FontFile = subbook->TextFile;
	}
	else
	{
		if ( initialized )
		{
			if ( zio->Code != ZioCode::ZIO_INVALID )
				zio_code = ZioCode::ZIO_REOPEN;
		}
		else
		{
			try
			{
				FontFile = FileName::eb_find_file_name( subbook->GaijiDir, Utils::ToWStr( file_name ) );
				zio_code = FileName::eb_path_name_zio_code( FontFile, ZioCode::ZIO_PLAIN );
			}
			catch ( Exception^ ex )
			{
				EBException::Throw( EBErrorCode::EB_ERR_FAIL_OPEN_FONT );
			}
		}
	}

	zio = ref new Zio( FontFile, zio_code );
	if( zio->Code == ZioCode::ZIO_INVALID )
	{
		EBException::Throw( EBErrorCode::EB_ERR_FAIL_OPEN_FONT );
	}
}

void EBFont::LoadHeaders()
{
	if ( initialized ) return;

	/*
	 * Read information from the text file.
	 */
	zio->LSeekRaw( ( ( off_t ) page - 1 ) * EB_SIZE_PAGE );
	Array<byte>^ buff = ref new Array<byte>( 16 );
	zio->Read( 16, buff );
	byte* buffer = buff->Data;

	/*
	 * If the number of characters (`character_count') is 0, the font
	 * is unavailable.  We return EB_ERR_NO_SUCH_FONT.
	 */
	int character_count = eb_uint2( buffer + 12 );
	if ( character_count == 0 )
	{
		EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_FONT );
	}

	/*
	 * Set the information.
	 */
	EBBook^ book = subbook->ParentBook;
	start = eb_uint2( buffer + 10 );
	if ( book->character_code == EBCharCode::EB_CHARCODE_ISO8859_1 )
	{
		end = start
			+ ( ( character_count / 0xfe ) << 8 ) + ( character_count % 0xfe ) - 1;
		if ( 0xfe < ( end & 0xff ) )
			end += 3;
	}
	else
	{
		end = start
			+ ( ( character_count / 0x5e ) << 8 ) + ( character_count % 0x5e ) - 1;
		if ( 0x7e < ( end & 0xff ) )
			end += 0xa3;
	}

	if ( book->character_code == EBCharCode::EB_CHARCODE_ISO8859_1 )
	{
		if ( ( start & 0xff ) < 0x01
			|| 0xfe < ( start & 0xff )
			|| start < 0x0001
			|| 0x1efe < end )
		{
			EBException::Throw( EBErrorCode::EB_ERR_UNEXP_FONT );
		}
	}
	else
	{
		if ( ( start & 0xff ) < 0x21
			|| 0x7e < ( start & 0xff )
			|| start < 0xa121
			|| 0xfe7e < end )
		{
			EBException::Throw( EBErrorCode::EB_ERR_UNEXP_FONT );
		}
	}
}

int EBFont::FontHeight2( EBFontCode C )
{
	switch ( C )
	{
	case EB_FONT_16:
		return EB_HEIGHT_FONT_16;
	case EB_FONT_24:
		return EB_HEIGHT_FONT_24;
	case EB_FONT_30:
		return EB_HEIGHT_FONT_30;
	case EB_FONT_48:
		return EB_HEIGHT_FONT_48;
	}
	EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_FONT );
}