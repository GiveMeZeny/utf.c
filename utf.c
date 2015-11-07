#include <stdlib.h>
#include <string.h>
#include "utf.h"

union utf8 {
	const char *cp;   /* const pointer */
	unsigned char *p; /* pointer */
	char *pc;         /* plain char */
};

union utf16 {
	const char16_t *cp; /* const pointer */
	char16_t *p;        /* pointer */
	unsigned char *b;   /* byte buffer */
};

union utf32 {
	const char32_t *cp; /* const pointer */
	char32_t *p;        /* pointer */
	unsigned char *b;   /* byte buffer */
};

Rune Runeerror = 0xfffd;

/* return 1 if it's just ascii */
#define UTF8_IS_ASCII(c) (((unsigned char)(c) & 0x80) != 0x80)

/* return 1 if continuation bytes are introduced */
static inline int UTF8_IS_LEADING(unsigned char c) { return c >> 7 & c >> 6; }

/* return 1 if it's a continuation byte */
static inline int UTF8_IS_TRAILING(unsigned char c) { return c >> 7 & ~c >> 6; }

/* return 1 if it's a high surrogate */
#define UTF16_IS_LEADING(c) (((char16_t)(c) & 0xfc00) == 0xd800)

/* return 1 if it's a low surrogate */
#define UTF16_IS_TRAILING(c) (((char16_t)(c) & 0xfc00) == 0xdc00)

/* get the max rune for rune with x continuation bytes */
static inline Rune RuneX(int x)
{
	return x ? ((1u << (6 - x + (x * 6))) - 1) : ((1u << 7) - 1);
}

/* return the number of trailing bytes */
static inline int utf8_trail_cnt(unsigned char c)
{
	if (c < 0x80)
		return 0;
	else if (c < 0xc0)
		return -1;
	else if (c < 0xe0)
		return 1;
	else if (c < 0xf0)
		return 2;
	else if (c < 0xf8)
		return 3;
	else if (c < 0xfc)
		return 4;
	else if (c < 0xfe)
		return 5;
	else
		return -1;
}

int runetochar(char *buf, Rune *rune)
{
	union utf8 u = {.pc = buf};
	unsigned char *ptr, lead_byte;
	Rune c = *rune;
	int n, retval;

	if (c < Runeself) {
		*u.p = c;
		return 1;
	} else if (!validrune(c)) {
		c = Runeerror;
	}
	for (n = 1; n < UTFmax; n++) {
		if (c <= RuneX(n))
			break;
	}
	retval = n + 1;
	ptr = (u.p += n);
	lead_byte = (((1u << (n + 1)) - 1) << (7 - n));
	while (n--) {
		*ptr-- = (c & 0x3F) | 0x80;
		c >>= 6;
	}
	*ptr = c | lead_byte;
	return retval;
}

int chartorune(Rune *rune, const char *str)
{
	unsigned char tmp = *str++;
	int i = 0, n = utf8_trail_cnt(tmp);
	int retval = n + 1;
	Rune c = tmp;

	if (n > 0 && n < UTFmax) {
		c &= ((1u << (6 - n)) - 1);
		for (; (tmp = *str++) && i < n; i++) {
			if (!UTF8_IS_TRAILING(tmp))
				break;
			c <<= 6;
			c |= tmp & 0x3f;
		}
	}
	if (n < 0 || n >= UTFmax || i < n || !validrune(c) ||
	    (n > 0 && c <= RuneX(n - 1))) {
		c = Runeerror;
		retval = 1;
	}
	*rune = c;
	return retval;
}

int charntorune(Rune *rune, const char *str, size_t n)
{
	*rune = Runeerror;
	if (!n)
		return 0;
	else if (!fullrune(str, n))
		return 1;
	else
		return chartorune(rune, str);
}

int runelen(Rune rune)
{
	char tmp[UTFmax];

	return runetochar(tmp, &rune);
}

int runenlen(Rune *rune, int n)
{
	int retval = 0;

	while(n-- > 0)
		retval += runelen(*rune++);
	return retval;
}

int fullrune(const char *str, size_t n)
{
	int t = utf8_trail_cnt(*str);

	if (n <= 0)
		return 0;
	if (t < 0 || t >= UTFmax)
		return 1;
	else
		return n >= (size_t)t + 1;
}

