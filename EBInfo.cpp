#include "pch.h"
#include "EBInfo.h"

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
