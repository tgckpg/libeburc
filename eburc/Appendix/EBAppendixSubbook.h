#pragma once
#include <eburc/defs.h>
#include <eburc/Objects/EBAlternationCache.h>
#include <eburc/EBException.h>
#include <eburc/Zio.h>

namespace libeburc
{
	ref class EBAppendix;
	public ref class EBAppendixSubbook sealed
	{
	internal:
		EBAppendix^ ParentBook;
		/*
		* Initialization flag.
		*/
		int initialized;

		/*
		 * Subbook ID.
		 */
		EBSubbookCode code;

		/*
		 * Directory name.
		 */
		char directory_name[ EB_MAX_DIRECTORY_NAME_LENGTH + 1 ];

		/*
		 * Sub-directory name. (EPWING only)
		 */
		char data_directory_name[ EB_MAX_DIRECTORY_NAME_LENGTH + 1 ];

		/*
		 * File name.
		 */
		char file_name[ EB_MAX_FILE_NAME_LENGTH + 1 ];

		/*
		 * Character code of the book.
		 */
		EBCharCode character_code;

		/*
		 * Start character number of the narrow/wide font.
		 */
		int narrow_start;
		int wide_start;

		/*
		 * End character number of the narrow/wide font.
		 */
		int narrow_end;
		int wide_end;

		/*
		 * Start page number of the narrow/wide font.
		 */
		int narrow_page;
		int wide_page;

		/*
		 * Stop code (first and second characters).
		 */
		int stop_code0;
		int stop_code1;

		/*
		 * Compression Information for appendix file.
		 */
		Zio^ zio;

		/*
		 * Get the alternation text of the character number `character_number'.
		 */
		void WideAltCharText( int character_number, char *text );
		void WideCharTextJIS( int character_number, char *text );
		void WideCharTextLatin( int character_number, char *text );
		void NarrowAltCharText( int character_number, char *text );
		void NarrowCharTextJIS( int character_number, char *text );
		void NarrowCharTextLatin( int character_number, char *text );

		EBAppendixSubbook( EBAppendix^ appendix );

	public:

	};
}
