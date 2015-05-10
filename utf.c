#include <stdlib.h>
#include <string.h>
#include <wchar.h>
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

const char *const BOM_UTF8 = "\xef\xbb\xbf";
const char *const BOM_UTF16_BE = "\xfe\xff";
const char *const BOM_UTF16_LE = "\xff\xfe";
const char *const BOM_UTF32_BE = "\x00\x00\xfe\xff";
const char *const BOM_UTF32_LE = "\xff\xfe\x00\x00";

/* return 1 if it's just ascii */
#define UTF8_IS_ASCII(c) (((unsigned char)(c) & 0x80) != 0x80)
/* return 1 if continuation bytes are introduced */
static inline int UTF8_IS_LEADING(unsigned char c) {return c >> 7 & c >> 6;}
/* return 1 if it's a continuation byte */
static inline int UTF8_IS_TRAILING(unsigned char c) {return c >> 7 & ~c >> 6;}
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

/* helper macro for runetochar16 */
#define RUNETOCHAR16(str, c)                             \
	do {                                             \
		if (c <= 0xffff) {                       \
			*str = c;                        \
			return 1;                        \
		}                                        \
		c -= 0x10000;                            \
		*str++ = (((c >> 10) & 0x3ff) | 0xd800); \
		*str = ((c & 0x3ff) | 0xdc00);           \
		return 2;                                \
	} while (0)

/* helper macro for runetochar32 */
#define RUNETOCHAR32(str, c) \
	do {                 \
		*str = c;    \
		return 1;    \
	} while (0)

/* helper macro for char16torune */
#define CHAR16TORUNE(c, str)                                   \
	do {                                                   \
		char16_t tmp = c = *str++;                     \
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
	} while (0)

/* helper macro for char32torune */
#define CHAR32TORUNE(c, str) c = *str

/* helper macro for the helper macros below */
#define CONVSTR_(str, len, strlen, factor, ret, totype, from, to)         \
	do {                                                              \
		int retval, skip, n = (len < 0) ? (int)strlen(str) : len; \
		totype *buf, *dest;                                       \
		Rune c;                                                   \
		buf = malloc(n * factor + sizeof(*buf));                  \
		if (!buf)                                                 \
			return -1;                                        \
		dest = buf;                                               \
		do {                                                      \
			skip = from;                                      \
			if (!skip)                                        \
				break;                                    \
			str += skip;                                      \
			dest += to;                                       \
			n -= skip;                                        \
		} while (c && n);                                         \
		if (c) {                                                  \
			c = 0;                                            \
			dest += to;                                       \
		}                                                         \
		retval = dest - buf;                                      \
		dest = realloc(buf, retval * sizeof(*buf));               \
		*ret = dest ? dest : buf;                                 \
		return retval - 1;                                        \
	} while (0)

/* helper macro for converting from/to utf-8 */
#define CONVSTRN(str, len, strlen, factor, ret, totype, from, to) \
	CONVSTR_(str, len, strlen, factor, ret, totype,           \
		 (from(&c, str, n)), (to(dest, &c)))

/* helper macro for converting from utf-32 to utf-8 */
#define CONVSTR(str, len, strlen, factor, ret, totype, from, to) \
	CONVSTR_(str, len, strlen, factor, ret, totype,          \
		 (from(&c, str)), (to(dest, &c)))

/* helper macro for converting from utf-16 le/be to utf-8 */
#define CONVSTRNFROMXE(str, len, strlen, factor, ret, totype, from, to, be) \
	CONVSTR_(str, len, strlen, factor, ret, totype,                     \
		 (from(&c, str, n, be)), (to(dest, &c)))

/* helper macro for converting from utf-32 le/be to utf-8 */
#define CONVSTRFROMXE(str, len, strlen, factor, ret, totype, from, to, be) \
	CONVSTR_(str, len, strlen, factor, ret, totype,                    \
		 (from(&c, str, be)), (to(dest, &c)))

/* helper macro for converting from utf-8 to utf-16/-32 le/be */
#define CONVSTRNTOXE(str, len, strlen, factor, ret, totype, from, to, be) \
	CONVSTR_(str, len, strlen, factor, ret, totype,                   \
		 (from(&c, str, n)), (to(dest, &c, be)))

