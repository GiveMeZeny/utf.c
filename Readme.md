utf.c
=====
A simple C99 library for converting from and to UTF-8, UTF-16 and UTF-32, based on plan9's libutf, but with more accurate handling of malformed UTF sequences.

Included functions are:
  * `runetochar`: like plan9's. It won't write more than `UTFMax` bytes (which is 4) and returns the number of bytes needed for the rune (uses `Runeerror` for invalid ones).
  * There's also `runetowchar`, `runetochar16`, `runetochar16be`, `runetochar16le`, `runetochar32`, `runetochar32be` and `runetochar32le`, which emit either UTF-16 or UTF-32 code units.
  * `chartorune`: reads `Runeerror` for invalid runes and returns 1, to skip one byte and try again.
  * `charntorune`: like above, but safer because of an additional buffer size parameter
  * `runelen`: returns the number of bytes `runetochar` would write.
  * `runenlen`: like above, but for multiple runes.
  * `validrune`: checks whether `runetochar` would write `Runeerror`.
  * `fullrune`: tests if a buffer might contain a full rune.
  * `utflen`: returns the number of code points in a string.
  * `utfnlen`: like `utflen` as `strnlen`.
  * `runeindex`: like `strchr`, but for runes.
  * `runerindex`: like `strrchr`.
  * `validbom`: checks if a buffer starts with a _Byte Order Mark_ for UTF-8, UTF-16 or UTF-32.
  * `bomtostr`: converts a string to UTF-8 (assumes UTF-8 if no _Byte Order Mark_ was found and replaces invalid runes in such case);
  * `str16len` and `str32len`: like `strlen`, but for `char16_t` and `char32_t`.
  * `strtowcs`, `strtou16`, `strtou16be`, `strtou16le`, `strtou32`, `strtou32be` and `strtou32le`: convert UTF-8 to UTF-16 or UTF-32.
  * `wcstostr`, `u16tostr`, `u16betostr`, `u16letostr`, `u32tostr`, `u32betostr` and `u32letostr`: convert UTF-16 or UTF-32 to UTF-8.
