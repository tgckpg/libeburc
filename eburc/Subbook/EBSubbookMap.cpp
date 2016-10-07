#include "pch.h"
#include "eburc/Book/EBBook.h"
#include "eburc/Subbook/EBSubbook.h"

using namespace libeburc;

void EBSubbook::LoadMap()
{
	EBHMap = ref new EBMap( EBMapType::EB_MAP_HAN_FONT );
	EBZMap = ref new EBMap( EBMapType::EB_MAP_ZEN_FONT );
	EBCMap = ref new EBMap( EBMapType::EB_MAP_C );
	EBGMap = ref new EBMap( EBMapType::EB_MAP_G );

	try
	{
		wstring wname( DirRoot->Name->Data() );
		wname.append( L"-ExtB" );

		MapFile = FileName::eb_find_file_name( ParentBook->DirRoot, wname, vector<wchar_t*> { L".map" } );

		if ( MapFile == nullptr )
		{
			wstring wname( DirRoot->Name->Data() );
			MapFile = FileName::eb_find_file_name( ParentBook->DirRoot, wname, vector<wchar_t*> { L".map" } );
		}
	}
	catch ( Exception^ ex )
	{
		return;
	}

	if ( MapFile == nullptr ) return;

	auto inputStream = task<IInputStream^>( MapFile->OpenSequentialReadAsync() );

	DataReader^ dataReader = ref new DataReader( inputStream.get() );
	int currChunk = 0;
	int chunkSize = 1024;
	int nbytes = 1024;

	int skip = false;
	bool line_started = true;

	while ( nbytes == chunkSize )
	{
		nbytes = 0;
		create_task( dataReader->LoadAsync( chunkSize ) ).then(
			[ & ] ( task<unsigned int> bytesLoaded )
		{
			nbytes = bytesLoaded.get();

			int i = 0;
			while ( i < nbytes )
			{
				char b = dataReader->ReadByte();
				EBHMap->Feed( &b );
				EBZMap->Feed( &b );
				EBCMap->Feed( &b );
				EBGMap->Feed( &b );
				i++;
			}
			currChunk++;
		} ).wait();
	}

	const char *  cc = EBHMap->Get( 42022 );
}