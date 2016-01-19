#pragma once

namespace libeburc
{
	/// <summary>
	/// UTF-8 normalization table.
	/// </summary>
	ref class EBUTF8Table sealed
	{
	internal:
		int code;
		char *string;
	public:
		EBUTF8Table();
	};
}
