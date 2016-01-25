#pragma once
#include <eburc/defs.h>
#include <eburc/Objects/EBHook.h>

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
