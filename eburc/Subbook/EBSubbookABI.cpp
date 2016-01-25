#include "pch.h"
#include "eburc/Book/EBBook.h"
#include "eburc/Subbook/EBSubbook.h"

using namespace libeburc;

#define MAX_HITS 50
#define MAXLEN_TEXT 1023

String^ EBSubbook::GetPage( EBPosition^ Pos, ReadAction^ Action, EBHookSet^ HookSet )
{
	SeekText( Pos );

	char text[ MAXLEN_TEXT + 1 ];
	try
	{
		SSIZE_T text_length;

		int stopped = 0;
		while ( !stopped )
		{
			ReadText( nullptr, HookSet, nullptr, MAXLEN_TEXT, text, &text_length );
			stopped = IsTextStopped();

			// Call for action
			if ( Action )
			{
				Action( ref new String( ( LPWSTR ) Utils::EucJP2Utf16( text ) ) );
			}
		}

		// ForwardText( nullptr );
	}
	catch ( Exception^ ex )
	{
		EBErrorCode code = ( EBErrorCode ) -ex->HResult;
		if ( code != EBErrorCode::EB_ERR_END_OF_CONTENT )
			throw ex;
	}

	EBPosition^ npos = TellText();
	Pos->page = npos->page;
	Pos->offset = npos->offset;

	return ref new String( ( LPWSTR ) Utils::EucJP2Utf16( text ) );
}

IIterable<EBHit^>^ EBSubbook::Search( const char** phrase, EBSearchCode Code )
{
	Vector<EBHit^>^ results = ref new Vector<EBHit^>();

	try
	{
		switch ( Code )
		{
		case EBSearchCode::EB_SEARCH_CROSS:
		case EBSearchCode::EB_SEARCH_ENDWORD:
		case EBSearchCode::EB_SEARCH_MULTI:
		case EBSearchCode::EB_SEARCH_WORD:
			throw ref new NotImplementedException( "This method is not yet implemented" );
		case EBSearchCode::EB_SEARCH_NONE:
			break;
		case EBSearchCode::EB_SEARCH_KEYWORD:
			SearchKeyword( phrase );
			break;
		case EBSearchCode::EB_SEARCH_EXACTWORD:
			SeachExactWord( phrase[0] );
			break;

		default:
			EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
		}

		EBHit^ hits[ MAX_HITS ];
		int hit_count;
		HitList( MAX_HITS, hits, &hit_count );

		for ( int i = 0; i < hit_count; i++ )
		{
			results->Append( hits[ i ] );
		}
	}
	catch ( Exception^ ex )
	{
		ParentBook->ResetSearchContext();
	}

	return results->GetView();
}

IAsyncAction^ EBSubbook::OpenAsync()
{
	return create_async( [ & ] { SetAuto(); } );
}

IAsyncOperation<String^>^ EBSubbook::GetPageAsync( EBPosition^ Pos )
{
	return create_async( [ = ] { return GetPage( Pos ); } );
}

IAsyncOperation<String^>^ EBSubbook::GetPageAsync( EBPosition^ Pos, ReadAction^ Action )
{
	return create_async( [ = ] { return GetPage( Pos, Action ); } );
}

IAsyncOperation<String^>^ EBSubbook::GetPageAsync( EBPosition^ Pos, ReadAction^ Action, EBHookSet^ HookSet )
{
	return create_async( [ = ] { return GetPage( Pos, Action, HookSet ); } );
}

IAsyncOperation<IIterable<EBHit^>^>^ EBSubbook::SearchAysnc( IVector<String^>^ Phrases, EBSearchCode Code )
{
	return create_async( [ = ]
	{
		const char **converted = new const char*[ Phrases->Size ];

		int i = 0;
		for_each( begin( Phrases ), end( Phrases ), [&] ( String^ phrase )
		{
			char * n = ( char * ) Utils::Utf82EucJP( phrase->Data() );
			converted[ i++ ] = n;
		} );
		converted[ i ] = NULL;

		return Search( converted, Code );
	} );
}
