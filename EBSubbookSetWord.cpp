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
		ConvertLatin( input_word, word, word_code );
	else if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
		ConvertUtf8( input_word, word, word_code );
	else
		ConvertEUCJP( input_word, word, word_code );

	strcpy_s( canonicalized_word, strlen( word ), word );

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
	eb_fix_word( book, search, word, canonicalized_word );
}

void EBSubbook::ConvertLatin( const char *input_word, char *word, EBWordCode *word_code )
{
	unsigned char *wp = ( unsigned char * ) word;
	const unsigned char *inp = ( const unsigned char * ) input_word;
	const unsigned char *tail;
	unsigned char c1;
	int word_length = 0;

	*word_code = EBWordCode::EB_WORD_INVALID;

	/*
	 * Find the tail of `input_word'.
	 */
	tail = ( const unsigned char * ) input_word + strlen( input_word ) - 1;
	while ( ( const unsigned char * ) input_word <= tail
		&& ( *tail == ' ' || *tail == '\t' ) )
		tail--;
	tail++;

	/*
	 * Ignore spaces and tabs in the beginning of `input_word'.
	 */
	while ( *inp == ' ' || *inp == '\t' )
		inp++;

	while ( inp < tail )
	{
		/*
		 * Check for the length of the word.
		 * If exceeds, return with an error code.
		 */
		if ( EB_MAX_WORD_LENGTH < word_length + 1 )
		{
			EBException::Throw( EBErrorCode::EB_ERR_TOO_LONG_WORD );
		}

		c1 = *inp++;

		/*
		 * Tabs are translated to spaces.
		 */
		if ( c1 == '\t' )
			c1 = ' ';

		*wp++ = c1;

		/*
		 * Skip successive spaces and tabs.
		 */
		if ( c1 == ' ' )
		{
			while ( *inp == '\t' || *inp == ' ' )
				inp++;
		}

		word_length++;
	}
	*wp = '\0';

	if ( word_length == 0 )
	{
		EBException::Throw( EBErrorCode::EB_ERR_EMPTY_WORD );
	}

	*word_code = EBWordCode::EB_WORD_ALPHABET;
}


/*
 * Convert `input_word' to UTF 8 and put it into `word'.
 *
 * If `input_word' is a valid string to search, EB_WORD_OTHER is returned.
 * Otherwise, -1 is returned.
 */
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

/*
 * Table used to convert JIS X 0208 to ASCII.
 */
static const unsigned int jisx0208_table[] = {
    /* 0x20 -- 0x2f */
    0x2121, 0x212a, 0x2149, 0x2174, 0x2170, 0x2173, 0x2175, 0x2147,
    0x214a, 0x214b, 0x2176, 0x215c, 0x2124, 0x215d, 0x2125, 0x213f,
    /* 0x30 -- 0x3f */
    0x2330, 0x2331, 0x2332, 0x2333, 0x2334, 0x2335, 0x2336, 0x2337,
    0x2338, 0x2339, 0x2127, 0x2128, 0x2163, 0x2161, 0x2164, 0x2129,
    /* 0x40 -- 0x4f */
    0x2177, 0x2341, 0x2342, 0x2343, 0x2344, 0x2345, 0x2346, 0x2347,
    0x2348, 0x2349, 0x234a, 0x234b, 0x234c, 0x234d, 0x234e, 0x234f,
    /* 0x50 -- 0x5f */
    0x2350, 0x2351, 0x2352, 0x2353, 0x2354, 0x2355, 0x2356, 0x2357,
    0x2358, 0x2359, 0x235a, 0x214e, 0x2140, 0x214f, 0x2130, 0x2132,
    /* 0x60 -- 0x6f */
    0x2146, 0x2361, 0x2362, 0x2363, 0x2364, 0x2365, 0x2366, 0x2367,
    0x2368, 0x2369, 0x236a, 0x236b, 0x236c, 0x236d, 0x236e, 0x236f,
    /* 0x70 -- 0x7e */
    0x2370, 0x2371, 0x2372, 0x2373, 0x2374, 0x2375, 0x2376, 0x2377,
    0x2378, 0x2379, 0x237a, 0x2150, 0x2143, 0x2151, 0x2141
};