int validrune(Rune rune)
{
	if (rune > Runemax || (rune & 0xf800) == 0xd800)
		return 0;
	else
		return 1;
}

size_t utflen(const char *str)
{
	size_t len;
	Rune rune;

	for (len = 0; *str; len++)
		str += chartorune(&rune, str);
	return len;
}

size_t utfnlen(const char *str, size_t maxlen)
{
	size_t len;
	Rune rune;
	int n;

	for (len = 0; (n = charntorune(&rune, str, maxlen)); len++) {
		if (!rune)
			break;
		str += n;
		maxlen -= n;
	}
	return len;
}

char *utfrune(const char *str, Rune rune)
{
	union utf8 u = {.cp = str};
	Rune c;

	if (rune < Runeself)
		return strchr(str, rune);
	if (rune != Runeerror) {
		char tmp[UTFmax + 1];
		int n = runetochar(tmp, &rune);

		tmp[n] = '\0';
		return strstr(str, tmp);
	}
	do {
		int n = chartorune(&c, u.cp);

		if (c == rune)
			return u.pc;
		u.p += n;
	} while (c);
	return NULL;
}

char *utfrrune(const char *str, Rune rune)
{
	union utf8 u = {.cp = str};
	char *save = NULL;
	Rune c;

	if (rune < Runeself)
		return strrchr(str, rune);
	do {
		int n = chartorune(&c, u.cp);

		if (c == rune)
			save = u.pc;
		u.p += n;
	} while (c);
	return save;
}

char *utfutf(const char *str, const char *substr)
{
	Rune c;
	int n = chartorune(&c, substr);
	union utf8 u;
	size_t len;

	if (c < Runeself)
		return strstr(str, substr);
	len = strlen(substr);
	u.cp = str;
	for (; (u.pc = utfrune(u.cp, c)); u.p += n) {
		if (!strncmp(u.cp, substr, len))
			return u.pc;
	}
	return NULL;
}

#define RUNETOCHAR16(buf, rune)                          \
	do {                                             \
		Rune c = *rune;                          \
		if (!validrune(c))                       \
			c = Runeerror;                   \
		if (c <= 0xffff) {                       \
			*buf = c;                        \
			return 1;                        \
		}                                        \
		c -= 0x10000;                            \
		*buf++ = (((c >> 10) & 0x3ff) | 0xd800); \
		*buf = ((c & 0x3ff) | 0xdc00);           \
		return 2;                                \
	} while (0)

int runetochar16(char16_t *buf, Rune *rune)
{
	RUNETOCHAR16(buf, rune);
}

int runetochar16xe(char16_t *buf, Rune *rune, int be)
{
	union utf16 u = {.p = buf};
	Rune c = *rune, c1, c2;

	if (!validrune(c))
		c = Runeerror;
	if (c <= 0xffff) {
		*u.b++ = (be ? (c >> 8) : (c >> 0)) & 0xff;
		*u.b = (be ? (c >> 0) : (c >> 8)) & 0xff;
		return 1;
	}
	c -= 0x10000; /* remove 21th bit */
	c1 = (((c >> 10) & 0x3ff) | 0xd800); /* high surrogate */
	c2 = ((c & 0x3ff) | 0xdc00); /* low surrogate */
	*u.b++ = (be ? (c1 >> 8) : (c1 >> 0)) & 0xff;
	*u.b++ = (be ? (c1 >> 0) : (c1 >> 8)) & 0xff;
	*u.b++ = (be ? (c2 >> 8) : (c2 >> 0)) & 0xff;
	*u.b = (be ? (c2 >> 0) : (c2 >> 8)) & 0xff;
	return 2;
}

#define RUNETOCHAR32(buf, rune)                      \
	do {                                         \
		Rune c = *rune;                      \
		*buf = validrune(c) ? c : Runeerror; \
		return 1;                            \
	} while (0)

int runetochar32(char32_t *buf, Rune *rune)
{
	RUNETOCHAR32(buf, rune);
}

