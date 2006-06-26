/* Standard Pike include files. */
#include "bignum.h"
#include "array.h"
#include "builtin_functions.h"
#include "constants.h"
#include "interpret.h"
#include "mapping.h"
#include "multiset.h"
#include "module_support.h"
#include "object.h"
#include "pike_macros.h"
#include "pike_types.h"
#include "program.h"
#include "stralloc.h"
#include "svalue.h"
#include "threads.h"
#include "version.h"
#include "operators.h"

#if (PIKE_MAJOR_VERSION == 7 && PIKE_MINOR_VERSION == 1 && PIKE_BUILD_VERSION >= 12) || PIKE_MAJOR_VERSION > 7 || (PIKE_MAJOR_VERSION == 7 && PIKE_MINOR_VERSION > 1)
# include "pike_error.h"
#else
# include "error.h"
# ifndef Pike_error
#  define Pike_error error
# endif
#endif

#ifndef ARG
/* Get argument # _n_ */
#define ARG(_n_) Pike_sp[-((args - _n_) + 1)]
#endif


/* 
    some functions that might not be available otherwise 

*/

int sqlite_decode_binary(const unsigned char *in, unsigned char *out);
int sqlite_encode_binary(const unsigned char *in, int n, unsigned char *out);


int sqlite_encode_binary(const unsigned char *in, int n, unsigned char *out)
{
		int i, j, e, m;
		int cnt[256];
		if (n <= 0) {
				out[0] = 'x';
				out[1] = 0;
				return 1;
		}
		memset(cnt, 0, sizeof(cnt));
		for (i = n - 1; i >= 0; i--) {
				cnt[in[i]]++;
		}
		m = n;
		for (i = 1; i < 256; i++) {
				int sum;
				if (i == '\'')
						continue;
				sum = cnt[i] + cnt[(i + 1) & 0xff] + cnt[(i + '\'') & 0xff];
				if (sum < m) {
						m = sum;
						e = i;
						if (m == 0)
								break;
				}
		}
		out[0] = e;
		j = 1;
		for (i = 0; i < n; i++) {
				int c = (in[i] - e) & 0xff;
				if (c == 0) {
						out[j++] = 1;
						out[j++] = 1;
				} else if (c == 1) {
						out[j++] = 1;
						out[j++] = 2;
				} else if (c == '\'') {
						out[j++] = 1;
						out[j++] = 3;
				} else {
						out[j++] = c;
				}
		}
		out[j] = 0;
		return j;
}

/*
** Decode the string "in" into binary data and write it into "out".
** This routine reverses the encoding created by sqlite_encode_binary().
** The output will always be a few bytes less than the input.  The number
** of bytes of output is returned.  If the input is not a well-formed
** encoding, -1 is returned.
**
** The "in" and "out" parameters may point to the same buffer in order
** to decode a string in place.
*/
int sqlite_decode_binary(const unsigned char *in, unsigned char *out)
{
		int i, c, e;
		e = *(in++);
		i = 0;
		while ((c = *(in++)) != 0) {
				if (c == 1) {
						c = *(in++);
						if (c == 1) {
								c = 0;
						} else if (c == 2) {
								c = 1;
						} else if (c == 3) {
								c = '\'';
						} else {
								return -1;
						}
				}
				out[i++] = (c + e) & 0xff;
		}
		return i;
}
