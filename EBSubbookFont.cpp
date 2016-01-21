#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

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

void EBSubbook::FontList( EBFontCode *font_list, int *font_count )
{
	*font_count = 0;
	for ( int i = 0; i < EB_MAX_FONTS; i++ )
	{
		EBFont^ NF = narrow_fonts[ i ];
		EBFont^ WF = wide_fonts[ i ];

		if ( ( NF && NF->font_code != EB_FONT_INVALID )
			|| ( WF && WF->font_code != EB_FONT_INVALID ) )
		{
			*font_list++ = i;
			*font_count += 1;
		}
	}
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

void EBSubbook::SetFont( EBFontCode code )
{
	/*
	 * If the current font is the font with `font_code', return immediately.
	 * Otherwise close the current font and continue.
	 */
	if ( narrow_current )
	{
		if ( narrow_current->font_code == code ) return;

		narrow_current = nullptr;
	}
	if ( wide_current )
	{
		if ( wide_current->font_code == code )
			wide_current = nullptr;
	}

	/*
	 * Set the current font.
	 */
	if ( narrow_fonts[ code ]->font_code != EB_FONT_INVALID )
		narrow_current = narrow_fonts[ code ];
	if ( wide_fonts[ code ]->font_code != EB_FONT_INVALID )
		wide_current = wide_fonts[ code ];

	if ( !( narrow_current || wide_current ) )
	{
		EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_FONT );
	}

	/*
	 * Initialize current font informtaion.
	 */
	if ( narrow_current )
	{
		narrow_current->Open();
	}
	if ( wide_current )
	{
		wide_current->Open();
	}
}

