#include "pch.h"
#include "EBBook.h"
#include "EBSubbook.h"

using namespace libeburc;

EBSubbook::EBSubbook( EBBook^ Book )
{
	ParentBook = Book;
}


IAsyncAction^ EBSubbook::OpenAsync()
{
	return create_async( [ & ] { SetAuto(); } );
}

void EBSubbook::SetAuto()
{
	/*
     * Dispatch.
     */
	if ( ParentBook->disc_code == EBDiscCode::EB_DISC_EB )
	{
		SetEB();
	}
	else
	{
		SetEPWING();
	}

	/*
     * Load the subbook.
     */
	Load();
}

void EBSubbook::Load()
{
	/*
     * Reset contexts.
     */
    // eb_reset_search_contexts(book);
    // eb_reset_text_context(book);
    // eb_reset_binary_context(book);

	/*
	 * If the subbook has already initialized, return immediately.
	 */
	if ( initialized ) return;

	if ( TextZio->Code != ZioCode::ZIO_INVALID )
	{
		/*
		 * Read index information.
		 */
		LoadIndexes();

		/*
		 * Read mutli search information.
		 */
		LoadMultiSearches();
		LoadMultiTitles();

		/*
		 * Rewind the file descriptor of the start file.
		 */
		TextZio->LSeekRaw( ( ( off_t ) IndexPage - 1 ) * EB_SIZE_PAGE );
	}
}


void EBSubbook::LoadIndexes()
{
	throw ref new NotImplementedException( "LoadIndexes not impl yet" );
}

void EBSubbook::LoadMultiSearches()
{
	throw ref new NotImplementedException( "LoadMultiSearch not impl yet" );
}

void EBSubbook::LoadMultiTitles()
{
	throw ref new NotImplementedException( "LoadMultiTitle not impl yet" );
}

void EBSubbook::SetEPWING()
{
	throw ref new NotImplementedException( "Subbook SetEPWing is not impl yet" );
}

void EBSubbook::SetEB()
{
	/*
	 * Open a text file if exists.
	 */
	ZioCode TextZCode = ZioCode::ZIO_INVALID;

	if ( initialized )
	{
		if ( TextZio->Code != ZioCode::ZIO_INVALID )
			TextZCode = ZioCode::ZIO_REOPEN;
	}
	else
	{
		try
		{
			TextFile = FileName::eb_find_file_name( DirRoot, EB_FILE_NAME_START );
			TextZCode = FileName::eb_path_name_zio_code( TextFile, ZioCode::ZIO_PLAIN );
		}
		catch ( Exception^ ex )
		{

		}
	}

	if ( TextZCode != ZioCode::ZIO_INVALID )
	{
		TextZio = ref new Zio( TextFile, TextZCode );
		TextZCode = TextZio->Code;
	}

	IStorageFile^ GraphicFile;
	/*
	 * Open a graphic file if exists.
	 */
	ZioCode GraphZCode = ZioCode::ZIO_INVALID;
	if ( initialized )
	{
		if ( GraphicZio->Code != ZioCode::ZIO_INVALID )
			GraphZCode = ZioCode::ZIO_REOPEN;
	}
	else if ( TextZCode != ZioCode::ZIO_INVALID )
	{
		GraphicFile = TextFile;
		GraphZCode = TextZCode;
	}

	/* The following code does not make sense
	 * Before the Subbook is initialize, the graph_file_name
	 * can only be initialized via text_file_name
	 * and there were no handling for last ELSE case for graphic file
	if ( graphic_zio_code != ZIO_INVALID )
	{
		eb_compose_path_name2( book->path, subbook->directory_name,
			subbook->graphic_file_name, graphic_path_name );
		if ( zio_open( &subbook->graphic_zio, graphic_path_name,
			graphic_zio_code ) < 0 )
		{
			error_code = EB_ERR_FAIL_OPEN_BINARY;
			goto failed;
		}
		graphic_zio_code = zio_mode( &subbook->graphic_zio );
	}
	* Since we are using file reference now
	* we just use the GraphicFile ( i.e. TextFile ) instead
	* Notice GrapZCode can only be valid where TextFile is valid
	*/
	if ( GraphZCode != ZioCode::ZIO_INVALID )
	{
		GraphicZio = ref new Zio( GraphicFile, GraphZCode );
	}
}