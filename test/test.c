#include <stdlib.h>
#include <string.h>
#include "../utf.h"
#include "test.h"

const char *KOSME = "\xCE\xBA\xE1\xBD\xB9\xCF\x83\xCE\xBC\xCE\xB5";

TEST(test_KOSME)
{
	const char *p1 = KOSME;
	char buf[strlen(KOSME) + 1], *p2 = buf;
	Rune rune;

	ASSERT(chartorune(&rune, p1) == 2 && rune == 0x03BA);
	ASSERT(runetochar(p2, &rune) == 2);
	p1 += 2;
	p2 += 2;
	ASSERT(chartorune(&rune, p1) == 3 && rune == 0x1F79);
	ASSERT(runetochar(p2, &rune) == 3);
	p1 += 3;
	p2 += 3;
	ASSERT(chartorune(&rune, p1) == 2 && rune == 0x03C3);
	ASSERT(runetochar(p2, &rune) == 2);
	p1 += 2;
	p2 += 2;
	ASSERT(chartorune(&rune, p1) == 2 && rune == 0x03BC);
	ASSERT(runetochar(p2, &rune) == 2);
	p1 += 2;
	p2 += 2;
	ASSERT(chartorune(&rune, p1) == 2 && rune == 0x03B5);
	ASSERT(runetochar(p2, &rune) == 2);
	p1 += 2;
	p2 += 2;
	ASSERT(chartorune(&rune, p1) == 1 && rune == 0);
	ASSERT(runetochar(p2, &rune) == 1);
	ASSERT(!strcmp(KOSME, buf));
}

TEST(test_convert)
{
	char *p8;
	char16_t *p16;
	char32_t *p32;
	int n;

	n = strtou16be(&p16, KOSME, -1);
	n = u16betostr(&p8, p16, n);
	free(p16);
	n = strtou32be(&p32, p8, n);
	free(p8);
	n = u32betostr(&p8, p32, n);
	free(p32);
	ASSERT(!strcmp(KOSME, p8));
	free(p8);
}

SUITE(all_tests)
{
	RUN_TEST(test_KOSME);
	RUN_TEST(test_convert);
}

int main()
{
	RUN_SUITE(all_tests, NULL, NULL);
	REPORT();
	return 0;
}
