#include "pch.h"
#include "eburc/Helpers/FileName.h"

using namespace std;
using namespace libeburc;
using namespace concurrency;
using namespace Platform::Collections;
using namespace Windows::Storage;
using namespace Windows::Foundation::Collections;

StorageFile^ FileName::eb_find_file_name( IStorageFolder^ Folder, wstring TargetName )
{
	vector<wchar_t*> exts { L"", L";1", L".", L".;1", L".ebz", L".ebz;1", L".org", L".org:1" };
	return eb_find_file_name( Folder, TargetName, exts );
}

ZioCode FileName::eb_path_name_zio_code( IStorageFile^ File, ZioCode DefaultCode )
{
	wstring fName = File->Name->Data();

	size_t ExtS = fName.find_last_of( '.' );

	wstring ext = L"";
	if ( ExtS != string::npos )
	{
		ext = fName.substr( ExtS );
	}

	const wchar_t * extc = ext.c_str();

	if ( _wcsnicmp( extc, L".ebz", 4 ) == 0 )
	{
		return ZioCode::ZIO_EBZIP1;
	}
	else if ( _wcsnicmp( extc, L".org", 4 ) == 0 || wcscmp( extc, L"" ) == 0 )
	{
		return ZioCode::ZIO_PLAIN;
	}

	return DefaultCode;
}

StorageFile^ FileName::eb_find_file_name( IStorageFolder^ Folder, wstring TargetName, vector<wchar_t*> supp_exts )
{
	StorageFile^ FoundFile;

	const wchar_t* tName = TargetName.c_str();

	auto Task = task<IVectorView<StorageFile^>^>( Folder->GetFilesAsync() );
	Task.then(
		[ & ] ( IVectorView<StorageFile^>^ Files )
	{
		VectorViewIterator<StorageFile^> TargetFile = find_if( begin( Files ), end( Files ), [ & ] ( StorageFile^ File ) {
			wstring fName = File->Name->Data();
			size_t ExtS = fName.find_last_of( '.' );

			wstring basename = fName;
			wstring ext = L"";
			if ( ExtS != string::npos )
			{
				basename = fName.substr( 0, ExtS );
				ext = fName.substr( ExtS );
			}

			if ( _wcsicmp( basename.c_str(), tName ) == 0 )
			{
				const wchar_t * wext = ext.c_str();

				auto s = find_if( supp_exts.begin(), supp_exts.end(), [ & ] ( wchar_t * lext ) {
					return _wcsicmp( lext, wext ) == 0;
				} );

				if ( s != end( supp_exts ) )
				{
					return true;
				}
			}
			return false;
		} );

		if ( TargetFile == end( Files ) )
			EBException::Throw( EBErrorCode::EB_ERR_BAD_FILE_NAME );

		FoundFile = *TargetFile;
	} ).wait();

	return FoundFile;
}

IStorageFolder^ FileName::eb_fix_directory( IStorageFolder^ Folder, wstring TargetName )
{
	IStorageFolder^ FoundFolder;

	const wchar_t* tName = TargetName.c_str();

	auto Task = task<IVectorView<StorageFolder^>^>( Folder->GetFoldersAsync() ).then(
		[ & ] ( IVectorView<StorageFolder^>^ Folders )
	{
		VectorViewIterator<StorageFolder^> TargetFolder = find_if( begin( Folders ), end( Folders ), [ & ] ( StorageFolder^ Folder ) {
			wstring fName = Folder->Name->Data();
			return _wcsicmp( fName.c_str(), tName ) == 0;
		} );

		if ( TargetFolder == end( Folders ) )
			EBException::Throw( EBErrorCode::EB_ERR_BAD_DIR_NAME, tName );

		FoundFolder = *TargetFolder;
	} );

	Task.wait();

	return FoundFolder;
}
