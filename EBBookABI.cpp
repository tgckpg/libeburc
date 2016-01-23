#include "pch.h"
#include "EBSubbook.h"
#include "EBBook.h"

using namespace libeburc;

using namespace concurrency;
using namespace Microsoft::WRL;

EBBook::EBBook( IStorageFolder^ BookDir )
{
	DirRoot = BookDir;
	code = EB_BOOK_NONE;
}

IAsyncOperation<EBBook^>^ EBBook::Parse( IStorageFolder^ BookDir )
{
	return create_async( [=]
	{
		EBBook^ Book = ref new EBBook( BookDir );
		Book->Bind();
		return Book;
	} );
}

IIterable<EBSubbook^>^ EBBook::Subbooks::get()
{
	return subbooks->GetView();
}
