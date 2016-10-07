#include "pch.h"
#include "eburc/Objects/EBHookSet.h"
#include "eburc/Book/EBBook.h"
#include "eburc/Subbook/EBSubbook.h"
#include "eburc/Appendix/EBAppendix.h"
#include "eburc/Appendix/EBAppendixSubbook.h"

using namespace libeburc;
/*
 * EUC JP to ASCII conversion table.
 */
#define EUC_TO_ASCII_TABLE_START	0xa0
#define EUC_TO_ASCII_TABLE_END		0xff

static const unsigned char euc_a1_to_ascii_table[] = {
    0x00, 0x20, 0x00, 0x00, 0x2c, 0x2e, 0x00, 0x3a,     /* 0xa0 */
    0x3b, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x60, 0x00,     /* 0xa8 */
    0x5e, 0x7e, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x2f,     /* 0xb8 */
    0x5c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x27,     /* 0xc0 */
    0x00, 0x22, 0x28, 0x29, 0x00, 0x00, 0x5b, 0x5d,     /* 0xc8 */
    0x7b, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd0 */
    0x00, 0x00, 0x00, 0x00, 0x2b, 0x2d, 0x00, 0x00,     /* 0xd8 */
    0x00, 0x3d, 0x00, 0x3c, 0x3e, 0x00, 0x00, 0x00,     /* 0xe0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c,     /* 0xe8 */
    0x24, 0x00, 0x00, 0x25, 0x23, 0x26, 0x2a, 0x40,     /* 0xf0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xf8 */
};

static const unsigned char euc_a3_to_ascii_table[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa8 */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,     /* 0xb0 */
    0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb8 */
    0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,     /* 0xc0 */
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,     /* 0xc8 */
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,     /* 0xd0 */
    0x58, 0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd8 */
    0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,     /* 0xe0 */
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,     /* 0xe8 */
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,     /* 0xf0 */
    0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xf8 */
};



/*
 * Hook for narrow local character.
 */
void eb_hook_narrow_character_text( EBSubbook^ subbook, EBAppendixSubbook^ appendix,
	void *container, EBHookCode hook_code, int argc,
	const unsigned int *argv )
{
	char alt_text[ EB_MAX_ALTERNATION_TEXT_LENGTH + 1 ];

	if ( appendix == nullptr )
	{
		const char * bytes = subbook->EBHMap->Get( argv[ 0 ] );
		if ( bytes == nullptr )
		{
			subbook->ParentBook->WriteTextByte2( '?', '\0' );
		}
		else
		{
			subbook->ParentBook->WriteText( bytes, subbook->EBHMap->Size( argv[ 0 ] ) );
		}
	}
	else
	{
		appendix->NarrowAltCharText( ( int ) argv[ 0 ], alt_text );
		subbook->ParentBook->WriteTextString( alt_text );
	}
}


/*
 * Hook for wide local character.
 */
void eb_hook_wide_character_text( EBSubbook^ subbook, EBAppendixSubbook^ appendix,
	void *container, EBHookCode hook_code, int argc,
	const unsigned int *argv )
{
	char alt_text[ EB_MAX_ALTERNATION_TEXT_LENGTH + 1 ];

	if ( appendix == nullptr )
	{
		const char * bytes = subbook->EBZMap->Get( argv[ 0 ] );
		if ( bytes == nullptr )
		{
			subbook->ParentBook->WriteTextByte2( '?', '\0' );
		}
		else
		{
			subbook->ParentBook->WriteText( bytes, subbook->EBZMap->Size( argv[ 0 ] ) );
		}
	}
	else
	{
		appendix->WideAltCharText( ( int ) argv[ 0 ], alt_text );
		subbook->ParentBook->WriteTextString( alt_text );
	}
}

void eb_hook_newline(EBSubbook^ subbook, EBAppendixSubbook^ appendix, void *container,
    EBHookCode code, int argc, const unsigned int *argv)
{
	subbook->ParentBook->WriteTextByte2( '\n', '\0' );
}

void eb_hook_gb2312( EBSubbook^ subbook, EBAppendixSubbook^ appendix, void *container,
	EBHookCode hook_code, int argc, const unsigned int *argv )
{
	byte gb_char[] { argv[ 0 ] >> 8 & 0xFF, argv[ 0 ] & 0xFF, 0 };
	size_t Size = MultiByteToWideChar( CP_GB2312, 0, ( LPCSTR ) gb_char, -1, NULL, 0 );

	byte buff16[ 4 ];
	MultiByteToWideChar( CP_GB2312, 0, ( LPCSTR ) gb_char, -1, ( LPWSTR ) buff16, Size );

	subbook->ParentBook->WriteText( ( const char * ) buff16, Size );
}

void eb_hook_eucjp( EBSubbook^ subbook, EBAppendixSubbook^ appendix, void *container,
	EBHookCode hook_code, int argc, const unsigned int *argv )
{
	byte euc_char[] { argv[ 0 ] >> 8 & 0xFF, argv[ 0 ] & 0xFF, 0 };
	size_t Size = MultiByteToWideChar( CP_EUCJP, 0, ( LPCSTR ) euc_char, -1, NULL, 0 );

	byte buff16[ 4 ];
	MultiByteToWideChar( CP_EUCJP, 0, ( LPCSTR ) euc_char, -1, ( LPWSTR ) buff16, Size );

	subbook->ParentBook->WriteText( ( const char * ) buff16, Size );
}

/*
 * Hook which does nothing.
 */
void eb_hook_empty(EBSubbook^ subbook, EBAppendixSubbook^ appendix, void *container,
    EBHookCode hook_code, int argc, const unsigned int *argv)
{
}

void EBHookSet::BindDefaultHooks()
{
	hooks[ ( int ) EBHookCode::EB_HOOK_NARROW_JISX0208 ]->function = eb_hook_eucjp;
	hooks[ ( int ) EBHookCode::EB_HOOK_WIDE_JISX0208 ]->function = eb_hook_eucjp;
	hooks[ ( int ) EBHookCode::EB_HOOK_NARROW_FONT ]->function = eb_hook_narrow_character_text;
	hooks[ ( int ) EBHookCode::EB_HOOK_WIDE_FONT ]->function = eb_hook_wide_character_text;
	hooks[ ( int ) EBHookCode::EB_HOOK_NEWLINE ]->function = eb_hook_newline;
	hooks[ ( int ) EBHookCode::EB_HOOK_GB2312 ]->function = eb_hook_gb2312;
}