int runetochar32xe(char32_t *buf, Rune *rune, int be)
{
	union utf32 u = {.p = buf};
	Rune c = *rune;

	if (!validrune(c))
		c = Runeerror;
	*u.b++ = (be ? (c >> 24) : (c >> 0)) & 0xff;
	*u.b++ = (be ? (c >> 16) : (c >> 8)) & 0xff;
	*u.b++ = (be ? (c >> 8) : (c >> 16)) & 0xff;
	*u.b = (be ? (c >> 0) : (c >> 24)) & 0xff;
	return 1;
}

int runetowchar(wchar_t *buf, Rune *rune)
{
	if (sizeof(wchar_t) == 2)
		RUNETOCHAR16(buf, rune);
	else
		RUNETOCHAR32(buf, rune);
}

#define CHAR16TORUNE(rune, str, n)                             \
	do {                                                   \
		int retval;                                    \
		Rune c;                                        \
		char16_t tmp;                                  \
		*rune = Runeerror;                             \
		if (!n)                                        \
			return 0;                              \
		if (UTF16_IS_LEADING(*str) && n == 1)          \
			return 1;                              \
		retval = 1;                                    \
		tmp = c = *str++;                              \
		if (UTF16_IS_LEADING(tmp)) {                   \
			if (UTF16_IS_TRAILING((tmp = *str))) { \
				c = (c & 0x3ff) << 10;         \
				c |= (tmp & 0x3ff);            \
				c += 0x10000;                  \
				retval++;                      \
			} else {                               \
				c = Runeerror;                 \
			}                                      \
		}                                              \
		if (validrune(c))                              \
			*rune = c;                             \
		return retval;                                 \
	} while (0)



int char16ntorune(Rune *rune, const char16_t *str, size_t n)
{
	CHAR16TORUNE(rune, str, n);
}

int char16xentorune(Rune *rune, const char16_t *str, size_t n, int be)
{
	union utf16 u;
	int retval;
	Rune c;
	char16_t tmp;

	*rune = Runeerror;
	if (!n)
		return 0;
	u.cp = str;
	tmp = be ? (u.b[1] | u.b[0] << 8) :
	           (u.b[0] | u.b[1] << 8);
	if (UTF16_IS_LEADING(tmp) && n == 1)
		return 1;
	retval = 1;
	c = tmp;
	u.p++;
	if (UTF16_IS_LEADING(tmp)) {
		tmp = be ? (u.b[1] | u.b[0] << 8) :
		           (u.b[0] | u.b[1] << 8);
		if (UTF16_IS_TRAILING((tmp))) {
			c = (c & 0x3ff) << 10; /* high surrogate */
			c |= (tmp & 0x3ff); /* low surrogate */
			c += 0x10000; /* full 21-bit code point */
			retval++;
		} else {
			c = Runeerror;
		}
	}
	if (validrune(c))
		*rune = c;
	return retval;
}

#define CHAR32TORUNE(rune, str, n) \
	do {                       \
		Rune c;            \
		*rune = Runeerror; \
		if (!n)            \
			return 0;  \
		c = *str;          \
		if (validrune(c))  \
			*rune = c; \
		return 1;          \
	} while (0)

int char32ntorune(Rune *rune, const char32_t *str, size_t n)
{
	CHAR32TORUNE(rune, str, n);
}

int char32xentorune(Rune *rune, const char32_t *str, size_t n, int be)
{
	union utf32 u;
	Rune c;

	*rune = Runeerror;
	if (!n)
		return 0;
	u.cp = str;
	c = be ? (u.b[3] | u.b[2] << 8 | u.b[1] << 16 | u.b[0] << 24) :
	         (u.b[0] | u.b[1] << 8 | u.b[2] << 16 | u.b[3] << 24);
	if (validrune(c))
		*rune = c;
	return 1;
}

