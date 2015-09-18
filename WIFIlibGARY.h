int _eepromStart = 0;

void setEEPROMString(int start, int len, String string) {
  int si = 0;
  for (int i = _eepromStart + start; i < _eepromStart + start + len; i++) {
    char c;
    if (si < string.length()) {
      c = string[si];
     
    } else {
      c = 0;
    }
    EEPROM.write(i, c);
    si++;
  }

}

String getEEPROMString(int start, int len) {
  String string = "";
  for (int i = _eepromStart + start; i < _eepromStart + start + len; i++) {
    //DEBUG_PRINT(i);
    byte val = EEPROM.read(i);
    if (val == 0) {
      return string;
    }
    string += char(val);
  }
  return string;
}

String urldecode(const char *src)
{
  String decoded = "";
  char a, b;
  while (*src) {
    if ((*src == '%') &&
        ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';

      decoded += char(16 * a + b);
      src += 3;
    } else if (*src == '+') {
      decoded += ' ';
      *src++;
    } else {
      decoded += *src;
      *src++;
    }
  }
  decoded += '\0';

  return decoded;
}
