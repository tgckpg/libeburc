#include "pch.h"
#include "EBAppendixSubbook.h"
#include "EBAppendix.h"

using namespace libeburc;

using namespace Platform;

/*
 * Hash macro for cache data.
 */
#define EB_HASH_ALT_CACHE(c)	((c) & 0x0f)
#define MAX_ALT_LEN1 EB_MAX_ALTERNATION_TEXT_LENGTH + 1

void EBAppendixSubbook::WideAltCharText( int character_number, char *text )
{
	/*
	 * The wide font must exist in the current subbook.
	 */
	if ( wide_page == 0 )
	{
		EBException::Throw( EBErrorCode::EB_ERR_NO_ALT );
	}

	if ( character_code == EBCharCode::EB_CHARCODE_ISO8859_1 )
	{
		WideCharTextLatin( character_number, text );
	}
	else
	{
		WideCharTextJIS( character_number, text );
	}
}

void EBAppendixSubbook::WideCharTextJIS( int character_number, char *text )
{
	int start = wide_start;
	int end = wide_end;

	/*
	 * Check for `character_number'.  Is it in a font?
	 * This test works correctly even when the font doesn't exist in
	 * the current subbook because `start' and `end' have set to -1
	 * in the case.
	 */
	if ( character_number < start
		|| end < character_number
		|| ( character_number & 0xff ) < 0x21
		|| 0x7e < ( character_number & 0xff ) )
	{
		EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_CHAR_TEXT );
	}

	/*
	 * Calculate the location of alternation data.
	 */
	off_t location = ( wide_page - 1 ) * EB_SIZE_PAGE
		+ ( ( ( character_number >> 8 ) - ( start >> 8 ) ) * 0x5e
			+ ( character_number & 0xff ) - ( start & 0xff ) )
		* MAX_ALT_LEN1;

	/*
	 * Check for the cache data.
	 */
	EBAlternationCache^ cachep = ParentBook->AcquireWideCache( EB_HASH_ALT_CACHE( character_number ) );
	if ( cachep->character_number == character_number )
	{
		memcpy_s( text, MAX_ALT_LEN1, cachep->text->Data, MAX_ALT_LEN1 );
		return;
	}

	/*
	 * Read the alternation data.
	 */
	zio->LSeek( location, SEEK_SET );
	zio->Read( MAX_ALT_LEN1, cachep->text );

	/*
	 * Update cache data.
	 */
	memcpy_s( text, MAX_ALT_LEN1, cachep->text->Data, MAX_ALT_LEN1 );
	cachep->text[ EB_MAX_ALTERNATION_TEXT_LENGTH ] = '\0';
	cachep->character_number = character_number;
}

void EBAppendixSubbook::WideCharTextLatin( int character_number, char *text )
{
	int start = wide_start;
	int end = wide_end;

	/*
	 * Check for `character_number'.  Is it in a font?
	 * This test works correctly even when the font doesn't exist in
	 * the current subbook because `start' and `end' have set to -1
	 * in the case.
	 */
	if ( character_number < start
		|| end < character_number
		|| ( character_number & 0xff ) < 0x01
		|| 0xfe < ( character_number & 0xff ) )
	{
		EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_CHAR_TEXT );
	}

	/*
	 * Calculate the location of alternation data.
	 */
	off_t location = ( wide_page - 1 ) * EB_SIZE_PAGE
		+ ( ( ( character_number >> 8 ) - ( start >> 8 ) ) * 0xfe
			+ ( character_number & 0xff ) - ( start & 0xff ) )
		* ( MAX_ALT_LEN1 );

	/*
	 * Check for the cache data.
	 */
	EBAlternationCache^ cache_p = ParentBook->AcquireWideCache( EB_HASH_ALT_CACHE( character_number ) );
	if ( cache_p->character_number == character_number )
	{
		memcpy_s( text, MAX_ALT_LEN1, cache_p->text->Data, MAX_ALT_LEN1 );
		return;
	}

	/*
	 * Read the alternation data.
	 */
	zio->LSeek( location, SEEK_SET );
	zio->Read( MAX_ALT_LEN1, cache_p->text );

	/*
	 * Update cache data.
	 */
	memcpy_s( text, MAX_ALT_LEN1, cache_p->text->Data, MAX_ALT_LEN1 );
	cache_p->text[ EB_MAX_ALTERNATION_TEXT_LENGTH ] = '\0';
	cache_p->character_number = character_number;
}