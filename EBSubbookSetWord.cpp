#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

void EBSubbook::SetWord( const char *input_word, char *word,
	char *canonicalized_word, EBWordCode *word_code )
{
	// In case failed
	// *word = '\0';
	// *canonicalized_word = '\0';
	*word_code = EBWordCode::EB_WORD_INVALID;

	const EBSearch^ search;

	/*
	 * Make a fixed word and a canonicalized word from `input_word'.
	 */
	if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_ISO8859_1 )
		JACode::ConvertLatin( input_word, word, word_code );
	else if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
		ConvertUtf8( input_word, word, word_code );
	else
		JACode::ConvertEUCJP( input_word, word, word_code );

	size_t slen = strlen( word );
	memcpy_s( canonicalized_word, slen , word, slen );

	/*
	 * Determine search method.
	 */
	switch ( *word_code )
	{
	case EBWordCode::EB_WORD_ALPHABET:
		if ( word_alphabet->start_page != 0 )
			search = word_alphabet;
		else if ( word_asis->start_page != 0 )
			search = word_asis;
		else
		{
			EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
		}
		break;

	case EBWordCode::EB_WORD_KANA:
		if ( word_kana->start_page != 0 )
			search = word_kana;
		else if ( word_asis->start_page != 0 )
			search = word_asis;
		else
		{
			EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
		}
		break;

	case EBWordCode::EB_WORD_OTHER:
		if ( word_asis->start_page != 0 )
			search = word_asis;
		else
		{
			EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
		}
		break;

	default:
		EBException::Throw( EBErrorCode::EB_ERR_NO_SUCH_SEARCH );
	}

	/*
	 * Fix the word.
	 */
	FixWord( search, word, canonicalized_word );
}

void EBSubbook::ConvertUtf8( const char *input_word, char *word, EBWordCode *word_code )
{
	char *wp = ( char * ) word;
	const char *inp = ( const char * ) input_word;
	int word_length = 0;

	const char *tail = ( const char * ) input_word + strlen( input_word );

	while ( inp < tail )
	{
		if ( *inp < 0x20 )
		{
			inp++;
			continue;
		}

		int chr;
		int count = Utils::ReadUtf8( inp, &chr );
		if ( !count )
		{
			EBException::Throw( EBErrorCode::EB_ERR_BAD_WORD );
		}

		char *result = NormalizeUtf8( chr );

		if ( result == NULL )
		{
			if ( EB_MAX_WORD_LENGTH < word_length + count )
			{
				EBException::Throw( EBErrorCode::EB_ERR_TOO_LONG_WORD );
			}

			memcpy_s( wp, count, inp, count );
			wp += count;
			word_length += count;
		}
		else if ( strlen( result ) )
		{
			if ( EB_MAX_WORD_LENGTH < word_length + strlen( result ) )
			{
				EBException::Throw( EBErrorCode::EB_ERR_TOO_LONG_WORD );
			}

			size_t rlen = strlen( result );
			memcpy_s( wp, rlen, result, rlen );
			wp += rlen;
			word_length += rlen;
		}
		inp += count;
	}

	if ( word_length == 0 )
	{
		wp = ( char * ) word;
		inp = ( const char * ) input_word;
		while ( inp < tail )
		{
			if ( *inp <= 0x20 )
			{
				inp++;
				continue;
			}
			*wp = *inp;
			wp++;
			inp++;
			word_length++;
		}
	}

	if ( word_length == 0 )
	{
		EBException::Throw( EBErrorCode::EB_ERR_EMPTY_WORD );
	}

	*wp = '\0';
	*word_code = EBWordCode::EB_WORD_OTHER;
}

char* EBSubbook::NormalizeUtf8( int code )
{
	int floor = -1;

	int ceil = table_count;

	while ( floor + 1 != ceil )
	{
		int current = ( floor + ceil ) / 2;
		EBUTF8Table^ t = table->GetAt( current );

		if ( t->code == code )
		{
			return t->string;
		}

		if ( t->code < code )
		{
			floor = current;
		}
		else
		{
			ceil = current;
		}
	}

	return NULL;
}

void EBSubbook::FixWord( const EBSearch^ search, char *word, char *canonicalized_word )
{
	if ( search->index_id == 0xa1 && search->candidates_page != 0 )
		return;

	if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
	{
		return;
	}
	else if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_ISO8859_1 )
	{
		if ( search->space == EBIndexStyleCode::EB_INDEX_STYLE_DELETE )
			JACode::DeleteSpacesLatin( canonicalized_word );

		if ( search->lower == EBIndexStyleCode::EB_INDEX_STYLE_CONVERT )
			JACode::ConvertLowerLatin( canonicalized_word );

	}
	else
	{
		if ( search->space == EBIndexStyleCode::EB_INDEX_STYLE_DELETE )
			JACode::DeleteSpacesJIS( canonicalized_word );

		if ( search->katakana == EBIndexStyleCode::EB_INDEX_STYLE_CONVERT )
			JACode::ConvertKataJIS( canonicalized_word );
		else if ( search->katakana == EBIndexStyleCode::EB_INDEX_STYLE_REVERSED_CONVERT )
			JACode::ConvertHiraJIS( canonicalized_word );

		if ( search->lower == EBIndexStyleCode::EB_INDEX_STYLE_CONVERT )
			JACode::ConvertLowerJIS( canonicalized_word );

		if ( search->mark == EBIndexStyleCode::EB_INDEX_STYLE_DELETE )
			JACode::DeletMarksJIS( canonicalized_word );

		if ( search->long_vowel == EBIndexStyleCode::EB_INDEX_STYLE_CONVERT )
			JACode::ConvertLongVowelsJIS( canonicalized_word );
		else if ( search->long_vowel == EBIndexStyleCode::EB_INDEX_STYLE_DELETE )
			JACode::DeleteLongVowelsJIS( canonicalized_word );

		if ( search->double_consonant == EBIndexStyleCode::EB_INDEX_STYLE_CONVERT )
			JACode::ConvertDoubleConsonantsJIS( canonicalized_word );

		if ( search->contracted_sound == EBIndexStyleCode::EB_INDEX_STYLE_CONVERT )
			JACode::ConvertContractedSoundsJIS( canonicalized_word );

		if ( search->small_vowel == EBIndexStyleCode::EB_INDEX_STYLE_CONVERT )
			JACode::ConvertSmallVowelsJIS( canonicalized_word );

		if ( search->voiced_consonant == EBIndexStyleCode::EB_INDEX_STYLE_CONVERT )
			JACode::ConvertVoicedConsonantsJIS( canonicalized_word );

		if ( search->p_sound == EBIndexStyleCode::EB_INDEX_STYLE_CONVERT )
			JACode::ConvertPSoundsJIS( canonicalized_word );
	}

	if ( search->index_id != 0x70 && search->index_id != 0x90 )
	{
		size_t slen = strlen( canonicalized_word );
		memcpy_s( word, slen, canonicalized_word, slen );
	}
}

