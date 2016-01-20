#pragma once
#include "EBFont.h"

namespace libeburc
{
	ref class EBSubbook;
	ref class EBWideFont sealed :
		public EBFont
	{
	internal:
		 void Open() override;
		 void LoadHeaders() override;
	public:
		EBWideFont( EBSubbook^ subbook ) : EBFont( subbook ) { };
	};
}
