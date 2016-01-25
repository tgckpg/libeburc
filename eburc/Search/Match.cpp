#include "pch.h"
#include "eburc/Search/Match.h"

using namespace libeburc;

int Match::Word( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = *word_p;
			break;
		}
		if ( *word_p == '\0' )
		{
			result = 0;
			break;
		}

		if ( *word_p != *pattern_p )
		{
			result = *word_p - *pattern_p;
			break;
		}

		word_p++;
		pattern_p++;
		i++;
	}

	return result;
}

int Match::PreWord( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = 0;
			break;
		}
		if ( *word_p == '\0' )
		{
			result = 0;
			break;
		}

		if ( *word_p != *pattern_p )
		{
			result = *word_p - *pattern_p;
			break;
		}

		word_p++;
		pattern_p++;
		i++;
	}

	return result;
}

int Match::ExactWordJIS( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = *word_p;
			break;
		}
		if ( *word_p == '\0' )
		{
			/* ignore spaces in the tail of the pattern */
			while ( i < length && *pattern_p == '\0' )
			{
				pattern_p++;
				i++;
			}
			result = ( i - length );
			break;
		}
		if ( *word_p != *pattern_p )
		{
			result = *word_p - *pattern_p;
			break;
		}

		word_p++;
		pattern_p++;
		i++;
	}

	return result;
}

int Match::ExactPreWordJIS( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = 0;
			break;
		}
		if ( *word_p == '\0' )
		{
			/* ignore spaces in the tail of the pattern */
			while ( i < length && *pattern_p == '\0' )
			{
				pattern_p++;
				i++;
			}
			result = ( i - length );
			break;
		}
		if ( *word_p != *pattern_p )
		{
			result = *word_p - *pattern_p;
			break;
		}

		word_p++;
		pattern_p++;
		i++;
	}

	return result;
}

int Match::ExactWordLatin( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = *word_p;
			break;
		}
		if ( *word_p == '\0' )
		{
			/* ignore spaces in the tail of the pattern */
			while ( i < length && ( *pattern_p == ' ' || *pattern_p == '\0' ) )
			{
				pattern_p++;
				i++;
			}
			result = ( i - length );
			break;
		}
		if ( *word_p != *pattern_p )
		{
			result = *word_p - *pattern_p;
			break;
		}

		word_p++;
		pattern_p++;
		i++;
	}

	return result;
}

int Match::ExactPreWordLatin( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = 0;
			break;
		}
		if ( *word_p == '\0' )
		{
			/* ignore spaces in the tail of the pattern */
			while ( i < length && ( *pattern_p == ' ' || *pattern_p == '\0' ) )
			{
				pattern_p++;
				i++;
			}
			result = ( i - length );
			break;
		}
		if ( *word_p != *pattern_p )
		{
			result = *word_p - *pattern_p;
			break;
		}

		word_p++;
		pattern_p++;
		i++;
	}

	return result;
}

int Match::WordKanaGroup( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	unsigned char wc0, wc1, pc0, pc1;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = *word_p;
			break;
		}
		if ( *word_p == '\0' )
		{
			result = 0;
			break;
		}
		if ( length <= i + 1 || *( word_p + 1 ) == '\0' )
		{
			result = *word_p - *pattern_p;
			break;
		}

		wc0 = *word_p;
		wc1 = *( word_p + 1 );
		pc0 = *pattern_p;
		pc1 = *( pattern_p + 1 );

		if ( ( wc0 == 0x24 || wc0 == 0x25 ) && ( pc0 == 0x24 || pc0 == 0x25 ) )
		{
			if ( wc1 != pc1 )
			{
				result = ( ( wc0 << 8 ) + wc1 ) - ( ( pc0 << 8 ) + pc1 );
				break;
			}
		}
		else
		{
			if ( wc0 != pc0 || wc1 != pc1 )
			{
				result = ( ( wc0 << 8 ) + wc1 ) - ( ( pc0 << 8 ) + pc1 );
				break;
			}
		}
		word_p += 2;
		pattern_p += 2;
		i += 2;
	}

	return result;
}

int Match::WordKanaSingle( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	unsigned char wc0, wc1, pc0, pc1;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = *word_p;
			break;
		}
		if ( *word_p == '\0' )
		{
			result = 0;
			break;
		}
		if ( length <= i + 1 || *( word_p + 1 ) == '\0' )
		{
			result = *word_p - *pattern_p;
			break;
		}

		wc0 = *word_p;
		wc1 = *( word_p + 1 );
		pc0 = *pattern_p;
		pc1 = *( pattern_p + 1 );

		if ( ( wc0 == 0x24 || wc0 == 0x25 ) && ( pc0 == 0x24 || pc0 == 0x25 ) )
		{
			if ( wc1 != pc1 )
			{
				result = wc1 - pc1;
				break;
			}
		}
		else
		{
			if ( wc0 != pc0 || wc1 != pc1 )
			{
				result = ( ( wc0 << 8 ) + wc1 ) - ( ( pc0 << 8 ) + pc1 );
				break;
			}
		}
		word_p += 2;
		pattern_p += 2;
		i += 2;
	}

	return result;
}

int Match::ExactWordKanaGroup( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	unsigned char wc0, wc1, pc0, pc1;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = *word_p;
			break;
		}
		if ( *word_p == '\0' )
		{
			result = -*pattern_p;
			break;
		}
		if ( length <= i + 1 || *( word_p + 1 ) == '\0' )
		{
			result = *word_p - *pattern_p;
			break;
		}
		wc0 = *word_p;
		wc1 = *( word_p + 1 );
		pc0 = *pattern_p;
		pc1 = *( pattern_p + 1 );

		if ( ( wc0 == 0x24 || wc0 == 0x25 ) && ( pc0 == 0x24 || pc0 == 0x25 ) )
		{
			if ( wc1 != pc1 )
			{
				result = ( ( wc0 << 8 ) + wc1 ) - ( ( pc0 << 8 ) + pc1 );
				break;
			}
		}
		else
		{
			if ( wc0 != pc0 || wc1 != pc1 )
			{
				result = ( ( wc0 << 8 ) + wc1 ) - ( ( pc0 << 8 ) + pc1 );
				break;
			}
		}
		word_p += 2;
		pattern_p += 2;
		i += 2;
	}
	return result;
}

int Match::ExactWordKanaSingle( const char *word, const char *pattern, size_t length )
{
	int i = 0;
	unsigned char *word_p = ( unsigned char * ) word;
	unsigned char *pattern_p = ( unsigned char * ) pattern;
	unsigned char wc0, wc1, pc0, pc1;
	int result;

	for ( ;;)
	{
		if ( length <= i )
		{
			result = *word_p;
			break;
		}
		if ( *word_p == '\0' )
		{
			result = -*pattern_p;
			break;
		}
		if ( length <= i + 1 || *( word_p + 1 ) == '\0' )
		{
			result = *word_p - *pattern_p;
			break;
		}
		wc0 = *word_p;
		wc1 = *( word_p + 1 );
		pc0 = *pattern_p;
		pc1 = *( pattern_p + 1 );

		if ( ( wc0 == 0x24 || wc0 == 0x25 ) && ( pc0 == 0x24 || pc0 == 0x25 ) )
		{
			if ( wc1 != pc1 )
			{
				result = wc1 - pc1;
				break;
			}
		}
		else
		{
			if ( wc0 != pc0 || wc1 != pc1 )
			{
				result = ( ( wc0 << 8 ) + wc1 ) - ( ( pc0 << 8 ) + pc1 );
				break;
			}
		}
		word_p += 2;
		pattern_p += 2;
		i += 2;
	}

	return result;
}
