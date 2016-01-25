#pragma once
#include <eburc/Search/EBSearch.h>

namespace libeburc
{
	ref class EBMultiSearch sealed
	{
	internal:
		/// <summary>
		/// Search method information.
		/// </summary>
		EBSearch^ search;

		/// <summary>
		/// Search title. (EPWING only)
		/// </summary>
		char title[ EB_MAX_MULTI_TITLE_LENGTH + 1 ];

		/// <summary>
		/// The number of entries the multi search has.
		/// </summary>
		int entry_count;

		/// <summary>
		/// List of Word entry information.
		/// </summary>
		EBSearch^ entries[ EB_MAX_MULTI_ENTRIES ];
	public:
		EBMultiSearch();
	};
}
