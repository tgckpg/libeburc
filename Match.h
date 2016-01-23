#pragma once

namespace libeburc
{
	ref class Match sealed
	{
	internal:
		/*
		 * Compare `word' and `pattern'.
		 * `word' must be terminated by `\0' and `pattern' is assumed to be
		 * `length' characters long.
		 *
		 * When `word' is equal to `pattern', or equal to the beginning of
		 * `pattern', 0 is returned.  A positive or negateive integer is
		 * returned according as `pattern' is greater or less than `word'.
		 */
		static int Word( const char *word, const char *pattern, size_t length );
		/*
		 * Compare `word' and `pattern' for pre-search.
		 * `word' must be terminated by `\0' and `pattern' is assumed to be
		 * `length' characters long.
		 *
		 * When `word' is equal to `pattern', or equal to the beginning of
		 * `pattern', 0 is returned.  A positive or negateive integer is
		 * returned according as `pattern' is greater or less than `word'.
		 */
		static int PreWord( const char *word, const char *pattern, size_t length );
		/*
		 * Compare `word' and `pattern' in JIS X 0208.
		 * `word' must be terminated by `\0' and `pattern' is assumed to be
		 * `length' characters long.
		 *
		 * When the word is equal to the pattern, 0 is returned.  A positive or
		 * negateive integer is returned according as `pattern' is greater or
		 * less than `word'.
		 */
		static int ExactWordJIS( const char *word, const char *pattern, size_t length );
		/*
		 * Compare `word' and `pattern' in JIS X 0208 for pre-search.
		 * `word' must be terminated by `\0' and `pattern' is assumed to be
		 * `length' characters long.
		 *
		 * When the word is equal to the pattern, 0 is returned.  A positive or
		 * negateive integer is returned according as `pattern' is greater or
		 * less than `word'.
		 */
		static int ExactPreWordJIS( const char *word, const char *pattern, size_t length );
		/*
		 * Compare `word' and `pattern' in Latin1.
		 * `word' must be terminated by `\0' and `pattern' is assumed to be
		 * `length' characters long.
		 *
		 * When the word is equal to the pattern, 0 is returned.  A positive or
		 * negateive integer is returned according as `pattern' is greater or
		 * less than `word'.
		 */
		static int ExactWordLatin( const char *word, const char *pattern, size_t length );
		/*
		 * Compare `word' and `pattern' in Latin1 for pre-search.
		 * `word' must be terminated by `\0' and `pattern' is assumed to be
		 * `length' characters long.
		 *
		 * When the word is equal to the pattern, 0 is returned.  A positive or
		 * negateive integer is returned according as `pattern' is greater or
		 * less than `word'.
		 */
		static int ExactPreWordLatin( const char *word, const char *pattern, size_t length );
		/*
		 * Compare `word' and `pattern' in JIS X 0208.
		 *
		 * This function is equivalent to eb_match_word() except that this function
		 * ignores differences of kana (katakana and hiragana).  The order of
		 * hiragana and katakana characters is:
		 *
		 * If `word' and `pattern' differ, the function compares their characters
		 * with the following rule:
		 *
		 *    HIRAGANA `KA' < HIRAGANA `GA' < KATAKANA `KA' < KATAKANA `GA'
		 */
		static int WordKanaGroup( const char *word, const char *pattern, size_t length );
		/*
		 * Compare `word' and `pattern' in JIS X 0208.
		 *
		 * This function is equivalent to eb_match_word() except that this function
		 * ignores differences of kana (katakana and hiragana).  The order of
		 * hiragana and katakana characters is:
		 *
		 * If `word' and `pattern' differ, the function compares their characters
		 * with the following rule:
		 *
		 *    HIRAGANA `KA' == KATAKANA `KA' < HIRAGANA `GA' == KATAKANA `GA'.
		 */
		static int WordKanaSingle( const char *word, const char *pattern, size_t length );

		/*
		 * Compare `word' and `pattern' in JIS X 0208.
		 *
		 * This function is equivalent to eb_exact_match_word_jis() except that
		 * this function ignores differences of kana (katakana and hiragana).
		 *
		 * If `word' and `pattern' differ, the function compares their characters
		 * with the following rule:
		 *
		 *    HIRAGANA `KA' < HIRAGANA `GA' < KATAKANA `KA' < KATAKANA `GA'
		 */
		static int ExactWordKanaGroup( const char *word, const char *pattern, size_t length );
		/*
		 * Compare `word' and `pattern' in JIS X 0208.
		 *
		 * This function is equivalent to eb_exact_match_word_jis() except that
		 * this function ignores differences of kana (katakana and hiragana).
		 * The order of hiragana and katakana characters is:
		 *
		 * If `word' and `pattern' differ, the function compares their characters
		 * with the following rule:
		 *
		 *    HIRAGANA `KA' == KATAKANA `KA' < HIRAGANA `GA' == KATAKANA `GA'.
		 */
		static int ExactWordKanaSingle( const char *word, const char *pattern, size_t length );
	};
}
