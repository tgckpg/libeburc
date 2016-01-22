#include "pch.h"
#include "EBAppendix.h"
#include "EBAppendixSubbook.h"

using namespace libeburc;

EBAppendixSubbook::EBAppendixSubbook( EBAppendix^ book )
{
	ParentBook = book;
}
