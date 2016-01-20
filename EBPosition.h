#pragma once

namespace libeburc
{
	ref class EBPosition sealed
	{
		/*
		 * Page. (1, 2, 3 ...)
		 */
		int page;

		/*
		 * Offset in `page'. (0 ... 2047)
		 */
		int offset;
	public:
		EBPosition();
	};
}
