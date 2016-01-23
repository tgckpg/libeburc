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

	return ref new String( ( LPWSTR ) Utils::EucJP2Utf16( text ) );
}

IIterable<EBPosition^>^ EBSubbook::Search( const char* phrase, EBSearchCode Code )
{
	Vector<EBPosition^>^ results = ref new Vector<EBPosition^>();

	try
	{
		switch ( Code )
		{
		case EBSearchCode::EB_SEARCH_CROSS:
		case EBSearchCode::EB_SEARCH_ENDWORD:
		case EBSearchCode::EB_SEARCH_KEYWORD:
		case EBSearchCode::EB_SEARCH_MULTI:
		case EBSearchCode::EB_SEARCH_WORD:
			throw ref new NotImplementedException( "This method is not yet implemented" );
		case EBSearchCode::EB_SEARCH_NONE:
			break;
		case EBSearchCode::EB_SEARCH_EXACTWORD:
			SeachExactWord( phrase );
			break;

		default:
			EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
		}
	}
	catch ( Exception^ ex )
	{
		ParentBook->ResetSearchContext();
	}

	return results->GetView();
}

IAsyncOperation<String^>^ EBSubbook::GetPageAsync( EBPosition^ Pos )
{
	return create_async( [ = ] { return GetPage( Pos ); } );
}

IAsyncOperation<IIterable<EBPosition^>^>^ EBSubbook::SearchAysnc( String^ Phrase, EBSearchCode Code )
{
	return create_async( [ = ]
	{
		return Search( ( char * ) Utils::Utf82EucJP( Phrase->Data() ), Code );
	} );
}