int wcharntorune(Rune *rune, const wchar_t *str, size_t n)
{
	if (sizeof(wchar_t) == 2) {
		CHAR16TORUNE(rune, str, n);
	} else {
		CHAR32TORUNE(rune, str, n);
	}
}

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define UTFCONV(fromtype, fromfunc, totype, tofunc, factor)               \
	do {                                                              \
		totype **ret = retv, *buf, *dest;                         \
		const fromtype *str = strv, *ptr;                         \
		Rune rune;                                                \
		size_t i, n, len;                                         \
		int w, retval;                                            \
		for (ptr = str; *ptr; ptr++)                              \
			/* do nothing */;                                 \
		len = (size_t)((uintptr_t)ptr - (uintptr_t)str) /         \
		      sizeof(*str) + 1;                                   \
		*ret = buf = malloc(len * (factor) * sizeof(*buf));       \
		if (!buf)                                                 \
			return -1;                                        \
		dest = buf;                                               \
		for (i = 0, n = len; i < n; i += w) {                     \
			w = fromfunc(&rune, str, len);                    \
			dest += tofunc(dest, &rune);                      \
			str += w;                                         \
			len -= w;                                         \
		}                                                         \
		retval = dest - buf;                                      \
		buf = realloc(buf, retval * sizeof(*buf));                \
		if (buf)                                                  \
			*ret = buf;                                       \
		return retval - 1;                                        \
	} while (0)

