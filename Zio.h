#pragma once

#include <pch.h>
#include <defs.h>

using namespace Windows::Storage;
using namespace Windows::Foundation;

namespace libeburc
{
	/// <summary>
	/// Compression type codes.
	/// </summary>
	enum class ZioCode
	{
		ZIO_PLAIN,
		ZIO_EBZIP1,
		ZIO_EPWING,
		ZIO_EPWING6,
		ZIO_SEBXA,
		ZIO_INVALID,
		ZIO_REOPEN,
	};

	/// <summary>
	/// Huffman node types.
	/// </summary>
	enum class ZioHuffmanNode
	{
		ZIO_HUFFMAN_NODE_INTERMEDIATE,
		ZIO_HUFFMAN_NODE_EOF,
		ZIO_HUFFMAN_NODE_LEAF8,
		ZIO_HUFFMAN_NODE_LEAF16,
		ZIO_HUFFMAN_NODE_LEAF32,
	};

	ref class Zio sealed
	{
		/*
		 * ID.
		 */
		int id;

		/*
		 * Zio type. (PLAIN, EBZIP, EPWING, EPWING6 or SEBXA)
		 */
		ZioCode code;

		/*
		 * File descriptor.
		 */
		int file;

		/*
		 * Current location.
		 */
		off_t location;

		/*
		 * Size of an uncompressed file.
		 */
		off_t file_size;

		/*
		 * Slice size of an EBZIP compressed file.
		 */
		size_t slice_size;

		/*
		 * Compression level. (EBZIP compression only)
		 */
		int zip_level;

		/*
		 * Length of an index. (EBZIP compression only)
		 */
		int index_width;

		/*
		 * Adler-32 check sum of an uncompressed file. (EBZIP compression only)
		 */
		unsigned int crc;

		/*
		 * mtime of an uncompressed file. (EBZIP compression only)
		 */
		time_t mtime;

		/*
		 * Location of an index table. (EPWING and S-EBXA compression only)
		 */
		off_t index_location;

		/*
		 * Length of an index table. (EPWING and S-EBXA compression only)
		 */
		size_t index_length;

		/*
		 * Location of a frequency table. (EPWING compression only)
		 */
		off_t frequencies_location;

		/*
		 * Length of a frequency table. (EPWING compression only)
		 */
		size_t frequencies_length;

		/*
		 * Huffman tree nodes. (EPWING compression only)
		 */
		ZioHuffmanNode *huffman_nodes;

		/*
		 * Root node of a Huffman tree. (EPWING compression only)
		 */
		ZioHuffmanNode *huffman_root;

		/*
		 * Region of compressed pages. (S-EBXA compression only)
		 */
		off_t zio_start_location;
		off_t zio_end_location;

		/*
		 * Add this value to offset written in index. (S-EBXA compression only)
		 */
		off_t index_base;

		/*
		 * ebnet mode flag.
		 */
		int is_ebnet;

		/*
		 * Open an EBZIP compression file.
		 */
		int OpenEbZip();

		Zio( IStorageFile^ File, ZioCode ZCode );

	public:

		static IAsyncOperation<Zio^>^ Open( IStorageFile^ File, ZioCode ZCode );
	};
}
