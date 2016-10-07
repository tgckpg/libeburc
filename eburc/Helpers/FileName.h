#pragma once

#include <pch.h>
#include <eburc/defs.h>
#include <eburc/Zio.h>
#include <eburc/EBException.h>

using namespace std;
using namespace Windows::Storage;

namespace libeburc
{
	class FileName
	{
	public:
		/// <summary>
		/// Rewrite `found_file_name' to a real file name in the `path_name'
		/// directory.
		///
		/// If a file matched to `target_file_name' exists, then EB_SUCCESS
		/// is returned, and `found_file_name' is rewritten to that name.
		/// Otherwise EB_ERR_BAD_FILE_NAME is returned.
		///
		/// Note that `target_file_name' must not contain `.' or excceed
		/// EB_MAX_DIRECTORY_NAME_LENGTH characters.
		///
		/// Extra: Returns the found_file instead.
		/// Extra: eb_find_file_name2 is also generalized within.
		/// </summary>
		static StorageFile^ eb_find_file_name( IStorageFolder^ Folder, wstring target_file_name );
		static StorageFile^ eb_find_file_name( IStorageFolder^ Folder, wstring target_file_name, vector<wchar_t*> supp_exts );
		/// <summary>
		/// Rewrite `directory_name' to a real directory name in the `path' directory.
		///
		/// If a directory matched to `directory_name' exists, then EB_SUCCESS is
		/// returned, and `directory_name' is rewritten to that name.  Otherwise
		/// EB_ERR_BAD_DIR_NAME is returned.
		/// 
		/// Ex: Now returns the target StorageFolder
		/// </summary>
		static IStorageFolder^ eb_fix_directory( IStorageFolder^ Folder, wstring target_dir_name );
		static ZioCode eb_path_name_zio_code( IStorageFile^ File, ZioCode DefaultCode );
	};
}
