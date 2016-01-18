#pragma once

#include <pch.h>
#include <EBBook.h>

using namespace Windows::Storage;

namespace libeburc
{
    public ref class EBInfo sealed
    {
    public:
        EBInfo( EBBook^ Book );
    };
}
