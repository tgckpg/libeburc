#pragma once

#include <pch.h>
#include <eburc/defs.h>

namespace libeburc
{
	public ref class EBException sealed
	{
	internal:
		static void Throw( EBErrorCode Code );
	public:
		static property EBErrorCode LastError { EBErrorCode get(); }
	};
}
