#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

/*
 * Book code of which `cache_buffer' records data.
 */
static EBBookCode cache_book_code = EB_BOOK_NONE;
/*
 * Cache data buffer.
 */
static char cache_buffer[ EB_SIZE_PAGE ];
/*
 * Location of cache data loaded in `cache_buffer'.
 */
static off_t cache_location;
/*
 * Length of cache data loaded in `cache_buffer'.
 */
static size_t cache_length;

EBPosition^ EBSubbook::TellText()
{
	return ref new EBPosition(
		ParentBook->text_context->location / EB_SIZE_PAGE + 1
		, ParentBook->text_context->location % EB_SIZE_PAGE
	);
}

void EBSubbook::SeekText( EBPosition^ Pos )
{
	ParentBook->ResetTextContext();
	ParentBook->text_context->code = EBTextCode::EB_TEXT_SEEKED;
	ParentBook->text_context->location
		= ( ( off_t ) Pos->page - 1 ) * EB_SIZE_PAGE + Pos->offset;
}


void EBSubbook::ReadText(
	EBAppendix^ appendix, EBHookSet^ hookset
	, void *container, size_t text_max_length
	, char *text, SSIZE_T *text_length )
{
	/*
	 * Use `eb_default_hookset' when `hookset' is `NULL'.
	 */

	if ( !hookset )
		hookset = ref new EBHookSet();

	/*
	 * Set text mode to `text'.
	 */
	EBBook^ book = ParentBook;
	if ( book->text_context->code == EBTextCode::EB_TEXT_INVALID )
	{
		EBException::Throw( EBErrorCode::EB_ERR_NO_PREV_SEEK );
	}
	else if ( book->text_context->code == EBTextCode::EB_TEXT_SEEKED )
	{
		EBPosition^ position = TellText();
		ParentBook->ResetTextContext();

		if ( menu->start_page <= position->page && position->page <= menu->end_page )
			book->text_context->code = EBTextCode::EB_TEXT_OPTIONAL_TEXT;

		else if ( image_menu->start_page <= position->page && position->page <= image_menu->end_page )
			book->text_context->code = EBTextCode::EB_TEXT_OPTIONAL_TEXT;

		else if ( copyright->start_page <= position->page && position->page <= copyright->end_page )
			book->text_context->code = EBTextCode::EB_TEXT_OPTIONAL_TEXT;

		else
			book->text_context->code = EBTextCode::EB_TEXT_MAIN_TEXT;

		EBHook^ hook = hookset->hooks[ (int) EBHookCode::EB_HOOK_INITIALIZE ];
		if ( hook->function )
		{
			hook->function( this, appendix, container, EB_HOOK_INITIALIZE, 0, NULL );
		}
	}
	else if ( book->text_context->code != EBTextCode::EB_TEXT_MAIN_TEXT
		&& book->text_context->code != EBTextCode::EB_TEXT_OPTIONAL_TEXT )
	{
		EBException::Throw( EBErrorCode::EB_ERR_DIFF_CONTENT );
	}

	ReadTextInternal( appendix, hookset, container,
		text_max_length, text, text_length, 0 );
}

