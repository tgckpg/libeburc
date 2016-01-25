#pragma once
#include <eburc/defs.h>
#include <eburc/Contexts/EBContext.h>

namespace libeburc
{
	ref class EBTextContext sealed : EBContext
	{
	internal:
		/*
		 * Current text content type.
		 * The context is not active, if this code is EB_TEXT_INVALID.
		 */
		EBTextCode code;

		/*
		 * Current offset pointer of the START or HONMON file.
		 */
		off_t location;

		/*
		 * The current point of a buffer on which text is written.
		 */
		char *out;

		/*
		 * Length of `out'.
		 */
		size_t out_rest_length;

		/*
		 * Unprocessed string that a hook function writes on text.
		 */
		char *unprocessed;

		/*
		 * Size of `unprocessed'.
		 */
		size_t unprocessed_size;

		/*
		 * Length of the current output text phrase.
		 */
		size_t out_step;

		/*
		 * Narrow character region flag.
		 */
		int narrow_flag;

		/*
		 * Whether a printable character has been appeared in the current
		 * text content.
		 */
		int printable_count;

		/*
		 * EOF flag of the current subbook.
		 */
		int file_end_flag;

		/*
		 * Status of the current text processing.
		 */
		EBTextStatusCode text_status;

		/*
		 * Skip until `skipcode' appears.
		 */
		int skip_code;

		/*
		 * Stop-code automatically set by EB Library.
		 */
		int auto_stop_code;

		/*
		 * The current candidate word for multi search.
		 */
		char candidate[ EB_MAX_WORD_LENGTH + 1 ];

		/*
		 * Whether the current text point is in the candidate word or not.
		 */
		int is_candidate;

		/*
		 * Whether the current text point is in EBXA-C gaiji area.
		 */
		int ebxac_gaiji_flag;

		void Reset();
	public:
		EBTextContext();
		EBTextContext( EBTextContext^ Context );
	};
}
