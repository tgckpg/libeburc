#pragma once
#include <defs.h>
#include <EBHook.h>

using namespace Windows::Foundation::Collections;

namespace libeburc
{
	public ref class EBHookSet sealed
	{
		void BindDefaultHooks();
	internal:
		EBHook^ hooks[ EB_NUMBER_OF_HOOKS ];
		EBHookSet();
	public:
		EBHookSet( IIterable<EBHook^>^ Hooks );
	};
}
