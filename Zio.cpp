#include "pch.h"
#include "Zio.h"

using namespace libeburc;
using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;

/*
 * NULL Zio ID.
 */
#define ZIO_ID_NONE         -1

/*
 * Zio object counter.
 */
static int zio_counter = 0;
/*
 * Zio ID which caches data in `cache_buffer'.
 */
static int cache_zio_id = ZIO_ID_NONE;
/*
 * Offset of the beginning of the cached data `cache_buffer'.
 */
static off_t cache_location;
/*
 * Buffer for caching uncompressed data.
 */
static byte* cache_buffer = nullptr;


/*
 * Get an unsigned value from an octet stream buffer.
 */
#define zio_uint1(p) (*(const unsigned char *)(p))

#define zio_uint2(p) ((*(const unsigned char *)(p) << 8) \
        + (*(const unsigned char *)((p) + 1)))

#define zio_uint3(p) ((*(const unsigned char *)(p) << 16) \
        + (*(const unsigned char *)((p) + 1) << 8) \
        + (*(const unsigned char *)((p) + 2)))

#define zio_uint4(p) (((off_t) *(const unsigned char *)(p) << 24) \
        + (*(const unsigned char *)((p) + 1) << 16) \
        + (*(const unsigned char *)((p) + 2) << 8) \
        + (*(const unsigned char *)((p) + 3)))

#define zio_uint5(p) (((off_t) (*(const unsigned char *)(p)) << 32) \
	+ ((off_t) (*(const unsigned char *)((p) + 1)) << 24) \
	+ (*(const unsigned char *)((p) + 2) << 16) \
	+ (*(const unsigned char *)((p) + 3) << 8) \
	+ (*(const unsigned char *)((p) + 4)))

/*
 * Size of a page (The term `page' means `block' in JIS X 4081).
 */
#define ZIO_SIZE_PAGE			2048

Zio::Zio() {}

Zio::Zio( IStorageFile^ File, ZioCode ZCode )
{
	SrcFile = File;
	Code = ZCode;
	location = 0;

	int result;

	switch ( Code )
	{
/*
	case ZIO_REOPEN:
		result = zio_reopen( zio, file_name );
		break;
*/
	case ZioCode::ZIO_PLAIN:
	case ZioCode::ZIO_SEBXA:
		OpenPlain();
		break;
	case ZioCode::ZIO_EBZIP1:
		OpenEbZip();
		break;
	case ZioCode::ZIO_EPWING:
		throw ref new NotImplementedException( "No such disc type" );
		// result = zio_open_epwing( zio, file_name );
		break;
	case ZioCode::ZIO_EPWING6:
		throw ref new NotImplementedException( "No such disc type" );
		// result = zio_open_epwing6( zio, file_name );
		break;
	default:
		throw ref new NotImplementedException( "No such disc type" );
	}
}

void Zio::ReadRaw( size_t length, WriteOnlyArray<byte>^ buffer )
{
	task<IRandomAccessStream^> RandStream( SrcFile->OpenAsync( FileAccessMode::Read ) );
	RandStream.then( [&] ( IRandomAccessStream^ RStream )
	{
		// Create a buffer if buffer not set
		RStream->Seek( posx );

		DataReader^ reader = ref new DataReader( RStream );
		task<unsigned int> RLoad( reader->LoadAsync( length ) );
		RLoad.then( [ & ] ( unsigned int t )
		{
			reader->ReadBytes( buffer );
		} ).wait();
		reader->DetachStream();

		posx += length;
	} ).wait();
}

void Zio::LSeekRaw( off_t offset )
{
	posx = offset;
}

void Zio::Read( size_t length, WriteOnlyArray<byte>^ buffer )
{
	switch ( Code )
	{
	case ZioCode::ZIO_PLAIN:
		return ReadRaw( length, buffer );
	case ZioCode::ZIO_EBZIP1:
		return ReadEBZip( length, buffer );
	case ZioCode::ZIO_EPWING:
		throw ref new NotImplementedException( "No such disc type" );
		// return ReadEPWING( zio, file_name );
	case ZioCode::ZIO_EPWING6:
		throw ref new NotImplementedException( "No such disc type" );
		// return result = ReadEPWing6( zio, file_name );
	case ZioCode::ZIO_SEBXA:
		throw ref new NotImplementedException( "No such disc type" );
		// return ReadSebXA( zio, file_name );
	}
}

