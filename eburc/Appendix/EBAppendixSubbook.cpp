#include "pch.h"
#include "eburc/Appendix/EBAppendix.h"
#include "eburc/Appendix/EBAppendixSubbook.h"

using namespace libeburc;

EBAppendixSubbook::EBAppendixSubbook( EBAppendix^ book )
{
	ParentBook = book;
}
