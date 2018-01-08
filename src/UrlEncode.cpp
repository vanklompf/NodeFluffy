#include "UrlEncode.h"
#include <cctype>

#include <cstring>

static char to_hex(char code) 
{
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

char* url_encode(const char *src)
{
  char *buf = new char[std::strlen(src) * 3 + 1];
  url_encode(src, buf);

  return buf;
}

void url_encode(const char *src, char *dst)
{
  const char *pstr = src;
  char *pbuf = dst;

  while (*pstr) 
  {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
}

