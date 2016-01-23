#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

#define MAXLEN_TEXT 1023

String^ EBSubbook::GetPage( EBPosition^ Pos )
{
	SeekText( Pos );

	SSIZE_T text_length;
	char text[ MAXLEN_TEXT + 1 ];

	int stopped = 0;
	while ( !stopped )
	{
		ReadText( nullptr, nullptr, nullptr, MAXLEN_TEXT, text, &text_length );
		stopped = IsTextStopped();
	}

	ForwardText( nullptr );
	EBPosition^ npos = TellText();
	Pos->page = npos->page;
	Pos->offset = npos->offset;

	return ref new String( ( LPWSTR ) Utils::MBEUCJP16( text ) );
}

IAsyncOperation<String^>^ EBSubbook::GetPageAsync( EBPosition^ Pos )
{
	return create_async( [ = ]
	{
		return GetPage( Pos );
	} );
}