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
	Rune rune;
	size_t i, len = strlen(str) + 1;

	for (i = 0; i < num; i++) {
		struct expected_rune *expected = &expected_runes[i];

		if ((uintptr_t)ptr - (uintptr_t)str >= len) {
			ok(0, "Don't read past %s", desc);
			break;
		}
		ptr += (n = chartorune(&rune, ptr));
		is(rune, expected->rune, "U+%"PRIRune,
		   "%zu. Rune of %s is: %s", i + 1, desc, expected->name);
		is(n, expected->consumes, "%d",
		   "%zu. Rune of %s consumes %d byte%s", i + 1, desc,
		   expected->consumes, expected->consumes > 1 ? "s" : "");
	}
}

int main()
{
	utf8_check("KOSME", "\xce\xba\xe1\xbd\xb9\xcf\x83\xce\xbc\xce\xb5",
	           {2, 0x03ba, "U+03BA GREEK SMALL LETTER KAPPA"},
	           {3, 0x1f79, "U+1F79 GREEK SMALL LETTER OMICRON WITH OXIA"},
	           {2, 0x03c3, "U+03C3 GREEK SMALL LETTER SIGMA"},
	           {2, 0x03bc, "U+03BC GREEK SMALL LETTER MU"},
	           {2, 0x03b5, "U+03B5 GREEK SMALL LETTER EPSILON"},
	);

	/* 2.1  First possible sequence of a certain length */

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

	utf8_check("U+200000", "\xf8\x88\x80\x80\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+4000000", "\xfc\x84\x80\x80\x80\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	/* 2.2  Last possible sequence of a certain length */

	utf8_check("U+007F", "\x7f",
	           {1, 0x007f, "U+007F DELETE"},
	);

	utf8_check("U+07FF", "\xdf\xbf",
	           {2, 0x07ff, "U+07FF"},
	);

	utf8_check("U+FFFF", "\xef\xbf\xbf",
	           {3, 0xffff, "U+FFFF"},
	);

	utf8_check("U+1FFFFF", "\xf7\xbf\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+3FFFFFF", "\xfb\xbf\xbf\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+7FFFFFFF", "\xfd\xbf\xbf\xbf\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	/* 2.3  Other boundary conditions */

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

	utf8_check("U+110000", "\xf4\x90\x80\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	/* 3.1  Unexpected continuation bytes */

	utf8_check("First continuation byte 0x80", "\x80",
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Last continuation byte 0xbf", "\xbf",
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("2 continuation bytes", "\x80\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("3 continuation bytes", "\x80\xbf\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("4 continuation bytes", "\x80\xbf\x80\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("5 continuation bytes", "\x80\xbf\x80\xbf\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("6 continuation bytes", "\x80\xbf\x80\xbf\x80\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("7 continuation bytes", "\x80\xbf\x80\xbf\x80\xbf\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	/* 3.5  Impossible bytes */

	utf8_check("fe", "\xfe",
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("ff", "\xff",
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("fe fe ff ff", "\xfe\xfe\xff\xff",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	/* 4.1  Examples of an overlong ASCII character */

	utf8_check("Overlong U+002F (2 bytes)", "\xc0\xaf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+002F (3 bytes)", "\xe0\x80\xaf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+002F (4 bytes)", "\xf0\x80\x80\xaf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+002F (5 bytes)", "\xf8\x80\x80\x80\xaf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+002F (6 bytes)", "\xfc\x80\x80\x80\x80\xaf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	/* 4.2  Maximum overlong sequences */

	utf8_check("Overlong U+007F", "\xc1\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+07FF", "\xe0\x9f\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+FFFF", "\xf0\x8f\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+1FFFFF", "\xf8\x87\xbf\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+3FFFFFF", "\xfc\x83\xbf\xbf\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	/* 4.3  Overlong representation of the NUL character */

	utf8_check("Overlong U+0000 (2 bytes)", "\xc0\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+0000 (3 bytes)", "\xe0\x80\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+0000 (4 bytes)", "\xf0\x80\x80\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+0000 (5 bytes)", "\xf8\x80\x80\x80\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("Overlong U+0000 (6 bytes)", "\xfc\x80\x80\x80\x80\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	/* 5.1 Single UTF-16 surrogates */

	utf8_check("U+D800", "\xed\xa0\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DB7F", "\xed\xad\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DB80", "\xed\xae\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DBFF", "\xed\xaf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DC00", "\xed\xb0\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DF80", "\xed\xbe\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DFFF", "\xed\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	/* 5.2 Paired UTF-16 surrogates */

	utf8_check("U+D800 U+DC00", "\xed\xa0\x80\xed\xb0\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+D800 U+DFFF", "\xed\xa0\x80\xed\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DB7F U+DC00", "\xed\xad\xbf\xed\xb0\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DB7F U+DFFF", "\xed\xad\xbf\xed\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DB80 U+DC00", "\xed\xae\x80\xed\xb0\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DB80 U+DFFF", "\xed\xae\x80\xed\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DBFF U+DC00", "\xed\xaf\xbf\xed\xb0\x80",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	utf8_check("U+DBFF U+DFFF", "\xed\xaf\xbf\xed\xbf\xbf",
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	           {1, Runeerror, "Runeerror"},
	);

	done_testing();
}
