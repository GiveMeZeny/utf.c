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
static const Rune Runeerror = 0xfffd; /* decoding error in UTF */
static const Rune Runemax = 0x10ffff; /* 21-bit rune */
static const Rune Runemask = 0x1fffff; /* bits used by runes */

extern char *BOM_UTF8;
extern char *BOM_UTF16_BE;
extern char *BOM_UTF16_LE;
extern char *BOM_UTF32_BE;
extern char *BOM_UTF32_LE;

/* forward declarations */
int runetochar16xe(char16_t *str, Rune *rune, int be);
int runetochar32xe(char32_t *str, Rune *rune, int be);
int char16nxetorune(Rune *rune, const char16_t *str, int n, int be);
int char32xetorune(Rune *rune, const char32_t *str, int be);
int strtoutf16xe(char16_t **ret, const char *str, int len, int be);
int strtoutf32xe(char32_t **ret, const char *str, int len, int be);
int utf16xetostr(char **ret, const char16_t *str, int len, int be);
int utf32xetostr(char **ret, const char32_t *str, int len, int be);

int runetochar(char *str, Rune *rune);
int runetowchar(wchar_t *str, Rune *rune);
int runetochar16(char16_t *str, Rune *rune);
#define runetochar16be(str, rune) runetochar16xe((str), (rune), 1)
#define runetochar16le(str, rune) runetochar16xe((str), (rune), 0)
int runetochar32(char32_t *str, Rune *rune);
#define runetochar32be(str, rune) runetochar32xe((str), (rune), 1)
#define runetochar32le(str, rune) runetochar32xe((str), (rune), 0)
int chartorune(Rune *rune, const char *str);
#define charntorune(rune, str, n) \
	(fullrune((str), (n)) ? chartorune((rune), (str)) : 0)
#define wchartorune(rune, str) wcharntorune((rune), (str), INT_MAX)
int wcharntorune(Rune *rune, const wchar_t *str, int n);
#define char16torune(rune, str) char16ntorune((rune), (str), INT_MAX)
int char16ntorune(Rune *rune, const char16_t *str, int n);
#define char16betorune(rune, str) char16xentorune((rune), (str), INT_MAX, 1)
#define char16letorune(rune, str) char16xentorune((rune), (str), INT_MAX, 0)
#define char16nbetorune(rune, str, n) char16xentorune((rune), (str), (n), 1)
#define char16nletorune(rune, str, n) char16xentorune((rune), (str), (n), 0)
int char32torune(Rune *rune, const char32_t *str);
#define char32betorune(rune, str) char32xetorune((rune), (str), 1)
#define char32letorune(rune, str) char32xetorune((rune), (str), 0)
int runelen(Rune rune);
int runenlen(Rune *rune, int n);
int validrune(Rune rune);
int fullrune(const char *str, int n);
char *runeindex(const char *str, Rune rune);
char *runerindex(const char *str, Rune rune);
int strtowcs(wchar_t **ret, const char *str, int len);
int strtou16(char16_t **ret, const char *str, int len);
#define strtou16be(ret, str, len) strtoutf16xe((ret), (str), (len), 1)
#define strtou16le(ret, str, len) strtoutf16xe((ret), (str), (len), 0)
int strtou32(char32_t **ret, const char *str, int len);
#define strtou32be(ret, str, len) strtoutf32xe((ret), (str), (len), 1)
#define strtou32le(ret, str, len) strtoutf32xe((ret), (str), (len), 0)
int wcstostr(char **ret, const wchar_t *str, int len);
int u16tostr(char **ret, const char16_t *str, int len);
#define u16betostr(ret, str, len) utf16xetostr((ret), (str), (len), 1)
#define u16letostr(ret, str, len) utf16xetostr((ret), (str), (len), 0)
int u32tostr(char **ret, const char32_t *str, int len);
#define u32betostr(ret, str, len) utf32xetostr((ret), (str), (len), 1)
#define u32letostr(ret, str, len) utf32xetostr((ret), (str), (len), 0)
int bomtostr(char **ret, const void *str, int len);
size_t str16len(const char16_t *str);
size_t str32len(const char32_t *str);

#endif /* UTF_H */
