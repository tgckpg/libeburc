#pragma once

#include <pch.h>
#include <defs.h>
#include <EBSubbook.h>
#include <FileName.h>
#include <EBException.h>
#include <JACode.h>
#include <build-post.h>
#include <Windows.h>

using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

namespace libeburc
{
	/// <summary>
	/// A Book.
	/// </summary>
	public ref class EBBook sealed
	{

		IStorageFolder^ DirRoot;

		/*
		 * Fix chachacter-code of the book if misleaded.
		 */
		void FixMislead();

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
		/*
		 * Read information from the `CATALOG' file in 'book'. (EB)
		 */
		void LoadCatalogEB( IStorageFile^ File );

		EBBook( IStorageFolder^ BookDir );
	internal:
		int subbook_count;
		EBCharCode character_code;
		EBDiscCode disc_code;
		EBBookCode code;
		Vector<EBSubbook^>^ subbooks;

	public:
		/// <summary>
		/// Book ID
		/// </summary>
		property EBBookCode Code { EBBookCode get() { return code; } }

		/// <summary>
		/// Disc type.  EB* or EPWING.
		/// </summary>
		property EBDiscCode DiscCode { EBDiscCode get() { return disc_code; } }

		/// <summary>
		/// Character code of the book.
		/// </summary>
		property EBCharCode CharCode { EBCharCode get() { return character_code; } }

		/// <summary>
		/// The number of subbooks the book has.
		/// </summary>
		property int SubbookCount { int get() { return subbook_count; } }

		/// <summary>
		/// Subbook list.
		/// </summary>
		property IIterable<EBSubbook^>^ Subbooks { IIterable<EBSubbook^>^ get() { return subbooks->GetView(); } }

		static IAsyncOperation<EBBook^>^ Parse( IStorageFolder^ BookDir );
	};
}
