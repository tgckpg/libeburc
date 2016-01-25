#include "pch.h"
#include "EBBinaryContext.h"

using namespace libeburc;

EBBinaryContext::EBBinaryContext()
{
	code = EBBinaryCode::EB_BINARY_INVALID;
	zio = nullptr;
	location = -1;
	size = 0;
	cache_length = 0;
	cache_offset = 0;
	width = 0;
}
