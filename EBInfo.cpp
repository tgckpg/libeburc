#include "pch.h"
#include "EBInfo.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

EBInfo::EBInfo() { }

String^ EBInfo::DiskType( EBBook^ Book )
{
	wchar_t * HStr = L"EPWING";
	if ( Book->disc_code == EBDiscCode::EB_DISC_EB )
	{
		HStr = L"EB/EBG/EBXA/EBXA-C/S-EBXA";
	}

	return ref new String( HStr );
}

String^ EBInfo::CharCode( EBBook^ Book )
{
	wchar_t * HStr;
	switch ( Book->character_code )
	{
	case EBCharCode::EB_CHARCODE_ISO8859_1:
		HStr = L"ISO 8859-1";
		break;
	case EBCharCode::EB_CHARCODE_JISX0208:
		HStr = L"JIS X 0208";
		break;
	case EBCharCode::EB_CHARCODE_JISX0208_GB2312:
		HStr = L"JIS X 0208 + GB 2312";
		break;
	case EBCharCode::EB_CHARCODE_UTF8:
		HStr = L"UTF-8";
		break;
	default:
		HStr = L"unknown";
		break;
	}

	return ref new String( HStr );
}

Array<String^>^ EBInfo::SearchMethods( EBSubbook^ Subbook )
{
	Vector<String^>^ methods = ref new Vector<String^>();

	if ( Subbook->have_word_search() )
	{
		methods->Append( ref new String( L"word" ) );
	}
	if ( Subbook->have_endword_search() )
	{
		methods->Append( ref new String( L"endword" ) );
	}
	if ( Subbook->have_cross_search() )
	{
		methods->Append( ref new String( L"cross" ) );
	}
	if ( Subbook->have_multi_search() )
	{
		methods->Append( ref new String( L"multi" ) );
	}
	if ( Subbook->have_menu() )
	{
		methods->Append( ref new String( L"menu" ) );
	}
	if ( Subbook->have_image_menu() )
	{
		methods->Append( ref new String( L"image-menu" ) );
	}
	if ( Subbook->have_copyright() )
	{
		methods->Append( ref new String( L"copyright" ) );
	}

	Array<String^>^ A = ref new Array<String^>( methods->Size );
	int i = 0;
	for_each( begin( methods ), end( methods ), [ & ] ( String^ s )
	{
		A->set( i++, s );
	} );

	return A;
}

Array<int>^ EBInfo::FontList( EBSubbook^ subbook )
{
	EBFontCode font_list[ EB_MAX_FONTS ];
    int font_count;
	subbook->FontList( font_list, &font_count );

	Array<int>^ A = ref new Array<int>( font_count );
	for ( int j = 0; j < font_count; j++ )
	{
		A->set( j, EBFont::FontHeight2( font_list[ j ] ) );
	}

	return A;
}
//*
CharRange^ EBInfo::NarrowFontRange( EBSubbook^ subbook )
{
	EBFontCode font_list[ EB_MAX_FONTS ];
    int font_count;
	subbook->FontList( font_list, &font_count );
	subbook->SetFont( font_list[ 0 ] );

	return ref new CharRange( subbook->narrow_current->start, subbook->narrow_current->end );
}

CharRange^ EBInfo::WideFontRange( EBSubbook^ subbook )
{
	EBFontCode font_list[ EB_MAX_FONTS ];
    int font_count;
	subbook->FontList( font_list, &font_count );
	subbook->SetFont( font_list[ 0 ] );

	return ref new CharRange( subbook->wide_current->start, subbook->wide_current->end );
}

IAsyncOperation<CharRange^>^ EBInfo::GetNarrowFontRangeAsync( EBSubbook^ subbook )
{
	return create_async( [ = ] { return NarrowFontRange( subbook ); } );
}

IAsyncOperation<CharRange^>^ EBInfo::GetWideFontRangeAsync( EBSubbook^ subbook )
{
	return create_async( [ = ] { return WideFontRange( subbook ); } );
}