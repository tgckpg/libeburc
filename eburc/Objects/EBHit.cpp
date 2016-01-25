#include "pch.h"
#include "eburc/Objects/EBHit.h"

using namespace libeburc;

EBHit::EBHit( EBPosition^ Heading, EBPosition^ Text )
{
	heading = Heading;
	text = Text;
}

EBHit::EBHit( EBHit^ Hit )
{
	heading = ref new EBPosition( Hit->heading->page, Hit->heading->offset );
	text = ref new EBPosition( Hit->text->page, Hit->text->offset );
}