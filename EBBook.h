#pragma once

#include <pch.h>
#include <defs.h>
#include <EBSubbook.h>
#include <FileName.h>
#include <EBException.h>

using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;

namespace libeburc
{
	/// <summary>
	/// A Book.
	/// </summary>
	public ref class EBBook sealed
	{

		IStorageFolder^ DirRoot;

		void Bind();
		/*
		 * Read information from the `LANGUAGE' file.
		 * If failed to initialize, JIS X 0208 is assumed.
		 */
		void LoadLanguage();
		/*
		 * Read information from the `CATALOG(S)' file in 'book'.
		 * Return EB_SUCCESS if it succeeds, error-code otherwise.
		 */
		void LoadCatalog();

		EBBook( IStorageFolder^ BookDir );

	public:
		/// <summary>
		/// Book ID
		/// </summary>
		property EBBookCode Code;

		/// <summary>
		/// Disc type.  EB* or EPWING.
		/// </summary>
		property EBDiscCode DiscCode; 

		/// <summary>
		/// Character code of the book.
		/// </summary>
		property EBCharacterCode CharCode;

		/// <summary>
		/// The number of subbooks the book has.
		/// </summary>
		property int SubbookCount;

		/// <summary>
		/// Subbook list.
		/// </summary>
		property IIterable<EBSubbook^>^ Subbooks;

		static IAsyncOperation<EBBook^>^ Parse(IStorageFolder^ BookDir);
	};
}
