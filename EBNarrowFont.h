#pragma once
#include "EBFont.h"
#include "FileName.h"

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