int utfconv(void *retv, enum utfconv_type rettype, const void *strv,
            enum utfconv_type strtype)
{
	if (strtype == UTFCONV_UTF8) {
		if (rettype == UTFCONV_UTF8)
			UTFCONV(char, charntorune, char, runetochar,
			        runelen(Runeerror));
		else if (rettype == UTFCONV_UTF16)
			UTFCONV(char, charntorune, char16_t, runetochar16, 1);
		else if (rettype == UTFCONV_UTF16LE)
			UTFCONV(char, charntorune, char16_t, runetochar16le, 1);
		else if (rettype == UTFCONV_UTF16BE)
			UTFCONV(char, charntorune, char16_t, runetochar16be, 1);
		else if (rettype == UTFCONV_UTF32)
			UTFCONV(char, charntorune, char32_t, runetochar32, 1);
		else if (rettype == UTFCONV_UTF32LE)
			UTFCONV(char, charntorune, char32_t, runetochar32le, 1);
		else if (rettype == UTFCONV_UTF32BE)
			UTFCONV(char, charntorune, char32_t, runetochar32be, 1);
		else if (rettype == UTFCONV_WCHAR)
			UTFCONV(char, charntorune, wchar_t, runetowchar, 1);
	} else if (strtype == UTFCONV_UTF16) {
		if (rettype == UTFCONV_UTF8)
			UTFCONV(char16_t, char16ntorune, char, runetochar,
			        MAX(runelen(Runeerror), 3));
		else if (rettype == UTFCONV_UTF16)
			UTFCONV(char16_t, char16ntorune, char16_t, runetochar16,
			        (Runeerror < 0x10000) ? 1 : 2);
		else if (rettype == UTFCONV_UTF16LE)
			UTFCONV(char16_t, char16ntorune, char16_t,
			        runetochar16le, (Runeerror < 0x10000) ? 1 : 2);
		else if (rettype == UTFCONV_UTF16BE)
			UTFCONV(char16_t, char16ntorune, char16_t,
			        runetochar16be, (Runeerror < 0x10000) ? 1 : 2);
		else if (rettype == UTFCONV_UTF32)
			UTFCONV(char16_t, char16ntorune, char32_t, runetochar32,
			        1);
		else if (rettype == UTFCONV_UTF32LE)
			UTFCONV(char16_t, char16ntorune, char32_t,
			        runetochar32le, 1);
		else if (rettype == UTFCONV_UTF32BE)
			UTFCONV(char16_t, char16ntorune, char32_t,
			        runetochar32be, 1);
		else if (rettype == UTFCONV_WCHAR)
			UTFCONV(char16_t, char16ntorune, wchar_t, runetowchar,
			        (sizeof(wchar_t) == 2) ? (Runeerror < 0x10000) ?
			                                 1 : 2 : 1);
	} else if (strtype == UTFCONV_UTF16LE) {
		if (rettype == UTFCONV_UTF8)
			UTFCONV(char16_t, char16lentorune, char, runetochar,
			        MAX(runelen(Runeerror), 3));
		else if (rettype == UTFCONV_UTF16)
			UTFCONV(char16_t, char16lentorune, char16_t,
			        runetochar16, (Runeerror < 0x10000) ? 1 : 2);
		else if (rettype == UTFCONV_UTF16LE)
			UTFCONV(char16_t, char16lentorune, char16_t,
			        runetochar16le, (Runeerror < 0x10000) ? 1 : 2);
		else if (rettype == UTFCONV_UTF16BE)
			UTFCONV(char16_t, char16lentorune, char16_t,
			        runetochar16be, (Runeerror < 0x10000) ? 1 : 2);
		else if (rettype == UTFCONV_UTF32)
			UTFCONV(char16_t, char16lentorune, char32_t,
			        runetochar32, 1);
		else if (rettype == UTFCONV_UTF32LE)
			UTFCONV(char16_t, char16lentorune, char32_t,
			        runetochar32le, 1);
		else if (rettype == UTFCONV_UTF32BE)
			UTFCONV(char16_t, char16lentorune, char32_t,
			        runetochar32be, 1);
		else if (rettype == UTFCONV_WCHAR)
			UTFCONV(char16_t, char16lentorune, wchar_t, runetowchar,
			        (sizeof(wchar_t) == 2) ? (Runeerror < 0x10000) ?
			                                 1 : 2 : 1);
	} else if (strtype == UTFCONV_UTF16BE) {
		if (rettype == UTFCONV_UTF8)
			UTFCONV(char16_t, char16bentorune, char, runetochar,
			        MAX(runelen(Runeerror), 3));
		else if (rettype == UTFCONV_UTF16)
			UTFCONV(char16_t, char16bentorune, char16_t,
			        runetochar16, (Runeerror < 0x10000) ? 1 : 2);
		else if (rettype == UTFCONV_UTF16LE)
			UTFCONV(char16_t, char16bentorune, char16_t,
			        runetochar16le, (Runeerror < 0x10000) ? 1 : 2);
		else if (rettype == UTFCONV_UTF16BE)
			UTFCONV(char16_t, char16bentorune, char16_t,
			        runetochar16be, (Runeerror < 0x10000) ? 1 : 2);
		else if (rettype == UTFCONV_UTF32)
			UTFCONV(char16_t, char16bentorune, char32_t,
			        runetochar32, 1);
		else if (rettype == UTFCONV_UTF32LE)
			UTFCONV(char16_t, char16bentorune, char32_t,
			        runetochar32le, 1);
		else if (rettype == UTFCONV_UTF32BE)
			UTFCONV(char16_t, char16bentorune, char32_t,
			        runetochar32be, 1);
		else if (rettype == UTFCONV_WCHAR)
			UTFCONV(char16_t, char16bentorune, wchar_t, runetowchar,
			        (sizeof(wchar_t) == 2) ? (Runeerror < 0x10000) ?
			                                 1 : 2 : 1);
	} else if (strtype == UTFCONV_UTF32) {
		if (rettype == UTFCONV_UTF8)
			UTFCONV(char32_t, char32ntorune, char, runetochar,
			        UTFmax);
		else if (rettype == UTFCONV_UTF16)
			UTFCONV(char32_t, char32ntorune, char16_t, runetochar16,
			        2);
		else if (rettype == UTFCONV_UTF16LE)
			UTFCONV(char32_t, char32ntorune, char16_t,
			        runetochar16le, 2);
		else if (rettype == UTFCONV_UTF16BE)
			UTFCONV(char32_t, char32ntorune, char16_t,
			        runetochar16be, 2);
		else if (rettype == UTFCONV_UTF32)
			UTFCONV(char32_t, char32ntorune, char32_t,
			        runetochar32, 1);
		else if (rettype == UTFCONV_UTF32LE)
			UTFCONV(char32_t, char32ntorune, char32_t,
			        runetochar32le, 1);
		else if (rettype == UTFCONV_UTF32BE)
			UTFCONV(char32_t, char32ntorune, char32_t,
			        runetochar32be, 1);
		else if (rettype == UTFCONV_WCHAR)
			UTFCONV(char32_t, char32ntorune, wchar_t, runetowchar,
			        (sizeof(wchar_t) == 2) ? 2 : 1);
	} else if (strtype == UTFCONV_UTF32LE) {
		if (rettype == UTFCONV_UTF8)
			UTFCONV(char32_t, char32lentorune, char, runetochar,
			        UTFmax);
		else if (rettype == UTFCONV_UTF16)
			UTFCONV(char32_t, char32lentorune, char16_t,
			        runetochar16, 2);
		else if (rettype == UTFCONV_UTF16LE)
			UTFCONV(char32_t, char32lentorune, char16_t,
			        runetochar16le, 2);
		else if (rettype == UTFCONV_UTF16BE)
			UTFCONV(char32_t, char32lentorune, char16_t,
			        runetochar16be, 2);
		else if (rettype == UTFCONV_UTF32)
			UTFCONV(char32_t, char32lentorune, char32_t,
			        runetochar32, 1);
		else if (rettype == UTFCONV_UTF32LE)
			UTFCONV(char32_t, char32lentorune, char32_t,
			        runetochar32le, 1);
		else if (rettype == UTFCONV_UTF32BE)
			UTFCONV(char32_t, char32lentorune, char32_t,
			        runetochar32be, 1);
		else if (rettype == UTFCONV_WCHAR)
			UTFCONV(char32_t, char32lentorune, wchar_t, runetowchar,
			        (sizeof(wchar_t) == 2) ? 2 : 1);
	} else if (strtype == UTFCONV_UTF32BE) {
		if (rettype == UTFCONV_UTF8)
			UTFCONV(char32_t, char32bentorune, char, runetochar,
			        UTFmax);
		else if (rettype == UTFCONV_UTF16)
			UTFCONV(char32_t, char32bentorune, char16_t,
			        runetochar16, 2);
		else if (rettype == UTFCONV_UTF16LE)
			UTFCONV(char32_t, char32bentorune, char16_t,
			        runetochar16le, 2);
		else if (rettype == UTFCONV_UTF16BE)
			UTFCONV(char32_t, char32bentorune, char16_t,
			        runetochar16be, 2);
		else if (rettype == UTFCONV_UTF32)
			UTFCONV(char32_t, char32bentorune, char32_t,
			        runetochar32, 1);
		else if (rettype == UTFCONV_UTF32LE)
			UTFCONV(char32_t, char32bentorune, char32_t,
			        runetochar32le, 1);
		else if (rettype == UTFCONV_UTF32BE)
			UTFCONV(char32_t, char32bentorune, char32_t,
			        runetochar32be, 1);
		else if (rettype == UTFCONV_WCHAR)
			UTFCONV(char32_t, char32bentorune, wchar_t, runetowchar,
			        (sizeof(wchar_t) == 2) ? 2 : 1);
	} else if (strtype == UTFCONV_WCHAR) {
		if (rettype == UTFCONV_UTF8)
			UTFCONV(wchar_t, wcharntorune, char, runetochar,
			        (sizeof(wchar_t) == 2) ? MAX(runelen(Runeerror),
			                                     3) : UTFmax);
		else if (rettype == UTFCONV_UTF16)
			UTFCONV(wchar_t, wcharntorune, char16_t, runetochar16,
			        (sizeof(wchar_t) == 2) ? (Runeerror < 0x10000) ?
			                                 1 : 2 : 2);
		else if (rettype == UTFCONV_UTF16LE)
			UTFCONV(wchar_t, wcharntorune, char16_t, runetochar16le,
			        (sizeof(wchar_t) == 2) ? (Runeerror < 0x10000) ?
			                                 1 : 2 : 2);
		else if (rettype == UTFCONV_UTF16BE)
			UTFCONV(wchar_t, wcharntorune, char16_t, runetochar16be,
			        (sizeof(wchar_t) == 2) ? (Runeerror < 0x10000) ?
			                                 1 : 2 : 2);
		else if (rettype == UTFCONV_UTF32)
			UTFCONV(wchar_t, wcharntorune, char32_t, runetochar32,
			        1);
		else if (rettype == UTFCONV_UTF32LE)
			UTFCONV(wchar_t, wcharntorune, char32_t, runetochar32le,
			        1);
		else if (rettype == UTFCONV_UTF32BE)
			UTFCONV(wchar_t, wcharntorune, char32_t, runetochar32be,
			        1);
		else if (rettype == UTFCONV_WCHAR)
			UTFCONV(wchar_t, wcharntorune, wchar_t, runetowchar,
			        (sizeof(wchar_t) == 2) ? (Runeerror < 0x10000) ?
			                                 1 : 2 : 1);
	}
	return -1;
}
