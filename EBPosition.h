#pragma once

namespace libeburc
{
	public ref class EBPosition sealed
	{
	internal:
		/*
		 * Page. (1, 2, 3 ...)
		 */
		int page;

		/*
		 * Offset in `page'. (0 ... 2047)
		 */
		int offset;

		EBPosition( int Page = 0, int Offset = 0 );

	public:
		property int Page {
			int get() { return page; }
		}

		property int Offset {
			int get() { return offset; }
		}
	};
}