void Zio::ReadEBZip( size_t length, WriteOnlyArray<byte>^ ibuffer )
{
	byte* buffer;
	SSIZE_T read_length = 0;
	size_t zipped_slice_size;
	off_t slice_location;
	off_t next_slice_location;
	int n;

	LOG( ( "in: zio_read_ebzip(zio=%d, length=%ld)", ( int ) id,
		( long ) length ) );

	/*
	 * Read data.
	 */
	while ( read_length < length ) {
		if ( file_size <= location )
			goto succeeded;

		/*
		 * If data in `cache_buffer' is out of range, read data from
		 * `file'.
		 */
		if ( cache_zio_id != id
			|| location < cache_location
			|| cache_location + slice_size <= location ) {

			cache_zio_id = ZIO_ID_NONE;
			cache_location = location - ( location % slice_size );

			/*
			 * Get buffer location and size from index table in `file'.
			 */
			LSeekRaw( location / slice_size * index_width + ZIO_SIZE_EBZIP_HEADER );

			// ORG: if (zio_read_raw(zio, temporary_buffer, zio->index_width * 2) != zio->index_width * 2)
			Array<byte>^ buff = ref new Array<byte>( index_width * 2 );
			ReadRaw( index_width * 2, buff );
			byte* temporary_buffer = buff->Data;


			switch ( index_width ) {
			case 2:
				slice_location = zio_uint2( temporary_buffer );
				next_slice_location = zio_uint2( temporary_buffer + 2 );
				break;
			case 3:
				slice_location = zio_uint3( temporary_buffer );
				next_slice_location = zio_uint3( temporary_buffer + 3 );
				break;
			case 4:
				slice_location = zio_uint4( temporary_buffer );
				next_slice_location = zio_uint4( temporary_buffer + 4 );
				break;
			case 5:
				slice_location = zio_uint5( temporary_buffer );
				next_slice_location = zio_uint5( temporary_buffer + 5 );
				break;
			default:
				goto failed;
			}
			zipped_slice_size = next_slice_location - slice_location;

			if ( next_slice_location <= slice_location
				|| slice_size < zipped_slice_size )
				goto failed;

			/*
			 * Read a compressed slice from `file' and uncompress it.
			 * The data is not compressed if its size is equals to
			 * slice size.
			 * ORG: if (zio_lseek_raw(zio, slice_location, SEEK_SET) < 0)
			 */
			LSeekRaw( slice_location );

			// ORG: 
			cache_buffer = UnzipSlice( zipped_slice_size );

			cache_zio_id = id;
		}

		/*
		 * Copy data from `cache_buffer' to `buffer'.
		n = slice_size - ( location % slice_size );
		if ( length - read_length < n )
			n = length - read_length;
		if ( file_size - location < n )
			n = file_size - location;
		memcpy( buffer + read_length,
			cache_buffer + ( location % slice_size ), n );
		read_length += n;
		location += n;
		 */
	}

succeeded:
	throw ref new NotImplementedException();

	/*
	 * An error occurs...
	 */
failed:
	throw ref new NotImplementedException();
}

