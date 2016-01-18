#pragma once

#include <pch.h>
#include <defs.h>

namespace libeburc
{
	ref class EBException sealed
	{
	public:
		static void Throw(EBErrorCode Code);
	};
}
