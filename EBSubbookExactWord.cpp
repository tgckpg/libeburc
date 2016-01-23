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
	error_code = eb_set_word( book, input_word, context->word,
		context->canonicalized_word, &word_code );
	if ( error_code != EB_SUCCESS )
		goto failed;

	/*
	 * Get a page number.
	 */
	switch ( word_code )
	{
	case EBWordCode::EB_WORD_ALPHABET:
		if ( book->subbook_current->word_alphabet.start_page != 0 )
			context->page = book->subbook_current->word_alphabet.start_page;
		else if ( book->subbook_current->word_asis.start_page != 0 )
			context->page = book->subbook_current->word_asis.start_page;
		else
		{
			error_code = EB_ERR_NO_SUCH_SEARCH;
			goto failed;
		}
		break;

	case EBWordCode::EB_WORD_KANA:
		if ( book->subbook_current->word_kana.start_page != 0 )
			context->page = book->subbook_current->word_kana.start_page;
		else if ( book->subbook_current->word_asis.start_page != 0 )
			context->page = book->subbook_current->word_asis.start_page;
		else
		{
			error_code = EB_ERR_NO_SUCH_SEARCH;
			goto failed;
		}
		break;

	case EBWordCode::EB_WORD_OTHER:
		if ( book->subbook_current->word_asis.start_page != 0 )
			context->page = book->subbook_current->word_asis.start_page;
		else
		{
			error_code = EB_ERR_NO_SUCH_SEARCH;
			goto failed;
		}
		break;

	default:
		error_code = EB_ERR_NO_SUCH_SEARCH;
		goto failed;
	}

	/*
	 * Choose comparison functions.
	 */
	if ( book->character_code == EB_CHARCODE_ISO8859_1
		|| book->character_code == EB_CHARCODE_UTF8 )
	{
		context->compare_pre = eb_exact_pre_match_word_latin;
		context->compare_single = eb_exact_match_word_latin;
		context->compare_group = eb_exact_match_word_latin;
	}
	else if ( context->page == book->subbook_current->word_kana.start_page )
	{
		context->compare_pre = eb_exact_pre_match_word_jis;
		context->compare_single = eb_exact_match_word_kana_single;
		context->compare_group = eb_exact_match_word_kana_group;
	}
	else
	{
		context->compare_pre = eb_exact_pre_match_word_jis;
		context->compare_single = eb_exact_match_word_jis;
		context->compare_group = eb_exact_match_word_kana_group;
	}

	/*
	 * Pre-search.
	 */
	error_code = eb_presearch_word( book, context );
	if ( error_code != EB_SUCCESS )
		goto failed;

	LOG( ( "out: eb_search_exactword() = %s", eb_error_string( EB_SUCCESS ) ) );
	eb_unlock( &book->lock );

	return EB_SUCCESS;

	/*
	 * An error occurs...
	 */
failed:
	ParentBook->ResetSearchContext();
}
