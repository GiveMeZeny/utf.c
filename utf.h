#ifndef UTF_H
#define UTF_H

#include <stddef.h>
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

/* possible encodings for utfconv() */
enum utfconv_type {
	UTFCONV_UTF8,
	UTFCONV_UTF16,
	UTFCONV_UTF16LE,
	UTFCONV_UTF16BE,
	UTFCONV_UTF32,
	UTFCONV_UTF32LE,
	UTFCONV_UTF32BE,
	UTFCONV_WCHAR
};

/* forward declarations */
int runetochar16xe(char16_t *buf, Rune *rune, int be);
int runetochar32xe(char32_t *buf, Rune *rune, int be);
int char16xentorune(Rune *rune, const char16_t *str, size_t n, int be);
int char32xentorune(Rune *rune, const char32_t *str, size_t n, int be);

/**
 * loop_runes() - macro to loop through a utf-8 string's runes
 * @i: size_t variable holding the loop counter
 * @rune: Rune variable holding the actual decoded rune
 * @str: pointer to the string
 * @n: size of the string
 * @body: loop body that should be wrapped in {}
 */
#define loop_runes(i, rune, str, n, body)                                    \
	{                                                                    \
		const char *_ptr = (str);                                    \
		size_t _j, _n, _len = (n);                                   \
		Rune *_rune = &(rune);                                       \
		int _w;                                                      \
		for (_j = 0, _n = _len, (i) = 0; _j < _n; _j += _w, (i)++) { \
			_w = charntorune(_rune, _ptr, _len);                 \
			body                                                 \
			_ptr += _w;                                          \
			_len -= _w;                                          \
		}                                                            \
	}

/**
 * runetochar() - write a rune to a buffer
 * @buf: pointer to the buffer >= UTFmax in size
 * @rune: pointer to the rune
 *
 * If @rune is invalid, Runeerror gets written.
 *
 * Return: The number of bytes written.
 */
int runetochar(char *buf, Rune *rune);

/**
 * chartorune() - read a rune from a utf-8 string
 * @rune: pointer that receives the rune
 * @str: pointer to the string >= UTFmax in size
 *
 * If the rune at @str is invalid, @rune gets set to Runeerror and 1 is
 * returned.
 *
 * Return: The number of bytes consumed.
 *
 * Note!: If @str is smaller than UTFmax, a buffer over-read might happen.
 */
int chartorune(Rune *rune, const char *str);

/**
 * charntorune() - read a rune from a fixed-size utf-8 string
 * @rune: pointer that receives the rune
 * @str: pointer to the string
 * @n: size of the string
 *
 * Unlike chartorune(), this won't access more than @n bytes of @str. So If
 * there is no full rune within those bytes, @rune gets set to Runeerror and 1
 * is returned. If @n is 0 and not a single byte can be accessed, @rune again
 * gets set to Runeerror, but 0 is returned.
 *
 * Return: The number of bytes consumed.
 */
int charntorune(Rune *rune, const char *str, size_t n);

/**
 * runelen() - return the size of a rune in chars
 * @rune: rune to be analyzed
 *
 * Return: The number of bytes needed to convert the rune to utf-8.
 */
int runelen(Rune rune);

/**
 * runenlen() - return the size of given runes in chars
 * @rune: pointer to the runes
 * @n: number of runes
 *
 * Return: The number of bytes needed to convert the runes to utf-8.
 */
int runenlen(Rune *rune, int n);

/**
 * fullrune() - check if a string is long enough to be decoded by chartorune()
 * @str: pointer to the string
 * @n: size of the string
 *
 * This does not guarantee that @str contains legal utf-8 encoding, but
 * indicates that chartorune() can be used safely to read a rune from @str.
 * Invalid encoding is considered a full rune if @n > 0, since chartorune()
 * would read Runeerror and consume only 1 byte.
 *
 * Return: When it's safe to call chartorune() on @str 1, otherwise 0.
 */
int fullrune(const char *str, size_t n);

/**
 * validrune() - check if a rune is valid
 * @rune: rune to be analyzed
 *
 * Return: When the rune is valid 1, otherwise 0.
 */
int validrune(Rune rune);

