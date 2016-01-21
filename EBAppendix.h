#pragma once

#include <pch.h>
#include <defs.h>
#include <EBAppendixSubbook.h>

using namespace Platform::Collections;

namespace libeburc
{
	public ref class EBAppendix sealed
	{
	internal:
		/*
		 * Book ID.
		 */
		EBBookCode code;

		/*
		 * Path of the book.
		 */
		char *path;

		/*
		 * The length of the path.
		 */
		size_t path_length;

		/*
		 * Disc type.  EB (EB/EBG/EBXA/EBXA-C/S-EBXA) or EPWING.
		 */
		EBDiscCode disc_code;

		/*
		 * The number of subbooks the book has.
		 */
		int subbook_count;

		/*
		 * Subbook list.
		 */
		Vector<EBAppendixSubbook^>^ subbooks;

		/*
		 * Current subbook.
		 */
		EBAppendixSubbook^ subbook_current;

		/*
		 * Cache table for alternation text.
		 */
		// EB_Alternation_Cache narrow_cache[ EB_MAX_ALTERNATION_CACHE ];
		// EB_Alternation_Cache wide_cache[ EB_MAX_ALTERNATION_CACHE ];
		EBAppendix();
	public:
	};
}
