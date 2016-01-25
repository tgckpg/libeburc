#pragma once

#include <eburc/defs.h>
#include <eburc/Contexts/EBContext.h>
#include <eburc/Zio.h>

namespace libeburc
{

	ref class EBBinaryContext sealed : EBContext
	{
		/*
		 * Binary type ID.
		 * The context is not active, if this code is EB_BINARY_INVALID.
		 */
		EBBinaryCode code;

		/*
		 * Compress information.
		 */
		Zio^ zio;

		/*
		 * Location of the the binary data, relative to the start of the file.
		 */
		off_t location;

		/*
		 * Data size.
		 * Size zero means that the binary has no size information.
		 */
		size_t size;

		/*
		 * The current offset of binary data.
		 */
		size_t offset;

		/*
		 * Cache buffer.
		 */
		char cache_buffer[ EB_SIZE_BINARY_CACHE_BUFFER ];

		/*
		 * Length of cached data.
		 */
		size_t cache_length;

		/*
		 * Current offset of cached data.
		 */
		size_t cache_offset;

		/*
		 * Width of Image. (monochrome graphic only)
		 */
		int width;
	public:
		EBBinaryContext();
	};
}
