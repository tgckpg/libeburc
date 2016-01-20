#pragma once
#include "EBFont.h"
#include "FileName.h"

namespace libeburc
{
	ref class EBSubbook;
	ref class EBNarrowFont sealed :
		public EBFont
	{
	internal:
		void Open() override;
		void LoadHeaders() override;
	public:
		EBNarrowFont( EBSubbook^ subbook ) : EBFont( subbook ) { };
	};
}
