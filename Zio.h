#pragma once

#include <pch.h>
#include <defs.h>
#include <EBException.h>
#include <robuffer.h>
#include <build-post.h>

#define ZIO_MAX_EBZIP_LEVEL             5
#define ZIO_SIZE_EBZIP_HEADER           22

using namespace concurrency;
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;

namespace libeburc
{
	/// <summary>
	/// Compression type codes.
	/// </summary>
	public enum class ZioCode
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
	public enum class ZioHuffmanNode
	{
		ZIO_HUFFMAN_NODE_INTERMEDIATE,
		ZIO_HUFFMAN_NODE_EOF,
		ZIO_HUFFMAN_NODE_LEAF8,
		ZIO_HUFFMAN_NODE_LEAF16,
		ZIO_HUFFMAN_NODE_LEAF32,
	};

	public ref class Zio sealed
	{
	internal:
		/*
		 * ID.
		 */
		int id;

		/*
		 * Source File
		 */
		IStorageFile^ SrcFile;

		/*
		 * Zio type. (PLAIN, EBZIP, EPWING, EPWING6 or SEBXA)
		 */
		ZioCode Code;

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
		 * LSeekRaw Location
		 */
		off_t posx;

		/*
		 * Low-level read function.
		 *
		 * If `zio->file' is socket, it calls ebnet_read().  Otherwise it calls
		 * the read() system call.
		 * *Does not support ebnet
		 */
		void ReadRaw( size_t length, WriteOnlyArray<byte>^ buffer );
		/*
		 * Uncompress an ebzip'ped slice.
		 *
		 * If it succeeds, 0 is returned.  Otherwise, -1 is returned.
		 */
		byte* UnzipSlice( size_t zipped_slice_size );

		/*
		 * Open an non-compressed file.
		 */
		void OpenPlain();

		/*
		 * Open an EBZIP compression file.
		 */
		void OpenEbZip();

		/*
		 * Low-level seek function.
		 *
		 * If `zio->file' is socket, it calls ebnet_close().  Otherwise it calls
		 * the close() system call.
		 */
		void LSeekRaw( off_t offset );

		/*
		 * Read data from `zio' file.
		 */
		void Read( size_t length, WriteOnlyArray<byte>^ buffer );
		/*
		 * Read data from the `zio' file compressed with the ebzip compression
		 * format.
		 */
		void ReadEBZip( size_t length, WriteOnlyArray<byte>^ buffer );

		static ZioCode Hint( int catolog_hint_value );

		Zio( IStorageFile^ File, ZioCode ZCode );
		Zio(); // For definition

	public:

		static IAsyncOperation<Zio^>^ OpenAsync( IStorageFile^ File, ZioCode ZCode );
	};
}
