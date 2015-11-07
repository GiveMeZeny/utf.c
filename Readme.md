utf.c
=====
A simple C99 library for converting from and to UTF-8/16/32, based on plan9's libutf, but with more predictable handling of malformed UTF sequences.

Like libutf:
  * `runetochar(buf, rune)`
  * `chartorune(rune, str)`
  * `runelen(rune)`
  * `runenlen(rune, n)`
  * `fullrune(str, n)`
  * `utflen(str)`
  * `utfnlen(str, maxlen)`
  * `utfrune(str, rune)`
  * `utfrrune(str, rune)`
  * `utfutf(str, substr)`

Additions:
  * `loop_runes(i, rune, str, n, body)`
  * `charntorune(rune, str, n)`
  * `validrune(rune)`
  * `utfvalid(str)`
  * `runetochar16(buf, rune)`
  * `runetochar32(buf, rune)`
  * `runetowchar(buf, rune)`
  * `char16ntorune(rune, str, n)`
  * `char32ntorune(rune, str, n)`
  * `wcharntorune(rune, str, n)`
  * `utfconv(ret, rettype, str, strtype)`
