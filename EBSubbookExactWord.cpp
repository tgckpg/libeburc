#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

void EBSubbook::SeachExactWord( const char * input_word )
{
	/*
	 * Initialize search context.
	 */

	ParentBook->ResetSearchContext();
	EBSearchContext^ context = ParentBook->search_contexts;
	context->code = EBSearchCode::EB_SEARCH_EXACTWORD;

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
		context->compare_pre = Match::ExactPreWordLatin;
		context->compare_single = Match::ExactWordLatin;
		context->compare_group = Match::ExactWordLatin;
	}
	else if ( context->page == word_kana->start_page )
	{
		context->compare_pre = Match::ExactPreWordJIS;
		context->compare_single = Match::ExactWordKanaSingle;
		context->compare_group = Match::ExactWordKanaGroup;
	}
	else
	{
		context->compare_pre = Match::ExactPreWordJIS;
		context->compare_single = Match::ExactWordJIS;
		context->compare_group = Match::ExactWordKanaGroup;
	}

	/*
	 * Pre-search.
	 */
	PreSearchWord( context );

	return;

	/*
	 * An error occurs...
	 */
failed:
	ParentBook->ResetSearchContext();
}