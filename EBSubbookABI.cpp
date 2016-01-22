#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

#define MAXLEN_TEXT 1023

String^ EBSubbook::GetPage( EBPosition^ Pos )
{
	SeekText( Pos );

	SSIZE_T text_length;
	char text[ MAXLEN_TEXT + 1 ];
	ReadText( nullptr, nullptr, nullptr, MAXLEN_TEXT, text, &text_length );

	return ref new String();
}