int runetochar(char *str, Rune *rune)
{
	union utf8 u = {.pc = str};
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

int runetowchar(wchar_t *str, Rune *rune)
{
	Rune c = *rune;

	if (!validrune(c))
		c = Runeerror;
	if (sizeof(wchar_t) == 2)
		RUNETOCHAR16(str, c);
	else
		RUNETOCHAR32(str, c);
}

int runetochar16(char16_t *str, Rune *rune)
{
	Rune c = *rune;

	if (!validrune(c))
		c = Runeerror;
	RUNETOCHAR16(str, c);
}

int runetochar16xe(char16_t *str, Rune *rune, int be)
{
	union utf16 u = {.p = str};
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

int runetochar32(char32_t *str, Rune *rune)
{
	Rune c = *rune;

	if (!validrune(c))
		c = Runeerror;
	RUNETOCHAR32(str, c);
}

int runetochar32xe(char32_t *str, Rune *rune, int be)
{
	union utf32 u = {.p = str};
	Rune c = *rune;

	if (!validrune(c))
		c = Runeerror;
	*u.b++ = (be ? (c >> 24) : (c >> 0)) & 0xff;
	*u.b++ = (be ? (c >> 16) : (c >> 8)) & 0xff;
	*u.b++ = (be ? (c >> 8) : (c >> 16)) & 0xff;
	*u.b = (be ? (c >> 0) : (c >> 24)) & 0xff;
	return 1;
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

int wcharntorune(Rune *rune, const wchar_t *str, int n)
{
	int retval = 1;
	Rune c;

	if (n <= 0)
		return 0;
	if (sizeof(wchar_t) == 2) {
		if (UTF16_IS_LEADING(*str) && n < 2)
			return 0;
		CHAR16TORUNE(c, str);
	} else {
		CHAR32TORUNE(c, str);
	}
	*rune = validrune(c) ? c : Runeerror;
	return retval;
}

int char16ntorune(Rune *rune, const char16_t *str, int n)
{
	int retval = 1;
	Rune c;

	if (n <= 0 || (UTF16_IS_LEADING(*str) && n < 2))
		return 0;
	CHAR16TORUNE(c, str);
	*rune = validrune(c) ? c : Runeerror;
	return retval;
}

int char16nxetorune(Rune *rune, const char16_t *str, int n, int be)
{
	union utf16 u = {.cp = str};
	char16_t tmp;
	int retval = 1;
	Rune c;

	if (n <= 0)
		return 0;
	tmp = be ? (u.b[1] | u.b[0] << 8) :
		   (u.b[0] | u.b[1] << 8);
	if (UTF16_IS_LEADING(tmp) && n < 2)
		return 0;
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
	*rune = validrune(c) ? c : Runeerror;
	return retval;
}

int char32torune(Rune *rune, const char32_t *str)
{
	int retval = 1;
	Rune c;

	CHAR32TORUNE(c, str);
	*rune = validrune(c) ? c : Runeerror;
	return retval;
}

int char32xetorune(Rune *rune, const char32_t *str, int be)
{
	union utf32 u = {.cp = str};
	Rune c = be ? (u.b[3] | u.b[2] << 8 | u.b[1] << 16 | u.b[0] << 24) :
		      (u.b[0] | u.b[1] << 8 | u.b[2] << 16 | u.b[3] << 24);

	*rune = validrune(c) ? c : Runeerror;
	return 1;
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

int fullrune(const char *str, int n)
{
	int t = utf8_trail_cnt(*str);

	if (n <= 0)
		return 0;
	if (t < 0 || t >= UTFmax)
		return 1;
	else
		return n >= t + 1;
}

int validrune(Rune rune)
{
	if (rune > Runemax || (rune & 0xf800) == 0xd800)
		return 0;
	else
		return 1;
}

char *runeindex(const char *str, Rune rune)
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
		int n = chartorune(&c, u.pc);

		if (c == rune)
			return u.pc;
		u.pc += n;
	} while (c);
	return NULL;
}

char *runerindex(const char *str, Rune rune)
{
	union utf8 u = {.cp = str};
	char *save = NULL;
	Rune c;

	if (rune < Runeself)
		return strrchr(str, rune);
	do {
		int n = chartorune(&c, u.pc);

		if (c == rune)
			save = u.pc;
		u.pc += n;
	} while (c);
	return save;
}

int strtowcs(wchar_t **ret, const char *str, int len)
{
	/* worst-case: like `strtou16` and `strtou32` */
	CONVSTRN(str, len, strlen, sizeof(wchar_t), ret, wchar_t,
		 charntorune, runetowchar);
}

int strtou16(char16_t **ret, const char *str, int len)
{
	/* worst-case: 1x utf8-unit makes 1x utf16-unit */
	CONVSTRN(str, len, strlen, sizeof(char16_t), ret, char16_t,
		 charntorune, runetochar16);
}

int strtou16xe(char16_t **ret, const char *str, int len, int be)
{
	/* worst-case: like `strtou16` */
	CONVSTRNTOXE(str, len, strlen, sizeof(char16_t), ret, char16_t,
		     charntorune, runetochar16xe, be);
}

int strtou32(char32_t **ret, const char *str, int len)
{
	/* worst-case: 1x utf8-unit makes 1x utf32-unit */
	CONVSTRN(str, len, strlen, sizeof(char32_t), ret, char32_t,
		 charntorune, runetochar32);
}

int strtou32xe(char32_t **ret, const char *str, int len, int be)
{
	/* worst-case: like `strtou32` */
	CONVSTRNTOXE(str, len, strlen, sizeof(char32_t), ret, char32_t,
		     charntorune, runetochar32xe, be);
}

int wcstostr(char **ret, const wchar_t *str, int len)
{
	/* worst-case: like `u32tostr` */
	CONVSTRN(str, len, wcslen, 4, ret, char,
		 wcharntorune, runetochar);
}

int u16tostr(char **ret, const char16_t *str, int len)
{
	/* worst-case: 1x utf16-unit make 3x utf8-units */
	CONVSTRN(str, len, str16len, 3, ret, char,
		 char16ntorune, runetochar);
}

int u16xetostr(char **ret, const char16_t *str, int len, int be)
{
	/* worst-case: like `u16tostr` */
	CONVSTRNFROMXE(str, len, str16len, 3, ret, char,
		       char16nxetorune, runetochar, be);
}

int u32tostr(char **ret, const char32_t *str, int len)
{
	/* worst-case: 1x utf32-unit make 4x utf8-units */
	CONVSTR(str, len, str32len, 4, ret, char,
		char32torune, runetochar);
}

int u32xetostr(char **ret, const char32_t *str, int len, int be)
{
	/* worst-case: like `utf32tostr` */
	CONVSTRFROMXE(str, len, str32len, 4, ret, char,
		      char32xetorune, runetochar, be);
}

const char *validbom(const void *str, int len)
{
	const char *ptr = str;

	if ((len >= 3 || len < 0) && !strncmp(ptr, BOM_UTF8, 3))
		return BOM_UTF8;
	else if ((len >= 2 || len < 0) && !strncmp(ptr, BOM_UTF16_BE, 2))
		return BOM_UTF16_BE;
	else if ((len >= 2 || len < 0) && !strncmp(ptr, BOM_UTF16_LE, 2))
		return BOM_UTF16_LE;
	else if ((len >= 4 || len < 0) && !strncmp(ptr, BOM_UTF32_BE, 4))
		return BOM_UTF32_BE;
	else if ((len >= 4 || len < 0) && !strncmp(ptr, BOM_UTF32_LE, 4))
		return BOM_UTF32_LE;
	else
		return NULL;
}

int bomtostr(char **ret, const void *str, int len)
{
	union {
		const char *cp;
		const char16_t *cp16;
		const char32_t *cp32;
	} u = {.cp = str};
	const char *bom = validbom(u.cp, len);

	if (bom == BOM_UTF16_BE)
		return u16betostr(ret, u.cp16 + 1, (len > 0) ? len / 2 : -1);
	else if (bom == BOM_UTF16_LE)
		return u16letostr(ret, u.cp16 + 1, (len > 0) ? len / 2 : -1);
	else if (bom == BOM_UTF32_BE)
		return u32betostr(ret, u.cp32 + 1, (len > 0) ? len / 4 : -1);
	else if (bom == BOM_UTF32_LE)
		return u32letostr(ret, u.cp32 + 1, (len > 0) ? len / 4 : -1);
	else if (bom == BOM_UTF8)
		u.cp += 3;
	CONVSTRN(u.cp, len, strlen, 2, ret, char, charntorune, runetochar);
}

size_t utflen(const char *str)
{
	size_t len;
	Rune rune;
	int n;

	for (len = 0; (n = chartorune(&rune, str)); str += n, len++)
		if (!rune)
			break;
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

size_t str16len(const char16_t *str)
{
	const char16_t *ptr;

	for (ptr = str; *ptr; ptr++)
		/* do nothing */;
	return ptr - str;
}

size_t str32len(const char32_t *str)
{
	const char32_t *ptr;

	for (ptr = str; *ptr; ptr++)
		/* do nothing */;
	return ptr - str;
}
