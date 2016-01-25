#include "pch.h"
#include "eburc/Book/EBBook.h"
#include "eburc/Subbook/EBSubbook.h"

using namespace libeburc;

void EBSubbook::SearchKeyword( const char * const input_words[] )
{
	try
	{
		/*
		 * Check whether the current subbook has keyword search.
		 */
		if ( keyword->start_page == 0 )
		{
			EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
		}

		/*
		 * Attach a search context for each keyword, and pre-search the
		 * keywords.
		 */
		ParentBook->ResetSearchContext();
		int word_count = 0;
		int i;

		for ( i = 0; i < EB_MAX_KEYWORDS; i++ )
		{
			if ( input_words[ i ] == NULL )
				break;

			/*
			 * Initialize search context.
			 */
			EBSearchContext^ context = ( EBSearchContext^ ) ParentBook->AcquireContext( word_count, EBContextType::SEARCH );
			context->code = EBSearchCode::EB_SEARCH_KEYWORD;

			/*
			 * Choose comparison functions.
			 */
			if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_ISO8859_1
				|| ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
			{
				context->compare_pre = Match::PreWord;
				context->compare_single = Match::Word;
				context->compare_group = Match::Word;
			}
			else
			{
				context->compare_pre = Match::PreWord;
				context->compare_single = Match::Word;
				context->compare_group = Match::WordKanaGroup;
			}
			context->page = keyword->start_page;

			/*
			 * Make a fixed word and a canonicalized word to search from
			 * `input_words[i]'.
			 */
			try
			{
				EBWordCode word_code;
				SetKeyword( input_words[ i ], context->word, context->canonicalized_word, &word_code );
			}
			catch ( Exception^ ex )
			{
				if ( ( EBErrorCode ) -ex->HResult == EBErrorCode::EB_ERR_EMPTY_WORD )
					continue;
				throw ex;
			}

			/*
			 * Pre-search.
			 */
			PreSearchWord( context );

			word_count++;
		}

		if ( word_count == 0 )
		{
			EBException::Throw( EBErrorCode::EB_ERR_NO_WORD );
		}
		else if ( EB_MAX_KEYWORDS <= i && input_words[ i ] != NULL )
		{
			EBException::Throw( EBErrorCode::EB_ERR_TOO_MANY_WORDS );
		}

		/*
		 * Set `EB_SEARCH_NONE' to the rest unused search context.
		 */
		for ( i = word_count; i < EB_MAX_KEYWORDS; i++ )
		{
			EBSearchContext^ context = ( EBSearchContext^ ) ParentBook->AcquireContext( i, EBContextType::SEARCH );
			context->code = EBSearchCode::EB_SEARCH_NONE;
		}
	}
	catch ( Exception^ ex )
	{
		/*
		 * An error occurs...
		 */
		ParentBook->ResetSearchContext();
	}
}