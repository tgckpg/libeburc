#pragma once

#include <pch.h>
#include <eburc/defs.h>

using namespace Platform;

namespace libeburc
{
	public ref class EBException sealed
	{
	internal:
		static void Throw( EBErrorCode Code );
		static void Throw( EBErrorCode Code, const wchar_t* Mesg );
	public:
		static property EBErrorCode LastError { EBErrorCode get(); }
		static property String^ LastMessage { String^ get(); }
	};
}
