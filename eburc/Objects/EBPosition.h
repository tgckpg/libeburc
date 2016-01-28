#pragma once

namespace libeburc
{
	public ref class EBPosition sealed
	{
	internal:
		/*
		 * Page. (1, 2, 3 ...)
		 */
		int _page;

		/*
		 * Offset in `page'. (0 ... 2047)
		 */
		int _offset;

		EBPosition( int Page, int Offset ) : _page( Page ), _offset( Offset ) { };
		EBPosition() : _page( 0 ), _offset( 0 ) { };

	public:
		property int Page {
			int get() { return _page; }
		}

		property int Offset {
			int get() { return _offset; }
		}
	};
}
