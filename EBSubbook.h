#pragma once

#include <pch.h>
#include <defs.h>
#include <FileName.h>
#include <EBAppendixSubbook.h>
#include <EBHookSet.h>
#include <EBMultiSearch.h>
#include <EBNarrowFont.h>
#include <EBPosition.h>
#include <EBSearch.h>
#include <EBTextContext.h>
#include <EBUTF8Table.h>
#include <EBWideFont.h>
#include <Zio.h>

using namespace std;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;

/*
 * The maximum number of arguments for an escape sequence.
 */
#define EB_MAX_ARGV 	7

/*
 * Read next when the length of cached data is shorter than this value.
 */
#define SIZE_FEW_REST	48

/*
 * Special skip-code that represents `no skip-code is set'.
 */
#define SKIP_CODE_NONE  -1

namespace libeburc
{
	ref class EBBook;
	/// <summary>
	/// A subbook in a book.
	/// </summary>
	public ref class EBSubbook sealed
	{
		void ReadTextInternal(
			EBAppendixSubbook^ appendix, EBHookSet^ hookset
			, void *container
			, size_t text_max_length, char *text
			, SSIZE_T *text_length, int forward_only );

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
		char directory_name[ EB_MAX_DIRECTORY_NAME_LENGTH + 1 ];

		/// <summary>
		/// The Subbook Root dir
		/// </summary>
		IStorageFolder^ DirRoot;

		/// <summary>
		/// Sub-directory names. (EPWING only)
		/// </summary>
		IStorageFolder^ DataDir;
		IStorageFolder^ GaijiDir;
		IStorageFolder^ MovieDir;
		/// <summary>
		/// filenames
		/// (temporary need, EPWING only).
		/// </summary>
		char text_file_name[ EB_MAX_FILE_NAME_LENGTH + 1 ];
		char graphic_file_name[ EB_MAX_FILE_NAME_LENGTH + 1 ];
		char sound_file_name[ EB_MAX_FILE_NAME_LENGTH + 1 ];
		/// <summary>
		/// Compression hints of Text, graphic and sound files.
		/// (temporary need, EPWING only).
		/// </summary>
		ZioCode text_hint_zio_code;
		ZioCode graphic_hint_zio_code;
		ZioCode sound_hint_zio_code;

		/// <summary>
		/// Files
		/// </summary>
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
		/// <summary>
		/// The number of multi-search methods the subbook has.
		/// </summary>
		int multi_count;
		/// <summary>
		/// Normalization table for UTF-8 subbook.
		/// </summary>
		int table_page;
		int table_size;

		Vector<EBUTF8Table^>^ table;
		int table_count;
		byte* table_buffer;
		/// <summary>
		/// The top page of multi search methods.
		/// </summary>
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

		void FontList( EBFontCode *font_list, int *font_count );
		void SetFont( EBFontCode font_code );



		/// <summary>
		/// Examine whether the current subbook in `book' supports `WORD SEARCH'
		/// or not.
		/// </summary>
		bool have_word_search()
		{
			return !( word_alphabet->start_page == 0
				&& word_asis->start_page == 0
				&& word_kana->start_page == 0 );
		}
		/// <summary>
		/// Examine whether the current subbook in `book' supports `ENDWORD SEARCH'
		/// or not.
		/// </summary>
		bool have_endword_search()
		{
			return !( endword_alphabet->start_page == 0
				&& endword_asis->start_page == 0
				&& endword_kana->start_page == 0 );
		}
		/// <summary>
		/// Examine whether the current subbook in `book' supports `KEYWORD SEARCH'
		/// or not.
		/// </summary>
		bool have_keyword_search() { return keyword->start_page != 0; }
		/// <summary>
		/// Examine whether the current subbook in `book' supports `CROSS SEARCH'
		/// or not.
		/// </summary>
		bool have_cross_search() { return cross->start_page != 0; }
		/// <summary>
		/// Examine whether the current subbook in `book' supports `MULTI SEARCH'
		/// or not.
		/// </summary>
		bool have_multi_search() { return multi_count != 0; }
		/// <summary>
		/// Examine whether the current subbook in `book' supports `MENU SEARCH'
		/// or not.
		/// </summary>
		bool have_menu() { return menu->start_page != 0; }
		/// <summary>
		/// Examine whether the current subbook in `book' supports `GRAPHIC MENU SEARCH'
		/// or not.
		/// </summary>
		bool have_image_menu() { return image_menu->start_page != 0; }
		/// <summary>
		/// Examine whether the current subbook in `book' have a copyright
		/// notice or not.
		/// </summary>
		bool have_copyright() { return copyright->start_page != 0; }


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
		EBFont^ narrow_current;
		EBFont^ wide_current;

		EBSubbook( EBBook^ Book );

		/// <summary>
		/// Index page.
		/// </summary>
		int IndexPage;

		int EBSubbook::IsStopCode( EBAppendixSubbook^ appendix
			, unsigned int code0, unsigned int code1 );
		EBPosition^ TellText();
		void ReadText(
			EBAppendixSubbook^ appendix, EBHookSet^ hookset
			, void *container, size_t text_max_length
			, char *text, SSIZE_T *text_length );
		void SeekText( EBPosition^ Pos );

		EBSubbookCode code;
	public:
		/// <summary>
		/// Subbook ID.
		/// This subbook is not available, if the code is EB_SUBBOOK_INVALID.
		/// </summary>
		property EBSubbookCode Code
		{
			EBSubbookCode get() { return code; }
		}

		property String^ Title
		{
			String^ get()
			{
				return ref new String( ( LPWSTR ) Utils::MBEUCJP16( title ) );
			}
		}

		property EBPosition^ FirstPage
		{
			EBPosition^ get()
			{
				return ref new EBPosition( text->start_page, 0 );
			}
		}

		property String^ Directory
		{
			String^ get() { return DirRoot->Name; }
		}

		IAsyncAction^ OpenAsync();

		String^ GetPage( EBPosition^ Pos );

	};
}