/**
 * utflen() - return a utf-8 string's length in code points
 * @str: pointer to the null-terminated string
 *
 * Return: Length of @str in code points.
 */
size_t utflen(const char *str);

/**
 * utfnlen() - return a fixed-size utf-8 string's length in code points
 * @str: pointer to the string
 * @maxlen: size of the string
 *
 * Like utflen(), but with respect to a buffer's size.
 *
 * Return: Length of @str in code points.
 */
size_t utfnlen(const char *str, size_t maxlen);

/**
 * utfrune() - get the first occurrence of a rune in a utf-8 string
 * @str: pointer to the null-terminated string
 * @rune: rune to look for
 *
 * If @rune is Runeerror, the first invalid rune is returned, if any.
 *
 * Return: Pointer to the encoded rune in @str, or NULL.
 */
char *utfrune(const char *str, Rune rune);

/**
 * utfrrune() - get the last occurrence of a rune in a utf-8 string
 * @str: pointer to the null-terminated string
 * @rune: rune to look for
 *
 * If @rune is Runeerror, the first invalid rune is returned, if any.
 *
 * Return: Pointer to the encoded rune in @str, or NULL.
 */
char *utfrrune(const char *str, Rune rune);

/**
 * utfutf() - get the first occurrence of a utf-8 string in a utf-8 string
 * @str: pointer to the null-terminated string to examine
 * @substr: pointer to the null-terminated string to look for
 *
 * Return: Pointer to the found substring in @str, or NULL.
 */
char *utfutf(const char *str, const char *substr);

/**
 * utfvalid() - check if a utf-8 string is free of invalid encodings
 * @str: pointer to the null-terminated string
 *
 * Return: When there is neither an invalid encoding nor Runeerror in @str 1,
 *	otherwise 0.
 */
#define utfvalid(str) (!utfrune((str), Runeerror))

/**
 * runetochar16() - write a rune to a char16_t-buffer
 * @buf: pointer to the buffer >= 2 in size
 * @rune: pointer to the rune
 *
 * The rune is written to @buf in utf-16, so either one or two char16_t are
 * needed per code point. If it's invalid, Runeerror gets written.
 *
 * Return: The number of char16_t written.
 */
int runetochar16(char16_t *buf, Rune *rune);

/**
 * runetochar16le() - write a rune to a little-endian char16_t-buffer
 * @buf: pointer to the buffer >= 2 in size
 * @rune: pointer to the rune
 *
 * Same as runetochar16().
 *
 * Return: The number of char16_t written.
 */
#define runetochar16le(buf, rune) runetochar16xe((buf), (rune), 0)

/**
 * runetochar16be() - write a rune to a big-endian char16_t-buffer
 * @buf: pointer to the buffer >= 2 in size
 * @rune: pointer to the rune
 *
 * Same as runetochar16().
 *
 * Return: The number of char16_t written.
 */
#define runetochar16be(buf, rune) runetochar16xe((buf), (rune), 1)

/**
 * runetochar32() - write a rune to a char32_t-buffer
 * @buf: pointer to the buffer >= 1 in size
 * @rune: pointer to the rune
 *
 * The rune is written to @buf in utf-32, so exactly one char32_t is needed per
 * code point. If it's invalid, Runeerror gets written.
 *
 * Return: 1 as one char32_t gets written.
 */
int runetochar32(char32_t *buf, Rune *rune);

/**
 * runetochar32le() - write a rune to a little-endian char32_t-buffer
 * @buf: pointer to the buffer >= 1 in size
 * @rune: pointer to the rune
 *
 * Same as runetochar32().
 *
 * Return: 1 as one char32_t gets written.
 */
#define runetochar32le(buf, rune) runetochar32xe((buf), (rune), 0)

/**
 * runetochar32be() - write a rune to a big-endian char32_t-buffer
 * @buf: pointer to the buffer >= 1 in size
 * @rune: pointer to the rune
 *
 * Same as runetochar32().
 *
 * Return: 1 as one char32_t gets written.
 */
#define runetochar32be(buf, rune) runetochar32xe((buf), (rune), 1)

/**
 * runetowchar() - write a rune to a wchar_t-buffer
 * @buf: pointer to the buffer
 * @rune: pointer to the rune
 *
 * This equals to either runetochar16 or runetochar32, depending on
 * `sizeof(wchar_t)`.
 *
 * Return: The number of wchar_t written.
 */
