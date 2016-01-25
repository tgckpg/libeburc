#include "pch.h"
#include "eburc/Book/EBBook.h"

using namespace libeburc;

void EBBook::WriteText( const char *stream, size_t stream_length )
{
	char *reallocated;

	/*
	 * If the text buffer has enough space to write `stream',
	 * save the stream in `text_context->unprocessed'.
	 */
	if ( text_context->unprocessed != NULL )
	{
		reallocated = ( char * ) realloc( text_context->unprocessed,
			text_context->unprocessed_size + stream_length );
		if ( reallocated == NULL )
		{
			free( text_context->unprocessed );
			text_context->unprocessed = NULL;
			text_context->unprocessed_size = 0;
			EBException::Throw( EBErrorCode::EB_ERR_MEMORY_EXHAUSTED );
		}
		memcpy_s(
			reallocated + text_context->unprocessed_size, stream_length
			, stream, stream_length );
		text_context->unprocessed = reallocated;
		text_context->unprocessed_size += stream_length;

	}
	else if ( text_context->out_rest_length < stream_length )
	{
		text_context->unprocessed
			= ( char * ) malloc( text_context->out_step + stream_length );
		if ( text_context->unprocessed == NULL )
		{
			EBException::Throw( EBErrorCode::EB_ERR_MEMORY_EXHAUSTED );
		}

		text_context->unprocessed_size = text_context->out_step + stream_length;

		memcpy_s(
			text_context->unprocessed, text_context->out_step
			, text_context->out - text_context->out_step, text_context->out_step );

		memcpy_s(
			text_context->unprocessed + text_context->out_step, stream_length
			, stream, stream_length );

		text_context->out -= text_context->out_step;
		text_context->out_step = 0;

	}
	else
	{
		memcpy_s( text_context->out, stream_length, stream, stream_length );
		text_context->out += stream_length;
		text_context->out_rest_length -= stream_length;
		text_context->out_step += stream_length;
	}
}

void EBBook::WriteTextByte1( int byte1 )
{
	char stream[ 1 ];

	/*
	 * If the text buffer has enough space to write `byte1',
	 * save the byte in `text_context->unprocessed'.
	 */
	if ( text_context->unprocessed != NULL
		|| text_context->out_rest_length < 1 )
	{
		*( unsigned char * ) stream = byte1;
		WriteText( stream, 1 );
	}
	else
	{
		*( text_context->out ) = byte1;
		text_context->out++;
		text_context->out_rest_length--;
		text_context->out_step++;
	}
}

void EBBook::WriteTextByte2( int byte1, int byte2 )
{
	char stream[ 2 ];

	/*
	 * If the text buffer has enough space to write `byte1' and `byte2',
	 * save the bytes in `book->text_context->unprocessed'.
	 */
	if ( text_context->unprocessed != NULL
		|| text_context->out_rest_length < 2 )
	{
		*( unsigned char * ) stream = byte1;
		*( unsigned char * ) ( stream + 1 ) = byte2;
		WriteText( stream, 2 );
	}
	else
	{
		*( text_context->out ) = byte1;
		text_context->out++;
		*( text_context->out ) = byte2;
		text_context->out++;
		text_context->out_rest_length -= 2;
		text_context->out_step += 2;
	}
}

void EBBook::WriteTextString( const char *string )
{
	/*
	 * If the text buffer has enough space to write `sting',
	 * save the string in `book->text_context.unprocessed'.
	 */
	size_t string_length = strlen( string );

	if ( text_context->unprocessed != NULL
		|| text_context->out_rest_length < string_length )
	{
		WriteText( string, string_length );
	}
	else
	{
		memcpy_s( text_context->out, string_length, string, string_length );
		text_context->out += string_length;
		text_context->out_rest_length -= string_length;
		text_context->out_step += string_length;
	}
}
