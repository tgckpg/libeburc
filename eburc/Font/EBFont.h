#pragma once

#include <pch.h>
#include <eburc/defs.h>
#include <eburc/Zio.h>
/*
 * Font types.
 */
#define EB_FONT_16		0
#define EB_FONT_24		1
#define EB_FONT_30		2
#define EB_FONT_48		3
#define EB_FONT_INVALID		-1
/*
 * Font height.
 */
#define EB_HEIGHT_FONT_16		16
#define EB_HEIGHT_FONT_24		24
#define EB_HEIGHT_FONT_30		30
#define EB_HEIGHT_FONT_48		48

using namespace Windows::Storage;

namespace libeburc
{
	ref class EBSubbook;

	ref class EBFont
	{
	internal:
		/*
		 * Font Code.
		 * This font is not available, if the code is EB_FONT_INVALID.
		 */
		EBFontCode font_code;

		/*
		 * Whether the object has been initialized.
		 */
		int initialized;

		/*
		 * Character numbers of the start and end of the font.
		 */
		int start;
		int end;

		/*
		 * Page number of the start page of the font data.
		 * Used in EB* only. (In EPWING, it is alyways 1).
		 */
		int page;

		/*
		 * File name of the font. (EPWING only)
		 */
		char file_name[ EB_MAX_FILE_NAME_LENGTH + 1 ];

		/*
		 * Font data cache.
		 */
		char *glyphs;

		/*
		 * Compression Information.
		 */
		Zio^ zio;

		void Open();
		void LoadHeaders();

		EBSubbook^ subbook;

		IStorageFile^ FontFile;

		EBFont( EBSubbook^ subbook );

		static int FontHeight2( EBFontCode fcode );
	};
}
