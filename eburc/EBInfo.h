#pragma once

#include <pch.h>
#include <eburc/defs.h>
#include <eburc/Font/EBFont.h>
#include <eburc/Objects/CharRange.h>

using namespace concurrency;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Storage;

namespace libeburc
{
	ref class EBBook;
	ref class EBSubbook;

    public ref class EBInfo sealed
    {
        EBInfo();
	internal:
		static CharRange^ NarrowFontRange( EBSubbook^ subbook );
		static CharRange^ WideFontRange( EBSubbook^ subbook );
    public:
		/*
		 * Output disc type.
		 */
		static String^ DiskType( EBBook^ Book );
		/*
		 * Output character code.
		 */
		static String^ CharCode( EBBook^ Book );
		/*
		 * Output supported methods.
		 */
		static Array<String^>^ SearchMethods( EBSubbook^ Subbook );
		/*
		 * Output a font list.
		 */
		static Array<int>^ FontList( EBSubbook^ Subbook );
		/*
		 * Output character range of the narrow font.
		 */
		static IAsyncOperation<CharRange^>^ GetNarrowFontRangeAsync( EBSubbook^ Subbook );
		/*
		 * Output character range of the wide font.
		 */
		static IAsyncOperation<CharRange^>^ GetWideFontRangeAsync( EBSubbook^ Subbook );
    };
}
