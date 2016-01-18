#include "pch.h"
#include "Zio.h"

using namespace libeburc;

using namespace concurrency;

Zio::Zio( IStorageFile^ File, ZioCode ZCode )
{

}

IAsyncOperation<Zio^>^ Zio::Open( IStorageFile^ File, ZioCode ZCode )
{
	return create_async( [ & ] {
		Zio^ ZInst = ref new Zio( File, ZCode );
		return ZInst;
	} );

}