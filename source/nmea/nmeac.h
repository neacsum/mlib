#pragma once

#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <nmea.h>

struct parse_context {
  char *lcl;            //local copy of string to parse
  char *toparse;        //parsing position in local string
  char saved;           //char replaced with NULL

  parse_context (const char *buf)
  {
    toparse = lcl = _strdup (buf);
    saved = 0;
  };

  ~parse_context ()
  {
    free (lcl);
  };
};

char *token (parse_context& ctx);

#define NEXT_TOKEN(A, B) if ((A = token(ctx)) == NULL)  return B
#define NEXT_VALIDTOKEN(A, B) if ((A = token(ctx)) == NULL || !strlen(A))  return B
#define IFPAR(par, exp) if (par) *par = (exp)