/*
 * Table used to convert JIS X 0201 KATAKANA to JIS X 0208.
 */
static const unsigned int jisx0201_table[] = {
    /* 0xa0 -- 0xaf */
    0x0000, 0x2123, 0x2156, 0x2157, 0x2122, 0x2126, 0x2572, 0x2521,
    0x2523, 0x2525, 0x2527, 0x2529, 0x2563, 0x2565, 0x2567, 0x2543,
    /* 0xb0 -- 0xbf */
    0x213c, 0x2522, 0x2524, 0x2526, 0x2528, 0x252a, 0x252b, 0x252d,
    0x252f, 0x2531, 0x2533, 0x2535, 0x2537, 0x2539, 0x253b, 0x253d,
    /* 0xc0 -- 0xcf */
    0x253f, 0x2541, 0x2544, 0x2546, 0x2548, 0x254a, 0x254b, 0x254c,
    0x254d, 0x254e, 0x254f, 0x2552, 0x2555, 0x2558, 0x255b, 0x255e,
    /* 0xd0 -- 0xdf */
    0x255f, 0x2560, 0x2561, 0x2562, 0x2564, 0x2566, 0x2568, 0x2569,
    0x256a, 0x256b, 0x256c, 0x256d, 0x256f, 0x2573, 0x212b, 0x212c
};

