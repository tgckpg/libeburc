#pragma once
#include <defs.h>

namespace libeburc
{
	/// <summary>
	/// Search methods in a subbook.
	/// </summary>
	ref class EBSearch sealed
	{
	internal:
		/// <summary>
		/// Index ID.
		///
		int index_id;

		/// <summary>
		/// Page number of the start page of an index.
		/// This search method is not available, if `start_page' is 0,
		/// </summary>
		int start_page;
		int end_page;

		/// <summary>
		/// Page number of the start page of candidates.
		/// (for multi search entry)
		/// </summary>
		int candidates_page;

		/// <summary>
		/// Index style flags.
		/// </summary>
		EBIndexStyleCode katakana;
		EBIndexStyleCode lower;
		EBIndexStyleCode mark;
		EBIndexStyleCode long_vowel;
		EBIndexStyleCode double_consonant;
		EBIndexStyleCode contracted_sound;
		EBIndexStyleCode voiced_consonant;
		EBIndexStyleCode small_vowel;
		EBIndexStyleCode p_sound;
		EBIndexStyleCode space;

		/// <summary>
		/// Label. (for an entry in multi search)
		/// </summary>
		char label[ EB_MAX_MULTI_LABEL_LENGTH + 1 ];

	public:
		EBSearch();
	};

}