byte* Zio::UnzipSlice( size_t zipped_slice_size )
{
	byte* out_buffer = nullptr;
	throw ref new NotImplementedException();
	/*
	char in_buffer[ ZIO_SIZE_PAGE ];
	z_stream stream;
	size_t read_length;
	int z_result;

	LOG( ( "in: zio_unzip_slice_ebzip1(zio=%d, zipped_slice_size=%ld)",
		( int ) id, ( long ) zipped_slice_size ) );

	if ( slice_size == zipped_slice_size ) {
		/*
		 * The input slice is not compressed.
		 * Read the target page in the slice.
		 * ORG. if ( zio_read_raw( zio, out_buffer, zipped_slice_size ) != zipped_slice_size )
		 *
		out_buffer = ReadRaw( zipped_slice_size );
	}
	else
	{
		/*
		 * The input slice is compressed.
		 * Read and uncompress the target page in the slice.
		 *
		stream.zalloc = NULL;
		stream.zfree = NULL;
		stream.opaque = NULL;

		if ( inflateInit( &stream ) != Z_OK )
			goto failed;

		stream.next_in = ( Bytef * ) in_buffer;
		stream.avail_in = 0;
		stream.next_out = ( Bytef * ) out_buffer;
		stream.avail_out = slice_size;

		while ( stream.total_out < slice_size ) {
			if ( 0 < stream.avail_in )
				memmove( in_buffer, stream.next_in, stream.avail_in );

			if ( zipped_slice_size - stream.total_in < ZIO_SIZE_PAGE ) {
				read_length = zipped_slice_size - stream.total_in
					- stream.avail_in;
			}
			else {
				read_length = ZIO_SIZE_PAGE - stream.avail_in;
			}

			if ( zio_read_raw( zio, in_buffer + stream.avail_in,
				read_length ) != read_length )
				goto failed;

			stream.next_in = ( Bytef * ) in_buffer;
			stream.avail_in += read_length;
			stream.avail_out = slice_size - stream.total_out;

			z_result = inflate( &stream, Z_SYNC_FLUSH );
			if ( z_result == Z_STREAM_END ) {
				break;
			}
			else if ( z_result != Z_OK && z_result != Z_BUF_ERROR ) {
				goto failed;
			}
		}

		inflateEnd( &stream );
	}

	LOG( ( "out: zio_unzip_slice_ebzip1() = %d", 0 ) );
	return 0;

	/*
	 * An error occurs...
	 *
failed:
	LOG( ( "out: zio_unzip_slice_ebzip1() = %d", -1 ) );
	inflateEnd( &stream );
	return -1;
	*/
}

void Zio::OpenPlain()
{
	Code = ZioCode::ZIO_PLAIN;
	slice_size = ZIO_SIZE_PAGE;

	try
	{
		task<FileProperties::BasicProperties^> PropOps( SrcFile->GetBasicPropertiesAsync() );
		PropOps.wait();
		FileProperties::BasicProperties^ Props = PropOps.get();

		file_size = Props->Size;
		id = zio_counter++;
	}
	catch ( Exception^ ex )
	{
		/*
		 * An error occurs...
		 */
		file = -1;
		Code = ZioCode::ZIO_INVALID;
	}
}

ZioCode Zio::Hint( int catalog_hint_value )
{
	switch ( catalog_hint_value )
	{
	case 0x00:
		return ZioCode::ZIO_PLAIN;
		break;
	case 0x11:
		return ZioCode::ZIO_EPWING;
		break;
	case 0x12:
		return ZioCode::ZIO_EPWING6;
		break;
	}

	return ZioCode::ZIO_INVALID;
}

void Zio::OpenEbZip()
{
	int ebzip_mode;

	/*
	 * Read header part of the ebzip'ped file.
	 */
	Array<byte>^ buff = ref new Array<byte>( ZIO_SIZE_EBZIP_HEADER );
	ReadRaw( ZIO_SIZE_EBZIP_HEADER, buff );
	byte* header = buff->Data;

	ebzip_mode = zio_uint1( header + 5 ) >> 4;
	zip_level = zio_uint1( header + 5 ) & 0x0f;
	slice_size = ZIO_SIZE_PAGE << zip_level;
	file_size = zio_uint5( header + 9 );
	crc = zio_uint4( header + 14 );
	mtime = zio_uint4( header + 18 );

	if ( file_size < ( off_t ) 1 << 16 )
		index_width = 2;
	else if ( file_size < ( off_t ) 1 << 24 )
		index_width = 3;
	else if ( file_size < ( off_t ) 1 << 32 || !off_t_is_large )
		index_width = 4;
	else
		index_width = 5;

	/*
	 * Check zio header information.
	 */
	if ( memcmp( header, "EBZip", 5 ) != 0
		|| ZIO_SIZE_PAGE << ZIO_MAX_EBZIP_LEVEL < slice_size )
		goto failed;

	if ( off_t_is_large ) {
		if ( ebzip_mode != 1 && ebzip_mode != 2 )
			goto failed;
	}
	else {
		if ( ebzip_mode != 1 )
			goto failed;
	}

	/*
	 * Assign ID.
	 */
	id = zio_counter++;

	return;

	/*
	 * An error occurs...
	 */
failed:
	Code = ZioCode::ZIO_INVALID;
}

IAsyncOperation<Zio^>^ Zio::OpenAsync( IStorageFile^ File, ZioCode ZCode )
{
	return create_async( [ & ] {
		return ref new Zio( File, ZCode );
	} );
}