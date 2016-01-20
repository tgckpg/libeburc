#pragma once

#include <pch.h>
#include <defs.h>
#include <FileName.h>
#include <EBSearch.h>
#include <EBMultiSearch.h>
#include <EBUTF8Table.h>
#include <EBNarrowFont.h>
#include <EBWideFont.h>
#include <Zio.h>

using namespace std;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;

namespace libeburc
{
	ref class EBBook;
	/// <summary>
	/// A subbook in a book.
	/// </summary>
	public ref class EBSubbook sealed
	{
	internal:
		/// <summary>
		/// The parent book containing this subbook
		/// </summary>
		EBBook^ ParentBook;
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
		/// Sub-directory names. (EPWING only)
		/// </summary>
		char data_directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
		char gaiji_directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
		char movie_directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];

		/// <summary>
		/// The Subbook Root dir
		/// </summary>
		IStorageFolder^ DirRoot;

		/// <summary>
		/// Files
		/// </summary>
		/* Originally it stores filenames.
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
		/// Page number where search method titles are stored.
		/// (temporary need, EPWING only).
		/// </summary>
		int search_title_page;
		/*
		 * The number of multi-search methods the subbook has.
		 */
		int multi_count;
		/*
		 * Normalization table for UTF-8 subbook.
		 */
		int table_page;
		int table_size;
		
		Vector<EBUTF8Table^>^ table;
		int table_count;
		byte* table_buffer;
		/*
		 * The top page of multi search methods.
		 */
		EBMultiSearch^ multis[ EB_MAX_MULTI_SEARCHES ];

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

		/// <summary>
		/// Load the UTF8 table.
		/// </summary>
		void LoadUTF8Table();

		/// <summary>
		/// Load font files.
		/// </summary>
		void LoadFontHeaders();

		/// <summary>
		/// The top page of search methods.
		/// </summary>
		EBSearch^ word_alphabet;
		EBSearch^ word_asis;
		EBSearch^ word_kana;
		EBSearch^ endword_alphabet;
		EBSearch^ endword_asis;
		EBSearch^ endword_kana;
		EBSearch^ keyword;
		EBSearch^ menu;
		EBSearch^ image_menu;
		EBSearch^ cross;
		EBSearch^ copyright;
		EBSearch^ text;
		EBSearch^ sound;

		/*
		 * Font list.
		 */
		EBFont^ narrow_fonts[ EB_MAX_FONTS ];
		EBFont^ wide_fonts[ EB_MAX_FONTS ];

		/*
		 * Current narrow and wide fonts.
		 */
		EBFont^ *narrow_current;
		EBFont^ *wide_current;

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

