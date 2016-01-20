#pragma once

#include <pch.h>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Storage;

namespace libeburc
{
	ref class EBBook;
	ref class EBSubbook;

    public ref class EBInfo sealed
    {
        EBInfo();
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
    };
}
