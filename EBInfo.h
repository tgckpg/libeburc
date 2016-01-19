#pragma once

#include <pch.h>
#include <EBBook.h>

using namespace Platform;
using namespace Windows::Storage;

namespace libeburc
{
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
    };
}
