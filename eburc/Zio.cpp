#include "pch.h"
#include "eburc/Zio.h"

using namespace libeburc;
using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;

/*
 * Zio object counter.
 */
static int zio_counter = 0;

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

Zio::Zio() { }

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
			if ( t == buffer->Length )
			{
				reader->ReadBytes( buffer );
			}
			else if( t < buffer->Length )
			{
				Array<byte>^ b = ref new Array<byte>( t );
				reader->ReadBytes( b );
				memcpy_s( buffer->Data, t, b->Data, t );
			}
			else
			{
				throw ref new Exception( EXCEPTION_STACK_OVERFLOW );
			}
		} ).wait();
		reader->DetachStream();

		posx += length;
	} ).wait();
}
/*
 * Seek `zio'.
 */
void Zio::LSeek( off_t location, int whence )
{
	off_t result;

	if ( SrcFile == nullptr )
		EBException::Throw( EBErrorCode::EB_ERR_ZIO_SEEK_FAILED );

	if ( Code == ZioCode::ZIO_PLAIN )
	{
		/*
		 * If `zio' is not compressed, simply call lseek().
		 */
		LSeekRaw( location );
	}
	else
	{
		/*
		 * Calculate new location according with `whence'.
		 */
		switch ( whence )
		{
		case SEEK_SET:
			this->location = location;
			break;
		case SEEK_CUR:
			this->location += location;
			break;
		case SEEK_END:
			this->location = file_size - location;
			break;
		default:
#ifdef EINVAL
			errno = EINVAL;
#endif
			EBException::Throw( EBErrorCode::EB_ERR_ZIO_SEEK_FAILED );
		}

		/*
		 * Adjust location.
		 */
		if ( this->location < 0 )
			this->location = 0;
		if ( file_size < this->location )
			this->location = file_size;
	}
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
		ReadRaw( length, buffer );
		break;
	case ZioCode::ZIO_EBZIP1:
		ReadEBZip( length, buffer );
		break;
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
	SSIZE_T read_length = 0;

	/*
	 * Read data.
	 */
	while ( read_length < length ) {
		if ( file_size <= location ) return;

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
			Array<byte>^ buff = ref new Array<byte>( 8 );
			ReadRaw( index_width * 2, buff );
			char* temporary_buffer = ( char * ) buff->Data;


			off_t slice_location;
			off_t next_slice_location;
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
				EBException::Throw( EBErrorCode::EB_ERR_FAIL_READ_EBZ );
			}

			size_t zipped_slice_size = next_slice_location - slice_location;

			if ( next_slice_location <= slice_location
				|| slice_size < zipped_slice_size )
				EBException::Throw( EBErrorCode::EB_ERR_FAIL_READ_EBZ );

			/*
			 * Read a compressed slice from `file' and uncompress it.
			 * The data is not compressed if its size is equals to
			 * slice size.
			 * ORG: if (zio_lseek_raw(zio, slice_location, SEEK_SET) < 0)
			 */
			LSeekRaw( slice_location );

			UnzipSlice( zipped_slice_size, cache_buffer );

			cache_zio_id = id;
		}

		/*
		 * Copy data from `cache_buffer' to `buffer'.
		 */
		int n = slice_size - ( location % slice_size );
		if ( length - read_length < n ) n = length - read_length;
		if ( file_size - location < n ) n = file_size - location;

		byte* buffer = ibuffer->Data;
		memcpy_s( buffer + read_length, n
			, cache_buffer + ( location % slice_size ), n );

		read_length += n;
		location += n;
	}
}

void Zio::UnzipSlice( size_t zipped_slice_size, byte *out_buffer )
{
	char in_buffer[ ZIO_SIZE_PAGE ];

	z_stream stream;
	size_t read_length;
	int z_result;

	if ( slice_size == zipped_slice_size )
	{
		/*
		 * The input slice is not compressed.
		 * Read the target page in the slice.
		 * ORG. if ( zio_read_raw( zio, out_buffer, zipped_slice_size ) != zipped_slice_size )
		 *
		 */
		Array<byte>^ buff = ref new Array<byte>( zipped_slice_size );
		ReadRaw( zipped_slice_size, buff );
		out_buffer = buff->Data;
	}
	else
	{
		/*
		 * The input slice is compressed.
		 * Read and uncompress the target page in the slice.
		 *
		 */
		stream.zalloc = NULL;
		stream.zfree = NULL;
		stream.opaque = NULL;

		if ( inflateInit( &stream ) != Z_OK )
			EBException::Throw( EBErrorCode::EB_ERR_FAIL_READ_EBZ );

		stream.next_in = ( Bytef * ) in_buffer;
		stream.avail_in = 0;
		stream.next_out = ( Bytef * ) out_buffer;
		stream.avail_out = slice_size;

		while ( stream.total_out < slice_size )
		{
			if ( 0 < stream.avail_in )
				memmove_s( in_buffer, stream.avail_in, stream.next_in, stream.avail_in );

			if ( zipped_slice_size - stream.total_in < ZIO_SIZE_PAGE )
			{
				read_length = zipped_slice_size - stream.total_in - stream.avail_in;
			}
			else
			{
				read_length = ZIO_SIZE_PAGE - stream.avail_in;
			}

			Array<byte>^ buff = ref new Array<byte>( read_length );
			ReadRaw( read_length, buff );
			memcpy_s( in_buffer + stream.avail_in, read_length, buff->Data, read_length );

			stream.next_in = ( Bytef * ) in_buffer;
			stream.avail_in += read_length;
			stream.avail_out = slice_size - stream.total_out;

			try
			{
				z_result = inflate( &stream, Z_SYNC_FLUSH );
				if ( z_result == Z_STREAM_END )
				{
					break;
				}
				else if ( z_result != Z_OK && z_result != Z_BUF_ERROR )
				{
					EBException::Throw( EBErrorCode::EB_ERR_FAIL_READ_EBZ );
				}
			}
			catch ( Exception^ ex )
			{
				inflateEnd( &stream );
				throw ex;
			}

		}
	}
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

