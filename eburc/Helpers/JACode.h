#pragma once

#include <eburc/defs.h>
#include <eburc/EBException.h>

namespace libeburc
{
	class JACode
	{
	public:
		/*
		 * Convert `input_word' to ISO 8859 1 and put it into `word'.
		 *
		 * If `input_word' is a valid string to search, EB_WORD_ALPHABET is returned.
		 * Otherwise, -1 is returned.
		 */
		static void ConvertLatin( const char *input_word, char *word, EBWordCode *word_code );
		/*
		 * Convert `input_word' to JIS X0208 and put it into `word'.
		 *
		 * If `input_word' is a valid string to search, EB_WORD_ALPHABET or
		 * EB_WORD_KANA is returned.
		 * Otherwise, -1 is returned.
		 */
		static void ConvertEUCJP( const char *input_word, char *word, EBWordCode *word_code );

		/*
		 * Convert HIRAGANA to KATAKANA in `word'.
		 */
		static void ConvertHiraJIS( char *word );
		/*
		 * Convert KATAKANA to HIRAGANA in `word'.
		 */
		static void ConvertKataJIS( char *word );
		/*
		 * Convert lower case to upper case in `word'.
		 */
		static void ConvertLowerJIS( char *word );
		/*
		 * Convert the small vowels to the normal vowels.
		 * (`a', `i', `u', `e', `o' -> `A', `I', `U', `E', `O')
		 */
		static void ConvertSmallVowelsJIS( char *word );
		/*
		 * Convert long vowel marks in `word' to the previous vowels.
		 */
		static void ConvertLongVowelsJIS( char *word );
		/*
		 * Convert lower case to upper case in `word'.
		 */
		static void ConvertLowerLatin( char *word );
		/*
		 * Fix `canonicalized_word' and `word' according with `book->character_code'
		 * and `search'.
		 */
		/*
		 * Convert the double consonant mark `tu' to `TU'.
		 */
		static void ConvertDoubleConsonantsJIS( char *word );
		/*
		 * Convert the contracted sound marks to the corresponding
		 * non-contracted sound marks (e.g. `GA' to `KA').
		 */
		static void ConvertVoicedConsonantsJIS( char *word );
		/*
		 * Convert the p sound marks
		 * (`PA', `PI', `PU', `PE', `PO' -> `HA', `HI', `HU', `HE', `HO')
		 */
		static void ConvertPSoundsJIS( char *word );
		/*
		 * Convert the contracted sound marks to the corresponding
		 * non-contracted sound marks.
		 * (`ya', `yu', `yo', `wa', `ka', `ke' -> `YA', `YU', `YO', `WA', `KA', `KE')
		 */
		static void ConvertContractedSoundsJIS( char *word );
		/*
		 * Delete some marks in `word'.
		 */
		static void DeletMarksJIS( char *word );
		/*
		 * Delete long vowel marks in `word'.
		 */
		static void DeleteLongVowelsJIS( char *word );
		/*
		 * Delete spaces in `word'.
		 */
		static void DeleteSpacesLatin( char *word );
		/*
		 * Delete spaces in `word'.
		 */
		static void DeleteSpacesJIS( char *word );

		static void eb_jisx0208_to_euc( char *out_string, const char *in_string );
	};
}
