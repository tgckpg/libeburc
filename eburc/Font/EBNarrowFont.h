#pragma once
#include "eburc/Font/EBFont.h"
#include "eburc/Helpers/FileName.h"

namespace libeburc
{
	ref class EBSubbook;
	ref class EBNarrowFont sealed :
		public EBFont
	{
	public:
		EBNarrowFont( EBSubbook^ subbook ) : EBFont( subbook ) { };
	};
}