void EBSubbook::ConvertEUCJP( const char *input_word, char *word, EBWordCode *word_code )
{
	char *wp = ( char * ) word;
	const char *inp = ( const char * ) input_word;
	unsigned char c1 = 0, c2 = 0;
	int kana_count = 0;
	int alphabet_count = 0;
	int kanji_count = 0;
	int word_length = 0;

	/*
	 * Find the tail of `input_word'.
	 */
	const char *tail = ( const char * ) input_word + strlen( input_word ) - 1;
	for ( ;;)
	{
		if ( inp < tail && ( *tail == ' ' || *tail == '\t' ) )
			tail--;
		else if ( inp < tail - 1 && *tail == 0xa1 && *( tail - 1 ) == 0xa1 )
			tail -= 2;
		else
			break;
	}
	tail++;

	/*
	 * Ignore spaces and tabs in the beginning of `input_word'.
	 */
	for ( ;;)
	{
		if ( *inp == ' ' || *inp == '\t' )
			inp++;
		else if ( *inp == 0xa1 && *( inp + 1 ) == 0xa1 )
			inp += 2;
		else
			break;
	}

	while ( inp < tail )
	{
		/*
		 * Check for the length of the word.
		 * If exceeds, return with an error code.
		 */
		if ( EB_MAX_WORD_LENGTH < word_length + 2 )
		{
			EBException::Throw( EBErrorCode::EB_ERR_TOO_LONG_WORD );
		}

		/*
		 * Tabs are translated to spaces.
		 */
		c1 = *inp++;
		if ( c1 == '\t' )
			c1 = ' ';

		if ( 0x20 <= c1 && c1 <= 0x7e )
		{
			/*
			 * `c1' is a character in ASCII.
			 */
			unsigned int c = jisx0208_table[ c1 - 0x20 ];
			c1 = c >> 8;
			c2 = c & 0xff;
		}
		else if ( 0xa1 <= c1 && c1 <= 0xfe )
		{
			/*
			 * `c1' is a character in JIS X 0208, or local character.
			 */
			c2 = *inp++;

			if ( 0xa1 <= c2 && c2 <= 0xfe )
			{
				c1 &= 0x7f;
				c2 &= 0x7f;
			}
			else if ( c2 < 0x20 || 0x7e < c2 )
			{
				EBException::Throw( EBErrorCode::EB_ERR_BAD_WORD );
			}
		}
		else if ( c1 == 0x8e )
		{
			/*
			 * `c1' is SS2.
			 */
			if ( c2 < 0xa1 || 0xdf < c2 )
			{
				EBException::Throw( EBErrorCode::EB_ERR_BAD_WORD );
			}

			c2 = jisx0201_table[ c2 - 0xa0 ];
			c1 = 0x25;
		}
		else
		{
			EBException::Throw( EBErrorCode::EB_ERR_BAD_WORD );
		}

		/*
		 * The following characters are recognized as alphabet.
		 *   2330 - 2339: `0' .. `9'
		 *   2341 - 235a: `A' .. `Z'
		 *   2361 - 237a: `a' .. `z' (convert to upper cases)
		 */
		*wp++ = c1;
		*wp++ = c2;

		if ( c1 == 0x23 )
			alphabet_count++;
		else if ( c1 == 0x24 || c1 == 0x25 )
			kana_count++;
		else if ( c1 != 0x21 )
			kanji_count++;

		word_length += 2;
	}
	*wp = '\0';

	if ( word_length == 0 )
	{
		EBException::Throw( EBErrorCode::EB_ERR_EMPTY_WORD );
	}
	if ( alphabet_count == 0 && kana_count != 0 && kanji_count == 0 )
		*word_code = EBWordCode::EB_WORD_KANA;
	else if ( alphabet_count != 0 && kana_count == 0 && kanji_count == 0 )
		*word_code = EBWordCode::EB_WORD_ALPHABET;
	else
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

void EBSubbook::FixWord( const EBSearch^ search, char *word,
	char *canonicalized_word )
{
	if ( search->index_id == 0xa1 && search->candidates_page != 0 )
		return;

	if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
	{
		return;
	}
	else if ( book->character_code == EB_CHARCODE_ISO8859_1 )
	{
		if ( search->space == EB_INDEX_STYLE_DELETE )
			eb_delete_spaces_latin( canonicalized_word );

		if ( search->lower == EB_INDEX_STYLE_CONVERT )
			eb_convert_lower_latin( canonicalized_word );

	}
	else
	{
		if ( search->space == EB_INDEX_STYLE_DELETE )
			eb_delete_spaces_jis( canonicalized_word );

		if ( search->katakana == EB_INDEX_STYLE_CONVERT )
			eb_convert_katakana_jis( canonicalized_word );
		else if ( search->katakana == EB_INDEX_STYLE_REVERSED_CONVERT )
			eb_convert_hiragana_jis( canonicalized_word );

		if ( search->lower == EB_INDEX_STYLE_CONVERT )
			eb_convert_lower_jis( canonicalized_word );

		if ( search->mark == EB_INDEX_STYLE_DELETE )
			eb_delete_marks_jis( canonicalized_word );

		if ( search->long_vowel == EB_INDEX_STYLE_CONVERT )
			eb_convert_long_vowels_jis( canonicalized_word );
		else if ( search->long_vowel == EB_INDEX_STYLE_DELETE )
			eb_delete_long_vowels_jis( canonicalized_word );

		if ( search->double_consonant == EB_INDEX_STYLE_CONVERT )
			eb_convert_double_consonants_jis( canonicalized_word );

		if ( search->contracted_sound == EB_INDEX_STYLE_CONVERT )
			eb_convert_contracted_sounds_jis( canonicalized_word );

		if ( search->small_vowel == EB_INDEX_STYLE_CONVERT )
			eb_convert_small_vowels_jis( canonicalized_word );

		if ( search->voiced_consonant == EB_INDEX_STYLE_CONVERT )
			eb_convert_voiced_consonants_jis( canonicalized_word );

		if ( search->p_sound == EB_INDEX_STYLE_CONVERT )
			eb_convert_p_sounds_jis( canonicalized_word );
	}

	if ( search->index_id != 0x70 && search->index_id != 0x90 )
		strcpy_s( word, strlen( canonicalized_word ), canonicalized_word );
}