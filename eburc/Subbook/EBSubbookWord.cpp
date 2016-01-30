#include "pch.h"
#include "eburc/Book/EBBook.h"
#include "eburc/Subbook/EBSubbook.h"
#include "eburc/Search/Match.h"

using namespace libeburc;

void EBSubbook::SearchWord( const char *input_word )
{
	try
	{
		/*
		 * Initialize search context.
		 */
		ParentBook->ResetSearchContext();
		EBSearchContext^ context = ParentBook->search_contexts->GetAt( 0 );
		context->code = EBSearchCode::EB_SEARCH_WORD;


		/*
		 * Make a fixed word and a canonicalized word to search from
		 * `input_word'.
		 */
		EBWordCode word_code;
		SetWord( input_word, context->word, context->canonicalized_word, &word_code );

		/*
		 * Get a page number.
		 */
		switch ( word_code )
		{
		case EBWordCode::EB_WORD_ALPHABET:
			if ( word_alphabet->start_page != 0 )
				context->page = word_alphabet->start_page;
			else if ( word_asis->start_page != 0 )
				context->page = word_asis->start_page;
			else
			{
				EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
			}
			break;

		case EBWordCode::EB_WORD_KANA:
			if ( word_kana->start_page != 0 )
				context->page = word_kana->start_page;
			else if ( word_asis->start_page != 0 )
				context->page = word_asis->start_page;
			else
			{
				EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
			}
			break;

		case EBWordCode::EB_WORD_OTHER:
			if ( word_asis->start_page != 0 )
				context->page = word_asis->start_page;
			else
			{
				EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
			}
			break;

		default:
			EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
		}

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
		else if ( context->page == word_kana->start_page )
		{
			context->compare_pre = Match::PreWord;
			context->compare_single = Match::WordKanaSingle;
			context->compare_group = Match::WordKanaGroup;
		}
		else
		{
			context->compare_pre = Match::PreWord;
			context->compare_single = Match::Word;
			context->compare_group = Match::WordKanaGroup;
		}

		/*
		 * Pre-search.
		 */
		PreSearchWord( context );
	}
	catch ( Exception^ ex )
	{
		ParentBook->ResetSearchContext();
	}
}
