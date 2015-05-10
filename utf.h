#ifndef UTF_H
#define UTF_H

#include <stdint.h>

#ifndef __STD_UTF_16__
typedef uint_least16_t char16_t;
#endif
#ifndef __STD_UTF_32__
typedef uint_least32_t char32_t;
#endif
typedef uint32_t Rune;

enum {
	UTFmax = 4 /* maximum bytes per rune */
};

static const Rune Runesync = 0x80; /* cannot represent part of a UTF sequence */
static const Rune Runeself = 0x80; /* rune and UTF sequences are the same */
extern Rune Runeerror; /* decoding error in UTF, can be modified */
static const Rune Runemax = 0x10ffff; /* 21-bit rune */
static const Rune Runemask = 0x1fffff; /* bits used by runes */

extern const char *const BOM_UTF8;
extern const char *const BOM_UTF16_BE;
extern const char *const BOM_UTF16_LE;
extern const char *const BOM_UTF32_BE;
extern const char *const BOM_UTF32_LE;

/* forward declarations */
int runetochar16xe(char16_t *str, Rune *rune, int be);
int runetochar32xe(char32_t *str, Rune *rune, int be);
int char16nxetorune(Rune *rune, const char16_t *str, int n, int be);
int char32xetorune(Rune *rune, const char32_t *str, int be);
int strtou16xe(char16_t **ret, const char *str, int len, int be);
int strtou32xe(char32_t **ret, const char *str, int len, int be);
int u16xetostr(char **ret, const char16_t *str, int len, int be);
int u32xetostr(char **ret, const char32_t *str, int len, int be);

/**
 * runetochar() - write rune to utf-8 buffer
 * @str: pointer to a buffer >= UTFmax in size
 * @rune: pointer to a rune
 *
 * If @rune is invalid, Runeerror gets written.
 *
 * Return: The number of bytes written.
 */
int runetochar(char *str, Rune *rune);

/* like runetochar16() or runetochar32() */
int runetowchar(wchar_t *str, Rune *rune);

/* like runetochar(), but emits utf-16 */
int runetochar16(char16_t *str, Rune *rune);
#define runetochar16be(str, rune) runetochar16xe((str), (rune), 1)
#define runetochar16le(str, rune) runetochar16xe((str), (rune), 0)

/* like runetochar(), but emits utf-32 */
int runetochar32(char32_t *str, Rune *rune);
#define runetochar32be(str, rune) runetochar32xe((str), (rune), 1)
#define runetochar32le(str, rune) runetochar32xe((str), (rune), 0)

/**
 * chartorune() - read rune from utf-8 buffer
 * @rune: pointer that receives the rune
 * @str: pointer to a buffer >= UTFmax in size
 *
 * If the rune at @str is invalid, @rune gets
 * set to Runeerror and 1 is returned.
 *
 * Return: The number of bytes read.
 */
int chartorune(Rune *rune, const char *str);

/**
 * charntorune() - read rune from utf-8 buffer
 * @rune: pointer that receives the rune
 * @str: pointer to a buffer
 * @n: buffer size of the buffer
 *
 * While chartorune() may read up to UTFmax bytes at any
 * time, charntorune() won't read more than @n bytes.
 *
 * Return: The number of bytes read.
 */
#define charntorune(rune, str, n) \
	(fullrune((str), (n)) ? chartorune((rune), (str)) : 0)

/* like char16torune() or char32torune() */
#define wchartorune(rune, str) wcharntorune((rune), (str), INT_MAX)

/* like char16ntorune() or char32ntorune() */
int wcharntorune(Rune *rune, const wchar_t *str, int n);

/* like chartorune(), but reads utf-16 */
#define char16torune(rune, str) char16ntorune((rune), (str), INT_MAX)

/* like charntorune(), but reads utf-16 */
int char16ntorune(Rune *rune, const char16_t *str, int n);
#define char16betorune(rune, str) char16xentorune((rune), (str), INT_MAX, 1)
#define char16nbetorune(rune, str, n) char16xentorune((rune), (str), (n), 1)
#define char16letorune(rune, str) char16xentorune((rune), (str), INT_MAX, 0)
#define char16nletorune(rune, str, n) char16xentorune((rune), (str), (n), 0)

