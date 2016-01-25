#pragma once

#include <pch.h>
#include <eburc/defs.h>

namespace libeburc
{
	ref class EBException sealed
	{
	public:
		static void Throw(EBErrorCode Code);
	};
}
