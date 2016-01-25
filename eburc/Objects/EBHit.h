#pragma once
#include <eburc/defs.h>
#include <eburc/Objects/EBPosition.h>

namespace libeburc
{
	/*
	 * In a word search, heading and text locations of a matched entry
	 * are stored.
	 */
	public ref class EBHit sealed
	{
	internal:
		EBPosition^ heading;
		EBPosition^ text;
		EBHit( EBPosition^ Heading, EBPosition^ Text );
		EBHit( EBHit^ Hit );
	public:
		/// <summary>
		/// Heading position.
		/// </summary>
		property EBPosition^ Heading { EBPosition^ get() { return heading; } }

		/// <summary>
		/// Text position.
		/// </summary>
		property EBPosition^ Text { EBPosition^ get() { return text; } }
	};
}
