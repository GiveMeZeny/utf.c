#include "tap.h"
#include "utf.h"

struct expected_rune {
	int consumes;
	Rune rune;
	const char *name;
};

#define utf8_check(desc, str, ...)                                   \
	(utf8_check)((desc), (str),                                  \
	             sizeof((struct expected_rune[]){__VA_ARGS__}) / \
	             sizeof(struct expected_rune),                   \
	             ((struct expected_rune[]){__VA_ARGS__}))

void (utf8_check)(const char *desc, const char *str, size_t num,
                  struct expected_rune *expected_runes)
{
	const char *ptr = str;
	int n;
	size_t i, len = strlen(str) + 1;

	for (i = 0; i < num; i++) {
		struct expected_rune *expected = &expected_runes[i];
		char buf[UTFmax];

		n = runetochar(buf, &expected->rune);
		ismem(ptr, buf, n < expected->consumes ? n : expected->consumes,
		      "The Rune %s is encoded like in the string %s",
		      expected->name, desc);
		is(n, expected->consumes, "%d",
		   "The Rune %s consumes %d byte%s", desc, expected->consumes,
		   expected->consumes > 1 ? "s" : "");
		if (len - ((uintptr_t)ptr - (uintptr_t)str) < (size_t)n) {
			ok(0, "Don't read past %s", desc);
			break;
		}
		ptr += n;
	}
}

int main()
{
	char runeerror[UTFmax * 2] = {0};

	runetochar(runeerror, &Runeerror);

	utf8_check("KOSME", "\xce\xba\xe1\xbd\xb9\xcf\x83\xce\xbc\xce\xb5",
	           {2, 0x03ba, "U+03BA GREEK SMALL LETTER KAPPA"},
	           {3, 0x1f79, "U+1F79 GREEK SMALL LETTER OMICRON WITH OXIA"},
	           {2, 0x03c3, "U+03C3 GREEK SMALL LETTER SIGMA"},
	           {2, 0x03bc, "U+03BC GREEK SMALL LETTER MU"},
	           {2, 0x03b5, "U+03B5 GREEK SMALL LETTER EPSILON"},
	);

	utf8_check("U+0000", "\x00",
	           {1, 0x0000, "U+0000 NULL"},
	);

	utf8_check("U+0080", "\xc2\x80",
	           {2, 0x0080, "U+0080 <control>"},
	);

	utf8_check("U+0800", "\xe0\xa0\x80",
	           {3, 0x0800, "U+0800 SAMARITAN LETTER ALAF"},
	);

	utf8_check("U+10000", "\xf0\x90\x80\x80",
	           {4, 0x10000, "U+10000 LINEAR B SYLLABLE B008 A"},
	);

	utf8_check("U+200000", runeerror,
	           {runelen(Runeerror), 0x200000, "U+200000"},
	);

	utf8_check("U+4000000", runeerror,
	           {runelen(Runeerror), 0x4000000, "U+4000000"},
	);

	utf8_check("U+007F", "\x7f",
	           {1, 0x007f, "U+007F DELETE"},
	);

	utf8_check("U+07FF", "\xdf\xbf",
	           {2, 0x07ff, "U+07FF"},
	);

	utf8_check("U+FFFF", "\xef\xbf\xbf",
	           {3, 0xffff, "U+FFFF"},
	);

	utf8_check("U+1FFFFF", runeerror,
	           {runelen(Runeerror), 0x1FFFFF, "U+1FFFFF"},
	);

	utf8_check("U+3FFFFFF", runeerror,
	           {runelen(Runeerror), 0x3FFFFFF, "U+3FFFFFF"},
	);

	utf8_check("U+7FFFFFFF", runeerror,
	           {runelen(Runeerror), 0x7fffffff, "U+7FFFFFFF"},
	);

	utf8_check("U+D7FF", "\xed\x9f\xbf",
	           {3, 0xd7ff, "U+D7FF"},
	);

	utf8_check("U+E000", "\xee\x80\x80",
	           {3, 0xe000, "U+E000 <Private Use, First>"},
	);

	utf8_check("U+FFFD", "\xef\xbf\xbd",
	           {3, 0xfffd, "U+FFFD REPLACEMENT CHARACTER"},
	);

	utf8_check("U+10FFFF", "\xf4\x8f\xbf\xbf",
	           {4, 0x10ffff, "U+10FFFF"},
	);

	utf8_check("U+110000", runeerror,
	           {runelen(Runeerror), 0x110000, "U+110000"},
	);

	utf8_check("U+D800", runeerror,
	           {runelen(Runeerror), 0xd800, "U+D800"},
	);

	utf8_check("U+DB7F", runeerror,
	           {runelen(Runeerror), 0xdb7f, "U+DB7F"},
	);

	utf8_check("U+DB80", runeerror,
	           {runelen(Runeerror), 0xdb80, "U+DB80"},
	);

	utf8_check("U+DBFF", runeerror,
	           {runelen(Runeerror), 0xdbff, "U+DBFF"},
	);

	utf8_check("U+DC00", runeerror,
	           {runelen(Runeerror), 0xdc00, "U+DC00"},
	);

	utf8_check("U+DF80", runeerror,
	           {runelen(Runeerror), 0xdf80, "U+DF80"},
	);

	utf8_check("U+DFFF", runeerror,
	           {runelen(Runeerror), 0xdfff, "U+DFFF"},
	);

	done_testing();
}