/* like chartorune(), but reads utf-32 */
int char32torune(Rune *rune, const char32_t *str);
#define char32betorune(rune, str) char32xetorune((rune), (str), 1)
#define char32letorune(rune, str) char32xetorune((rune), (str), 0)

/**
 * runelen() - return rune size in chars
 * @rune: rune to be analyzed
 *
 * Return: The number of bytes needed to convert @rune to utf-8.
 */
int runelen(Rune rune);

/**
 * runenlen() - return size of runes in chars
 * @rune: pointer to runes
 * @n: number of runes
 *
 * Return: The number of bytes needed to convert the @n runes to utf-8.
 */
int runenlen(Rune *rune, int n);

/**
 * validrune() - check if a rune is valid
 * @rune: rune to be analyzed
 *
 * Return: When @rune is valid 1, otherwise 0.
 */
int validrune(Rune rune);

/**
 * fullrune() - check if a buffer is big enough for a rune
 * @str: pointer to a buffer
 * @n: size of the buffer
 *
 * This does not guarantee that @str contains legal utf-8
 * encoding, but tells you that a full rune has arrived.
 *
 * Return: When @str is big enough to contain a full rune.
 */
int fullrune(const char *str, int n);

/**
 * runeindex() - search a rune in a buffer
 * @str: pointer to a buffer
 * @rune: rune to search
 *
 * If @rune is Runeerror, the first invalid
 * rune is returned, if any.
 *
 * Return: Pointer to the encoded rune in @str, or NULL.
 */
char *runeindex(const char *str, Rune rune);

/**
 * runerindex() - search a rune in a buffer from right-to-left
 * @str: pointer to a buffer
 * @rune: rune to search
 *
 * Return: Pointer to the encoded rune in @str, or NULL.
 */
char *runerindex(const char *str, Rune rune);

/* like bomtostr(), `len` is the length of `str` in code units */
int strtowcs(wchar_t **ret, const char *str, int len);
int strtou16(char16_t **ret, const char *str, int len);
#define strtou16be(ret, str, len) strtou16xe((ret), (str), (len), 1)
#define strtou16le(ret, str, len) strtou16xe((ret), (str), (len), 0)
int strtou32(char32_t **ret, const char *str, int len);
#define strtou32be(ret, str, len) strtou32xe((ret), (str), (len), 1)
#define strtou32le(ret, str, len) strtou32xe((ret), (str), (len), 0)
int wcstostr(char **ret, const wchar_t *str, int len);
int u16tostr(char **ret, const char16_t *str, int len);
#define u16betostr(ret, str, len) u16xetostr((ret), (str), (len), 1)
#define u16letostr(ret, str, len) u16xetostr((ret), (str), (len), 0)
int u32tostr(char **ret, const char32_t *str, int len);
#define u32betostr(ret, str, len) u32xetostr((ret), (str), (len), 1)
#define u32letostr(ret, str, len) u32xetostr((ret), (str), (len), 0)

/**
 * validbom() - check for a valid bom
 * @str: pointer to a buffer
 * @len: size of the buffer in bytes
 *
 * Return: Pointer to the found bom, or NULL if none was found.
 */
const char *validbom(const void *str, int len);

/**
 * bomtostr() - convert a string to utf-8
 * @ret: pointer to a pointer receiving the allocated result
 * @str: pointer to an utf-8/-16/-32 string
 * @len: size of the string in bytes
 *
 * If @len is -1, @str is assumed to be null-terminated.
 * If no bom was found, utf-8 is assumed.
 *
 * Return: Length of the result in code units, or -1 if malloc() failed.
 */
int bomtostr(char **ret, const void *str, int len);

/**
 * utflen() - get string length in code points
 * @str: pointer to an utf-8 string
 *
 * This might read beyond @str's end, if e.g. the
 * very last byte indicates a 3-byte sequence.
 *
 * Return: Length of @str in code points.
 */
size_t utflen(const char *str);

/**
 * utfnlen() - get string length in code points
 * @str: pointer to an utf-8 string
 * @maxlen: size of the buffer
 *
 * This returns the number of code points in @str,
 * without reading beyond the buffer's end.
 *
 * Return: Length of @str in code points.
 */
size_t utfnlen(const char *str, size_t maxlen);

/* like strlen(), but for `char16_t` */
size_t str16len(const char16_t *str);

/* like strlen(), but for `char32_t` */
size_t str32len(const char32_t *str);

#endif /* UTF_H */
