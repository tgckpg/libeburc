#pragma once
#include "eburc/Font/EBFont.h"

namespace libeburc
{
	ref class EBSubbook;
	ref class EBWideFont sealed :
		public EBFont
	{
	public:
		EBWideFont( EBSubbook^ subbook ) : EBFont( subbook ) { };
	};
}