int runetowchar(wchar_t *buf, Rune *rune);

/**
 * char16ntorune() - read a rune from a fixed-size utf-16 string
 * @rune: pointer that receives the rune
 * @str: pointer to the string
 * @n: size of the string
 *
 * This won't access more than @n char16_t of @str. If there is no full or an
 * invalid rune within those char16_t, @rune gets set to Runeerror and 1 is
 * returned. If @n is 0 and not a single char16_t can be accessed, @rune again
 * gets set to Runeerror, but 0 is returned.
 *
 * Return: The number of char16_t consumed.
 */
int char16ntorune(Rune *rune, const char16_t *str, size_t n);

/**
 * char16lentorune() - read a rune from a fixed-size little-endian utf-16 string
 * @rune: pointer that receives the rune
 * @str: pointer to the string
 * @n: size of the string
 *
 * Same as char16ntorune().
 *
 * Return: The number of char16_t consumed.
 */
#define char16lentorune(rune, str, n) char16xentorune((rune), (str), (n), 0)

 /**
 * char16bentorune() - read a rune from a fixed-size big-endian utf-16 string
 * @rune: pointer that receives the rune
 * @str: pointer to the string
 * @n: size of the string
 *
 * Same as char16ntorune().
 *
 * Return: The number of char16_t consumed.
 */
#define char16bentorune(rune, str, n) char16xentorune((rune), (str), (n), 1)

/**
 * char32ntorune() - read a rune from a fixed-size utf-32 string
 * @rune: pointer that receives the rune
 * @str: pointer to the string
 * @n: size of the string
 *
 * This won't access more than @n char32_t of @str. If an invalid rune is read,
 * @rune gets set to Runeerror and 1 is returned. If @n is 0 and not a single
 * char32_t can be accessed, @rune again gets set to Runeerror, but 0 is
 * returned.
 *
 * Return: The number of char32_t consumed.
 */
int char32ntorune(Rune *rune, const char32_t *str, size_t n);

/**
 * char32lentorune() - read a rune from a fixed-size little-endian utf-32 string
 * @rune: pointer that receives the rune
 * @str: pointer to the string
 * @n: size of the string
 *
 * Same as char32ntorune().
 *
 * Return: The number of char32_t consumed.
 */
#define char32lentorune(rune, str, n) char32xentorune((rune), (str), (n), 0)

 /**
 * char32bentorune() - read a rune from a fixed-size big-endian utf-32 string
 * @rune: pointer that receives the rune
 * @str: pointer to the string
 * @n: size of the string
 *
 * Same as char32ntorune().
 *
 * Return: The number of char32_t consumed.
 */
#define char32bentorune(rune, str, n) char32xentorune((rune), (str), (n), 1)

/**
 * wcharntorune() - read a rune from a fixed-size wchar_t string
 * @rune: pointer that receives the rune
 * @str: pointer to the string
 * @n: size of the string
 *
 * This equals to either char16ntorune or char32ntorune, depending on
 * `sizeof(wchar_t)`.
 *
 * Return: The number of wchar_t consumed.
 */
int wcharntorune(Rune *rune, const wchar_t *str, size_t n);

/**
 * utfconv() - convert a string to another utf encoding
 * @retv: pointer receiving a pointer to the new string
 * @rettype: encoding the new string should be created in
 * @strv: pointer to the null-terminated source string
 * @strtype: encoding the source string is in
 *
 * The pointers should match the types, so if @rettype is UTFCONV_UTF32 @retv
 * should be a pointer to `char32_t *`. If @strtype is UTFCONV_UTF8 @strv should
 * be a `char *`. UTFCONV_WCHAR uses `wchar_t *` and results in either utf-16 or
 * utf-32, depending on `sizeof(wchar_t)`.
 *
 * Return: When successful the number of code units @retv contains, otherwise -1
 *	with `*@retv == NULL` if malloc() failed. You have to free() *@retv,
 *	when you no longer need it.
 */
int utfconv(void *retv, enum utfconv_type rettype, const void *strv,
            enum utfconv_type strtype);

#endif /* UTF_H */
