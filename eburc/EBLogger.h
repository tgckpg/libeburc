#pragma once
#include <pch.h>

using namespace Platform;

namespace libeburc
{
	public delegate void EBLogHandler( String^ book );
	public ref class EBLogger sealed
	{
	internal:
		static void Log( const wchar_t* Mesg );
		static void Log( const char* Mesg );
	public:
		static void RegisterHandler( EBLogHandler^ Handler );
	};
}
