#pragma once

#include <pch.h>
#include <defs.h>
#include <Zio.h>


namespace libeburc
{
	public ref class EBSubbook sealed
	{
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

		EBSubbook();
	};
}

