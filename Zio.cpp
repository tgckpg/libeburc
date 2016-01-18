#include "pch.h"
#include "Zio.h"

using namespace libeburc;
using namespace concurrency;
using namespace Microsoft::WRL;

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

/*
 * Size of a page (The term `page' means `block' in JIS X 4081).
 */
#define ZIO_SIZE_PAGE			2048

Zio::Zio() {}

Zio::Zio( IStorageFile^ File, ZioCode ZCode )
{
	SrcFile = File;
	Code = ZCode;

	int result;

	switch ( Code )
	{
/*
	case ZIO_REOPEN:
		result = zio_reopen( zio, file_name );
		break;
	case ZIO_PLAIN:
		result = zio_open_plain( zio, file_name );
		break;
*/
	case ZioCode::ZIO_EBZIP1:
		OpenEbZip();
		break;
/*
	case ZIO_EPWING:
		result = zio_open_epwing( zio, file_name );
		break;
	case ZIO_EPWING6:
		result = zio_open_epwing6( zio, file_name );
		break;
	case ZIO_SEBXA:
		result = zio_open_plain( zio, file_name );
		break;
*/
	default:
		result = -1;
	}
}

IBuffer^ Zio::ReadRaw( IBuffer^ buffer, size_t length )
{
	IBuffer^ RBuffer;
	task<IInputStream^> RandStream( SrcFile->OpenSequentialReadAsync() );
	RandStream.then( [ & ] ( IInputStream^ RStream )
	{
		task<IBuffer^> ReadOp( RStream->ReadAsync( buffer, length, InputStreamOptions::None ) );
		RBuffer = ReadOp.get();
	} ).wait();

	return RBuffer;
}


IBuffer^ Zio::Read( IBuffer^ buffer, size_t length )
{
	switch ( Code )
	{
/*
	case ZIO_PLAIN:
		return ReadPlain( zio, file_name );
*/
	case ZioCode::ZIO_EBZIP1:
		return ReadRaw( buffer, length );
/*
	case ZIO_EPWING:
		return ReadEPWing( zio, file_name );
	case ZIO_EPWING6:
		return result = ReadEPWing6( zio, file_name );
	case ZIO_SEBXA:
		return ReadSebXA( zio, file_name );
*/
	}

	return nullptr;
}


void Zio::OpenEbZip()
{
	int ebzip_mode;
	byte* header = nullptr;

	/*
	 * Read header part of the ebzip'ped file.
	 */
	IBuffer^ HBuffer = ReadRaw( ref new Buffer( ZIO_SIZE_EBZIP_HEADER ), ZIO_SIZE_EBZIP_HEADER );
	ComPtr<IBufferByteAccess> BuffByteAccess;
	reinterpret_cast< IInspectable* >( HBuffer )->QueryInterface( IID_PPV_ARGS( &BuffByteAccess ) );

	BuffByteAccess->Buffer( &header );

	ebzip_mode = zio_uint1( header + 5 ) >> 4;
	zip_level = zio_uint1( header + 5 ) & 0x0f;
	slice_size = ZIO_SIZE_PAGE << zip_level;
	file_size = zio_uint5( header + 9 );
	crc = zio_uint4( header + 14 );
	mtime = zio_uint4( header + 18 );
	location = 0;

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