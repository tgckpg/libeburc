#pragma once

#include <pch.h>
#include <eburc/defs.h>
#include <eburc/Objects/EBAlternationCache.h>

using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;

namespace libeburc
{
	ref class EBAppendixSubbook;
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
		EBAlternationCache^ narrow_cache[ EB_MAX_ALTERNATION_CACHE ];
		EBAlternationCache^ wide_cache[ EB_MAX_ALTERNATION_CACHE ];

		EBAlternationCache^ AcquireWideCache( int i );
		EBAlternationCache^ AcquireNarrowCache( int i );
		EBAppendix();
	public:

		property IIterable<EBAppendixSubbook^>^ Subbooks
		{
			IIterable<EBAppendixSubbook^>^ get();
		}
	};
}
