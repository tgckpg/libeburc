#pragma once

#include <pch.h>
#include <defs.h>
#include <Zio.h>

using namespace std;
using namespace Platform;

namespace libeburc
{
	public ref class EBSubbook sealed
	{
	internal:
		/// <summary>
		/// Whether the object has been initialized.
		/// </summary>
		int initialized;

		/// <summary>
		/// File descriptor and compression information for text file.
		/// </summary>
		Zio TextZio;

		/// <summary>
		/// File descriptor and compression information for graphic file.
		/// </summary>
		Zio GraphicZio;

		/// <summary>
		/// File descriptor and compression information for sound file.
		/// </summary>
		Zio SoundZio;

		/// <summary>
		/// File descriptor and compression information for movie file.
		/// </summary>
		Zio MovieZio;

		/// <summary>
		/// Subbook directory name.
		/// </summary>
		char directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];

		/// <summary>
		/// The Subbook Root dir
		/// </summary>
		IStorageFolder^ DirRoot;

		/// <summary>
		/// Internal title
		/// </summary>
		char title[ EB_MAX_TITLE_LENGTH + 1 ];
	public:
		/// <summary>
		/// Index page.
		/// </summary>
		property int IndexPage;

		/// <summary>
		/// Subbook ID.
		/// This subbook is not available, if the code is EB_SUBBOOK_INVALID.
		/// </summary>
		property EBSubbookCode Code;

		property String^ Title
		{
			String^ get()
			{
				wstring wtitle = Utils::ToWStr( title );
				return ref new String( wtitle.c_str() );
			}
		}

		EBSubbook();
	};
}

