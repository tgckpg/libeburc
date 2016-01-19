#pragma once

#include <pch.h>
#include <defs.h>
#include <FileName.h>
#include <Zio.h>

using namespace std;
using namespace Platform;
using namespace Windows::Foundation;

namespace libeburc
{
	ref class EBBook;
	public ref class EBSubbook sealed
	{
		/// <summary>
		/// The parent book containing this subbook
		/// </summary>
		EBBook^ ParentBook;

	internal:
		/// <summary>
		/// Whether the object has been initialized.
		/// </summary>
		int initialized;

		/// <summary>
		/// File descriptor and compression information for text file.
		/// </summary>
		Zio^ TextZio;

		/// <summary>
		/// File descriptor and compression information for graphic file.
		/// </summary>
		Zio^ GraphicZio;

		/// <summary>
		/// File descriptor and compression information for sound file.
		/// </summary>
		Zio^ SoundZio;

		/// <summary>
		/// File descriptor and compression information for movie file.
		/// </summary>
		Zio^ MovieZio;

		/// <summary>
		/// Subbook directory name.
		/// </summary>
		char directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];

		/// <summary>
		/// The Subbook Root dir
		/// </summary>
		IStorageFolder^ DirRoot;

		/// <summary>
		/// Files
		/// </summary>
		/* Originally Stores filenames.
		 *	but lets now use the StorageFile now
		 * char text_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
		 * char graphic_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
		 * char sound_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
		 */
		IStorageFile^ TextFile;
		IStorageFile^ GraphFile;
		IStorageFile^ SoundFile;

		/// <summary>
		/// Internal title
		/// </summary>
		char title[ EB_MAX_TITLE_LENGTH + 1 ];

		/// <summary>
		/// Set via typec EB
		/// </summary>
		void SetEB();

		/// <summary>
		/// Set via typec EB
		/// </summary>
		void SetEPWING();

		/// <summary>
		/// Set the subbook
		/// </summary>
		void SetAuto();

		/// <summary>
		/// Actually load it
		/// </summary>
		void Load();

		/// <summary>
		/// Get index information in the current subbook.
		/// </summary>
		void LoadIndexes();

		/// <summary>
		/// Get information about the current subbook.
		/// </summary>
		void LoadMultiSearches();

		/// <summary>
		/// Load multi search titles.
		/// </summary>
		void LoadMultiTitles();

		EBSubbook( EBBook^ Book );

	public:
		/// <summary>
		/// Index page.
		/// </summary>
		property int IndexPage;

		/// <summary>
		/// Subbook ID.
		/// This subbook is not available, if the code is EB_SUBBOOK_INVALID.
		/// </summary>
		property EBSubbookCode Code;

		property String^ Title
		{
			String^ get()
			{
				return ref new String( ( LPWSTR ) Utils::MBEUCJP16( title ) );
			}
		}

		property String^ Directory
		{
			String^ get() { return DirRoot->Name; }
		}

		IAsyncAction^ OpenAsync();

	};
}

