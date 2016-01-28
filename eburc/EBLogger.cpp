#include "pch.h"
#include "EBLogger.h"
#include "Helpers/Utils.h"

using namespace std;
using namespace libeburc;


static EBLogHandler^ handler = nullptr;

void EBLogger::RegisterHandler( EBLogHandler^ Handler )
{
	handler = Handler;
	Log( L"Handler Registered" );
}

void EBLogger::Log( const wchar_t* mesg )
{
	if ( handler == nullptr ) return;

	handler( ref new String( mesg ) );
}

void EBLogger::Log( const char* mesg )
{
	wstring wmsg = Utils::ToWStr( mesg );
	Log( wmsg.c_str() );
}