void EBSubbook::ReadTextInternal(
	EBAppendix^ appendix, EBHookSet^ hookset
	, void *container
	, size_t text_max_length, char *text
    , SSIZE_T *text_length, int forward_only )
{
	/*
	 * Initialize variables.
	 */
	EBTextContext^ context = ParentBook->text_context;
	context->out = text;
	context->out_rest_length = text_max_length;

	size_t candidate_length;
	unsigned char *candidate_p;
	if ( context->is_candidate )
	{
		candidate_length = strlen( context->candidate );
		candidate_p = ( unsigned char * ) context->candidate + candidate_length;
	}
	else
	{
		candidate_length = 0;
		candidate_p = NULL;
	}

	/*
	 * If unprocessed string are rest in `context->unprocessed',
	 * copy them to `context->out'.
	 */
	if ( context->unprocessed != NULL )
	{
		if ( !forward_only )
		{
			if ( context->out_rest_length < context->unprocessed_size )
				goto succeeded;
			memcpy_s( context->out, context->unprocessed_size
				, context->unprocessed, context->unprocessed_size );
			context->out += context->unprocessed_size;
			context->out_rest_length -= context->unprocessed_size;
		}
		free( context->unprocessed );
		context->unprocessed = NULL;
		context->unprocessed_size = 0;
	}

	/*
	 * Return immediately if text-end-flag has been set.
	 */
	if ( context->text_status != EBTextStatusCode::EB_TEXT_STATUS_CONTINUED )
		goto succeeded;

	/*
	 * Check for cache data.
	 * If cache data is not what we need, discard it.
	 */
	char *cache_p;
	size_t cache_rest_length;
	if ( ParentBook->code == cache_book_code
		&& cache_location <= context->location
		&& context->location < cache_location + cache_length )
	{
		cache_p = cache_buffer + ( context->location - cache_location );
		cache_rest_length = cache_length - ( context->location - cache_location );
	}
	else
	{
		cache_book_code = EB_BOOK_NONE;
		cache_p = cache_buffer;
		cache_length = 0;
		cache_rest_length = 0;
	}

	for ( ;;)
	{
		size_t in_step = 0;
		context->out_step = 0;
		int argc = 1;

		/*
		 * If it reaches to the near of the end of the cache buffer,
		 * then moves remaind cache text to the beginning of the cache
		 * buffer, and reads a next chunk from a file.
		 */
		if ( cache_rest_length < SIZE_FEW_REST && !context->file_end_flag )
		{
			if ( 0 < cache_rest_length )
				memmove_s( cache_buffer, cache_rest_length, cache_p, cache_rest_length );

			TextZio->LSeekRaw( context->location + cache_rest_length );

			size_t rsize = EB_SIZE_PAGE - cache_rest_length;
			Array<byte>^ buff = ref new Array<byte>( rsize );
			TextZio->Read( rsize, buff );

			memcpy_s( cache_buffer + cache_rest_length, rsize, buff->Data, rsize );

			size_t read_result = rsize;
			if ( read_result < 0 )
			{
				EBException::Throw( EBErrorCode::EB_ERR_FAIL_READ_TEXT );
			}
			else if ( read_result != EB_SIZE_PAGE - cache_rest_length )
			{
				context->file_end_flag = 1;
			}

			cache_book_code = ParentBook->code;
			cache_location = context->location;
			cache_length = cache_rest_length + read_result;
			cache_p = cache_buffer;
			cache_rest_length = cache_length;
		}

		/*
		 * Get 1 byte from the buffer.
		 */
		if ( cache_rest_length < 1 )
		{
			EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
		}
		unsigned char c1 = eb_uint1( cache_p );

		EBHook^ hook = ref new EBHook();
		unsigned int argv[ EB_MAX_ARGV ];
		int argc;

		if ( c1 == 0x1f )
		{
			/*
			 * This is escape sequences.
			 */
			if ( cache_rest_length < 2 )
			{
				EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
			}

			argv[ 0 ] = eb_uint2( cache_p );
			unsigned char c2 = eb_uint1( cache_p + 1 );

			switch ( c2 )
			{
			case 0x02:
				/* beginning of text */
				in_step = 2;
				break;

			case 0x03:
				/* end of text (don't set `in_step') */
				context->text_status = EBTextStatusCode::EB_TEXT_STATUS_HARD_STOP;
				if ( forward_only )
				{
					EBException::Throw( EBErrorCode::EB_ERR_END_OF_CONTENT );
				}
				goto succeeded;

			case 0x04:
				/* beginning of NARROW */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_NARROW ];
				context->narrow_flag = 1;
				break;

			case 0x05:
				/* end of NARROW */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_NARROW ];
				context->narrow_flag = 0;
				break;

			case 0x06:
				/* beginning of subscript */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_SUBSCRIPT ];
				break;

			case 0x07:
				/* end of subscript */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_SUBSCRIPT ];
				break;

			case 0x09:
				/* set indent */
				in_step = 4;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
				}
				argc = 2;
				argv[ 1 ] = eb_uint2( cache_p + 2 );

				if ( 0 < context->printable_count
					&& context->code == EBTextCode::EB_TEXT_MAIN_TEXT )
				{
					if ( eb_is_stop_code( book, appendix, argv[ 0 ], argv[ 1 ] ) )
					{
						context->text_status = EBTextStatusCode::EB_TEXT_STATUS_SOFT_STOP;
						goto succeeded;
					}
				}

				hook = hookset->hooks[ EBHookCode::EB_HOOK_SET_INDENT ];
				break;

			case 0x0a:
				/* newline */
				in_step = 2;
				if ( context->code == EBTextCode::EB_TEXT_HEADING )
				{
					context->text_status = EBTextStatusCode::EB_TEXT_STATUS_SOFT_STOP;
					context->location += in_step;
					goto succeeded;
				}
				hook = hookset->hooks[ EBHookCode::EB_HOOK_NEWLINE ];
				break;

			case 0x0b:
				/* beginning of unicode */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_UNICODE ];
				break;

			case 0x0c:
				/* end of unicode */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_UNICODE ];
				break;

			case 0x0e:
				/* beginning of superscript */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_SUPERSCRIPT ];
				break;

			case 0x0f:
				/* end of superscript */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_SUPERSCRIPT ];
				break;

			case 0x10:
				/* beginning of newline prohibition */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_NO_NEWLINE ];
				break;

			case 0x11:
				/* end of newline prohibition */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_NO_NEWLINE ];
				break;

			case 0x12:
				/* beginning of emphasis */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_EMPHASIS ];
				break;

			case 0x13:
				/* end of emphasis */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_EMPHASIS ];
				break;

			case 0x14:
				in_step = 4;
				context->skip_code = 0x15;
				break;

			case 0x1a: case 0x1b: case 0x1e: case 0x1f:
				/* emphasis; described in JIS X 4081-1996 */
				in_step = 4;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				/* Some old EB books don't take an argument. */
				if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
					&& eb_uint1( cache_p + 2 ) >= 0x1f )
					in_step = 2;
				break;

			case 0x1c:
				if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_JISX0208_GB2312 )
				{
					/* beginning of EBXA-C gaiji */
					in_step = 2;
					hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_EBXAC_GAIJI ];
					context->ebxac_gaiji_flag = 1;
				}
				else
				{
					in_step = 4;
					if ( cache_rest_length < in_step )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
						goto failed;
					}
					/* Some old EB books don't take an argument. */
					if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
						&& eb_uint1( cache_p + 2 ) >= 0x1f )
						in_step = 2;
				}
				break;

			case 0x1d:
				if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_JISX0208_GB2312 )
				{
					/* end of EBXA-C gaiji */
					in_step = 2;
					hook = hookset->hooks[ EBHookCode::EB_HOOK_END_EBXAC_GAIJI ];
					context->ebxac_gaiji_flag = 0;
				}
				else
				{
					in_step = 4;
					if ( cache_rest_length < in_step )
					{
						EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
						goto failed;
					}
					/* Some old EB books don't take an argument. */
					if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
						&& eb_uint1( cache_p + 2 ) >= 0x1f )
						in_step = 2;
				}
				break;

			case 0x32:
				/* beginning of reference to monochrome graphic */
				in_step = 2;
				argc = 4;
				argv[ 1 ] = 0;
				argv[ 2 ] = 0;
				argv[ 3 ] = 0;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_MONO_GRAPHIC ];
				break;

			case 0x39:
				/* beginning of MPEG movie */
				in_step = 46;
				argc = 6;
				argv[ 1 ] = eb_uint4( cache_p + 2 );
				argv[ 2 ] = eb_uint4( cache_p + 22 );
				argv[ 3 ] = eb_uint4( cache_p + 26 );
				argv[ 4 ] = eb_uint4( cache_p + 30 );
				argv[ 5 ] = eb_uint4( cache_p + 34 );
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_MPEG ];
				break;

			case 0x3c:
				/* beginning of inline color graphic */
				in_step = 20;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 4;
				argv[ 1 ] = eb_uint2( cache_p + 2 );
				argv[ 2 ] = eb_bcd4( cache_p + 14 );
				argv[ 3 ] = eb_bcd2( cache_p + 18 );
				if ( argv[ 1 ] >> 8 == 0x00 )
					hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_IN_COLOR_BMP ];
				else
					hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_IN_COLOR_JPEG ];
				break;

			case 0x35: case 0x36: case 0x37: case 0x38: case 0x3a:
			case 0x3b: case 0x3d: case 0x3e: case 0x3f:
				in_step = 2;
				context->skip_code = eb_uint1( cache_p + 1 ) + 0x20;
				break;

			case 0x41:
				/* beginning of keyword */
				in_step = 4;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 2;
				argv[ 1 ] = eb_uint2( cache_p + 2 );

				if ( 0 < context->printable_count
					&& context->code == EBTextCode::EB_TEXT_MAIN_TEXT )
				{
					if ( eb_is_stop_code( book, appendix, argv[ 0 ], argv[ 1 ] ) )
					{
						context->text_status = EBTextStatusCode::EB_TEXT_STATUS_SOFT_STOP;
						goto succeeded;
					}
				}
				if ( context->auto_stop_code < 0 )
					context->auto_stop_code = eb_uint2( cache_p + 2 );

				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_KEYWORD ];
				break;

			case 0x42:
				/* beginning of reference */
				in_step = 4;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				if ( eb_uint1( cache_p + 2 ) != 0x00 )
					in_step -= 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_REFERENCE ];
				break;

			case 0x43:
				/* beginning of an entry of a candidate */
				in_step = 2;
				if ( context->skip_code == SKIP_CODE_NONE )
				{
					context->candidate[ 0 ] = '\0';
					context->is_candidate = 1;
					candidate_length = 0;
					candidate_p = ( unsigned char * ) context->candidate;
				}
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_CANDIDATE ];
				break;

			case 0x44:
				/* beginning of monochrome graphic */
				in_step = 12;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 4;
				argv[ 1 ] = eb_uint2( cache_p + 2 );
				argv[ 2 ] = eb_bcd4( cache_p + 4 );
				argv[ 3 ] = eb_bcd4( cache_p + 8 );
				if ( 0 < argv[ 2 ] && 0 < argv[ 3 ] )
					hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_MONO_GRAPHIC ];
				break;

			case 0x45:
				/* beginning of graphic block */
				in_step = 4;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				if ( eb_uint1( cache_p + 2 ) != 0x1f )
				{
					argc = 2;
					argv[ 1 ] = eb_bcd4( cache_p + 2 );
				}
				else
				{
					in_step = 2;
				}
				break;

			case 0x4a:
				/* beginning of WAVE sound */
				in_step = 18;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 6;
				argv[ 1 ] = eb_uint4( cache_p + 2 );
				argv[ 2 ] = eb_bcd4( cache_p + 6 );
				argv[ 3 ] = eb_bcd2( cache_p + 10 );
				argv[ 4 ] = eb_bcd4( cache_p + 12 );
				argv[ 5 ] = eb_bcd2( cache_p + 16 );
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_WAVE ];
				break;

			case 0x4b:
				/* beginning of paged reference */
				in_step = 8;
				if ( cache_rest_length < in_step + 2 )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 3;
				argv[ 1 ] = eb_bcd4( cache_p + 2 );
				argv[ 2 ] = eb_bcd2( cache_p + 6 );
				if ( cache_p[ 8 ] == 0x1f && cache_p[ 9 ] == 0x6b )
				{
					context->text_status = EBTextStatusCode::EB_TEXT_STATUS_SOFT_STOP;
					hook = hookset->hooks[ EBHookCode::EB_HOOK_GRAPHIC_REFERENCE ];
					in_step = 10;
				}
				else
				{
					hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_GRAPHIC_REFERENCE ];
				}
				break;

			case 0x4c:
				/* beginning of image page */
				in_step = 4;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_IMAGE_PAGE ];
				break;

			case 0x4d:
				/* beginning of color graphic (BMP or JPEG) */
				in_step = 20;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 4;
				argv[ 1 ] = eb_uint2( cache_p + 2 );
				argv[ 2 ] = eb_bcd4( cache_p + 14 );
				argv[ 3 ] = eb_bcd2( cache_p + 18 );
				if ( argv[ 1 ] >> 8 == 0x00 )
					hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_COLOR_BMP ];
				else
					hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_COLOR_JPEG ];
				break;

			case 0x4f:
				/* beginning of clickable area */
				in_step = 34;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 7;
				argv[ 1 ] = eb_bcd2( cache_p + 8 );
				argv[ 2 ] = eb_bcd2( cache_p + 10 );
				argv[ 3 ] = eb_bcd2( cache_p + 12 );
				argv[ 4 ] = eb_bcd2( cache_p + 14 );
				argv[ 5 ] = eb_bcd4( cache_p + 28 );
				argv[ 6 ] = eb_bcd2( cache_p + 32 );
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_CLICKABLE_AREA ];
				break;

			case 0x49: case 0x4e:

				in_step = 2;
				context->skip_code = eb_uint1( cache_p + 1 ) + 0x20;
				break;

			case 0x52:
				/* end of reference to monochrome graphic */
				in_step = 8;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 3;
				argv[ 1 ] = eb_bcd4( cache_p + 2 );
				argv[ 2 ] = eb_bcd2( cache_p + 6 );
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_MONO_GRAPHIC ];
				break;

			case 0x53:
				/* end of EB sound */
				in_step = 10;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				break;

			case 0x59:
				/* end of MPEG movie */
				in_step = 2;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_MPEG ];
				break;

			case 0x5c:
				/* end of inline color graphic */
				in_step = 2;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_IN_COLOR_GRAPHIC ];
				break;

			case 0x61:
				/* end of keyword */
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_KEYWORD ];
				break;

			case 0x62:
				/* end of reference */
				in_step = 8;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 3;
				argv[ 1 ] = eb_bcd4( cache_p + 2 );
				argv[ 2 ] = eb_bcd2( cache_p + 6 );
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_REFERENCE ];
				break;

			case 0x63:
				/* end of an entry of a candidate */
				in_step = 8;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 3;
				argv[ 1 ] = eb_bcd4( cache_p + 2 );
				argv[ 2 ] = eb_bcd2( cache_p + 6 );
				if ( argv[ 1 ] == 0 && argv[ 2 ] == 0 )
					hook = hookset->hooks[ EBHookCode::EB_HOOK_END_CANDIDATE_LEAF ];
				else
					hook = hookset->hooks[ EBHookCode::EB_HOOK_END_CANDIDATE_GROUP ];
				break;

			case 0x64:
				/* end of monochrome graphic */
				in_step = 8;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 3;
				argv[ 1 ] = eb_bcd4( cache_p + 2 );
				argv[ 2 ] = eb_bcd2( cache_p + 6 );
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_MONO_GRAPHIC ];
				break;

			case 0x6b:
				/* end of paged reference */
				in_step = 2;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_GRAPHIC_REFERENCE ];
				break;

			case 0x6a:
				/* end of WAVE sound */
				in_step = 2;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_WAVE ];
				break;

			case 0x6c:
				/* end of image page */
				in_step = 2;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				context->text_status = EBTextStatusCode::EB_TEXT_STATUS_SOFT_STOP;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_IMAGE_PAGE ];
				break;

			case 0x6d:
				/* end of color graphic (BMP or JPEG) */
				in_step = 2;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_COLOR_GRAPHIC ];
				break;

			case 0x6f:
				/* end of clickable area */
				in_step = 2;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_CLICKABLE_AREA ];
				break;

			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75:
			case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b:
			case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85:
			case 0x86: case 0x87: case 0x88: case 0x89: case 0x8a: case 0x8b:
			case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				in_step = 2;
				context->skip_code = eb_uint1( cache_p + 1 ) + 0x20;
				break;

			case 0xd0:
				/* halfwidth local character for UTF-8 text */
				in_step = 4;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}

				argv[ 0 ] = eb_uint2( cache_p + 2 ) | 0x8080;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_NARROW_FONT ];
				if ( forward_only )
				{
					; /* do nothing */
				}
				else if ( hook->function == NULL )
				{
					; /* do nothing */
				}
				else
				{
					hook->function( this, appendix, container, EBHookCode::EB_HOOK_NARROW_FONT, argc, argv );
				}
				break;
			case 0xd1:
				/* fullwidth local character for UTF-8 text */
				in_step = 4;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}

				argv[ 0 ] = eb_uint2( cache_p + 2 ) | 0x8080;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_WIDE_FONT ];
				if ( forward_only )
				{
					; /* do nothing */
				}
				else if ( hook->function == NULL )
				{
					; /* do nothing */
				}
				else
				{
					hook->function( this, appendix, container, EBHookCode::EB_HOOK_WIDE_FONT, argc, argv );
				}
			case 0xe0:
				/* character modification */
				in_step = 4;
				if ( cache_rest_length < in_step )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}
				argc = 2;
				argv[ 1 ] = eb_uint2( cache_p + 2 );
				hook = hookset->hooks[ EBHookCode::EB_HOOK_BEGIN_DECORATION ];

				/* Some old EB books don't take an argument. */
				if ( ParentBook->disc_code != EBDiscCode::EB_DISC_EPWING
					&& eb_uint1( cache_p + 2 ) >= 0x1f )
				{
					in_step = 2;
					hook = ref new EBHook();
				}
				break;

			case 0xe1:
				in_step = 2;
				hook = hookset->hooks[ EBHookCode::EB_HOOK_END_DECORATION ];
				break;

			case 0xe4: case 0xe6: case 0xe8: case 0xea: case 0xec: case 0xee:
			case 0xf0: case 0xf2: case 0xf4: case 0xf6: case 0xf8: case 0xfa:
			case 0xfc: case 0xfe:
				in_step = 2;
				context->skip_code = eb_uint1( cache_p + 1 ) + 0x01;
				break;

			default:
				in_step = 2;
				if ( context->skip_code == eb_uint1( cache_p + 1 ) )
					context->skip_code = SKIP_CODE_NONE;
				break;
			}

			if ( context->skip_code == SKIP_CODE_NONE
				&& hook->function != NULL
				&& !forward_only )
			{
				hook->function( this, appendix, container, hook->code, argc, argv );
			}

			/*
			 * Post process.  Clean a candidate.
			 */
			if ( c2 == 0x63 )
			{
				/* end of an entry of candidate */
				context->is_candidate = 0;
			}

		}
		else if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_ISO8859_1 )
		{
			/*
			 * The book is mainly written in ISO 8859 1.
			 */
			context->printable_count++;

			if ( ( 0x20 <= c1 && c1 < 0x7f ) || ( 0xa0 <= c1 && c1 <= 0xff ) )
			{
				/*
				 * This is an ISO 8859 1 character.
				 */
				in_step = 1;
				argv[ 0 ] = eb_uint1( cache_p );

				if ( context->skip_code == SKIP_CODE_NONE )
				{
					if ( context->is_candidate
						&& candidate_length < EB_MAX_WORD_LENGTH )
					{
						*candidate_p++ = c1 | 0x80;
						*candidate_p = '\0';
						candidate_length++;
					}

					hook = hookset->hooks[ EBHookCode::EB_HOOK_ISO8859_1 ];
					if ( forward_only )
					{
						; /* do nothing */
					}
					else if ( hook->function == NULL )
					{
						error_code = eb_write_text_byte1( book, c1 );
						if ( error_code != EB_SUCCESS )
							goto failed;
					}
					else
					{
						hook->function( this, appendix, container, EBHookCode::EB_HOOK_ISO8859_1, argc, argv );
					}
				}
			}
			else
			{
				/*
				 * This is a local character.
				 */
				if ( cache_rest_length < 2 )
				{
					EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
					goto failed;
				}

				in_step = 2;
				argv[ 0 ] = eb_uint2( cache_p );
				if ( context->skip_code == SKIP_CODE_NONE )
				{
					hook = hookset->hooks[ EBHookCode::EB_HOOK_NARROW_FONT ];
					if ( forward_only )
					{
						; /* do nothing */
					}
					else if ( hook->function == NULL )
					{
						error_code = eb_write_text_byte1( book, c1 );
						if ( error_code != EB_SUCCESS )
							goto failed;
					}
					else
					{
						hook->function( this, appendix, container, EBHookCode::EB_HOOK_NARROW_FONT, argc, argv );
					}
				}
			}

		}
		else if ( ParentBook->character_code == EBCharCode::EB_CHARCODE_UTF8 )
		{
			/*
			 * The book is mainly written in UTF-8.
			 */
			context->printable_count++;

			if ( !( c1 & 0x80 ) )
			{
				argv[ 0 ] = c1;
				in_step = 1;
			}
			else if ( ( cache_rest_length >= 2 ) && !( c1 & 0x20 ) )
			{
				argv[ 0 ] = ( ( c1 & 0x1f ) << 6 ) +
					( ( eb_uint1( cache_p + 1 ) & 0x3f ) );
				in_step = 2;
			}
			else if ( ( cache_rest_length >= 3 ) && !( c1 & 0x10 ) )
			{
				argv[ 0 ] = ( ( c1 & 0x0f ) << 12 ) +
					( ( eb_uint1( cache_p + 1 ) & 0x3f ) << 6 ) +
					( ( eb_uint1( cache_p + 2 ) & 0x3f ) );
				in_step = 3;
			}
			else if ( ( cache_rest_length >= 4 ) && !( c1 & 0x08 ) )
			{
				argv[ 0 ] = ( ( c1 & 0x07 ) << 18 ) +
					( ( eb_uint1( cache_p + 1 ) & 0x3f ) << 12 ) +
					( ( eb_uint1( cache_p + 2 ) & 0x3f ) << 6 ) +
					( ( eb_uint1( cache_p + 3 ) & 0x3f ) );
				in_step = 4;
			}
			else
			{
				EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
				goto failed;
			}

			if ( context->skip_code == SKIP_CODE_NONE )
			{
				if ( context->is_candidate
					&& candidate_length + in_step <= EB_MAX_WORD_LENGTH )
				{
					memcpy( candidate_p, cache_p, in_step );
					candidate_p += in_step;
					*candidate_p = '\0';
					candidate_length += in_step;
				}

				if ( !forward_only )
				{
					if ( in_step == 1 )
					{
						hook = hookset->hooks[ EBHookCode::EB_HOOK_ISO8859_1 ];
						if ( hook->function == NULL )
						{
							error_code = eb_write_text_byte1( book, c1 );
							if ( error_code != EB_SUCCESS )
								goto failed;
						}
						else
						{
							hook->function( this, appendix, container, EBHookCode::EB_HOOK_ISO8859_1, argc, argv );
						}
					}
					else
					{
						hook = hookset->hooks[ EBHookCode::EB_HOOK_UNICODE ];
						if ( hook->function == NULL )
						{
							error_code = eb_write_text( book, cache_p, in_step );
							if ( error_code != EB_SUCCESS )
								goto failed;
						}
						else
						{
							hook->function( this, appendix, container, EBHookCode::EB_HOOK_UNICODE, argc, argv );
						}
					}
				}
			}
		}
		else
		{
			/*
			 * The book is written in JIS X 0208 or JIS X 0208 + GB 2312.
			 */
			context->printable_count++;
			in_step = 2;

			if ( cache_rest_length < 2 )
			{
				EBException::Throw( EBErrorCode::EB_ERR_UNEXP_TEXT );
				goto failed;
			}

			c2 = eb_uint1( cache_p + 1 );

			if ( context->skip_code != SKIP_CODE_NONE )
			{
				/* nothing to be done. */
			}
			else if ( 0x20 < c1 && c1 < 0x7f && 0x20 < c2 && c2 < 0x7f )
			{
				/*
				 * This is a JIS X 0208 KANJI character.
				 */
				argv[ 0 ] = eb_uint2( cache_p ) | 0x8080;

				if ( context->is_candidate
					&& candidate_length < EB_MAX_WORD_LENGTH - 1 )
				{
					*candidate_p++ = c1 | 0x80;
					*candidate_p++ = c2 | 0x80;
					*candidate_p = '\0';
					candidate_length += 2;
				}

				if ( context->ebxac_gaiji_flag )
				{
					hook = hookset->hooks[ EBHookCode::EB_HOOK_EBXAC_GAIJI ];
					if ( forward_only )
					{
						; /* do nothing */
					}
					else if ( hook->function == NULL )
					{
						error_code = eb_write_text_byte2( book, c1 | 0x80,
							c2 | 0x80 );
						if ( error_code != EB_SUCCESS )
							goto failed;
					}
					else
					{
						hook->function( this, appendix, container, EBHookCode::EB_HOOK_EBXAC_GAIJI, 0, argv );
					}
				}
				else if ( context->narrow_flag )
				{
					hook = hookset->hooks[ EBHookCode::EB_HOOK_NARROW_JISX0208 ];
					if ( forward_only )
					{
						; /* do nothing */
					}
					else if ( hook->function == NULL )
					{
						error_code = eb_write_text_byte2( book, c1 | 0x80,
							c2 | 0x80 );
						if ( error_code != EB_SUCCESS )
							goto failed;
					}
					else
					{
						hook->function( this, appendix, container, EBHookCode::EB_HOOK_NARROW_JISX0208, 0, argv );
					}
				}
				else
				{
					hook = hookset->hooks[ EBHookCode::EB_HOOK_WIDE_JISX0208 ];
					if ( forward_only )
					{
						; /* do nothing */
					}
					else if ( hook->function == NULL )
					{
						error_code = eb_write_text_byte2( book, c1 | 0x80,
							c2 | 0x80 );
						if ( error_code != EB_SUCCESS )
							goto failed;
					}
					else
					{
						hook->function( this, appendix, container, EBHookCode::EB_HOOK_WIDE_JISX0208, argc, argv );
					}
				}
			}
			else if ( 0x20 < c1 && c1 < 0x7f && 0xa0 < c2 && c2 < 0xff )
			{
				/*
				 * This is a GB 2312 HANJI character.
				 */
				argv[ 0 ] = eb_uint2( cache_p ) | 0x8000;

				if ( context->is_candidate
					&& candidate_length < EB_MAX_WORD_LENGTH - 1 )
				{
					*candidate_p++ = c1 | 0x80;
					*candidate_p++ = c2;
					*candidate_p = '\0';
					candidate_length += 2;
				}

				hook = hookset->hooks[ EBHookCode::EB_HOOK_GB2312 ];
				if ( forward_only )
				{
					; /* do nothing */
				}
				else if ( hook->function == NULL )
				{
					error_code = eb_write_text_byte2( book, c1 | 0x80, c2 );
					if ( error_code != EB_SUCCESS )
						goto failed;
				}
				else
				{
					hook->function( this, appendix, container, EBHookCode::EB_HOOK_GB2312, 0, argv );
				}
			}
			else if ( 0xa0 < c1 && c1 < 0xff && 0x20 < c2 && c2 < 0x7f )
			{
				/*
				 * This is a local character.
				 */
				argv[ 0 ] = eb_uint2( cache_p );

				if ( context->narrow_flag )
				{
					hook = hookset->hooks[ EBHookCode::EB_HOOK_NARROW_FONT ];
					if ( forward_only )
					{
						; /* do nothing */
					}
					else if ( hook->function == NULL )
					{
						error_code = eb_write_text_byte2( book, c1, c2 );
						if ( error_code != EB_SUCCESS )
							goto failed;
					}
					else
					{
						hook->function( this, appendix, container, EBHookCode::EB_HOOK_NARROW_FONT, argc, argv );
					}
				}
				else
				{
					hook = hookset->hooks[ EBHookCode::EB_HOOK_WIDE_FONT ];
					if ( forward_only )
					{
						; /* do nothing */
					}
					else if ( hook->function == NULL )
					{
						error_code = eb_write_text_byte2( book, c1, c2 );
						if ( error_code != EB_SUCCESS )
							goto failed;
					}
					else
					{
						hook->function( this, appendix, container, EBHookCode::EB_HOOK_WIDE_FONT, argc, argv );
					}
				}
			}
		}

		/*
		 * Update variables.
		 */
		cache_p += in_step;
		cache_rest_length -= in_step;
		context->location += in_step;
		in_step = 0;

		/*
		 * Break if an unprocessed character is remained.
		 */
		if ( context->unprocessed != NULL )
			break;
		/*
		 * Break if EBTextStatusCode::EB_TEXT_STATUS_SOFT_STOP is set.
		 */
		if ( context->text_status == EBTextStatusCode::EB_TEXT_STATUS_SOFT_STOP )
			break;
	}

succeeded:
	if ( !forward_only )
	{
		*text_length = ( context->out - text );
		*( context->out ) = '\0';
	}

	return;

	/*
	 * An error occurs...
	 * Discard cache if read error occurs.
	 */
failed:
	if ( !forward_only )
	{
		*text_length = -1;
		*text = '\0';
	}
}
