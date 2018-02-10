/*!
  \file PROFILE.CPP Implementation of Profile

  (c) Mircea Neacsu 2017

  Reimplemented to UTF-8 compatibility with code base on:
  minIni - Multi-Platform INI file parser, suitable for embedded systems

  These routines are in part based on the article "Multiplatform .INI Files"
  by Joseph J. Graf in the March 1994 issue of Dr. Dobb's Journal.

  Copyright (c) CompuPhase, 2008-2012

  Licensed under the Apache License, Version 2.0 (the "License"); you may not
  use this file except in compliance with the License. You may obtain a copy
  of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
  License for the specific language governing permissions and limitations
  under the License.

*/
#ifndef UNICODE
#define UNICODE
#endif


#include <mlib/profile.h>
#include <stdio.h>
#include <string.h>
#include <math.h>        // for atof
#include <stdlib.h>      // for atoi
#include <assert.h>
#include <mlib/utf8.h>
#include <mlib/trace.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

static int getkeystring (FILE* fp, const char* Section, const char* Key, char* Buffer, int BufferSize);
static char* skipleading (char* str);
static char* skiptrailing (char* str, char* base);
static char *striptrailing(char *str);
inline static FILE *ini_openread (const char* fname) {return _wfopen (utf8::widen(fname).c_str(), L"rb, ccs=UTF-8");};
inline static FILE *ini_openwrite (const char* fname) {return _wfopen (utf8::widen(fname).c_str(), L"wb, ccs=UTF-8");};
static void ini_tempname (char* dest, const char* source, int maxlength);
static void writesection (char* LocalBuffer, const char* Section, FILE *fp);
static void writekey (char* buffer, const char* key, const char* value, FILE *fp);
static int cache_accum (const char* string, int* size, int max);
static int cache_flush (char* buffer, int* size, FILE* rfp, FILE* wfp, fpos_t* mark);
static bool close_rename(FILE* rfp, FILE* wfp, const char* filename);
static char* cleanstring (char* string);
static bool ini_puts (const char *key, const char *value, const char *section, char *filename);

using namespace std;

/*!
  \class Profile

  Profile class provides a handy object oriented encapsulation of functions
  needed to manipulate INI (profile) files.
  \pre file != NULL
*/
Profile::Profile (const char *file)
{
  assert (file);
  strcpy( filename, file );
  temp_file = false;
}

/*!
  Creates a temporary file as filename.
*/
Profile::Profile ()
{
  wchar_t tmp[_MAX_PATH];
  GetTempFileName (L".", L"INI", 0, tmp);
  strcpy (filename, utf8::narrow(tmp).c_str());
  temp_file = true;
}

/*!
  Allocate space and copies the filename from the passed object.
*/
Profile::Profile (const Profile& p)
{
  strcpy (filename, p.filename);
  temp_file = false;
}

/*!
  Releases the space used by filename.
*/
Profile::~Profile()
{
  if (temp_file)
    utf8::remove (filename);
}

/*!
  Change the file associated with this object. If previous one was a
  temporary file, it is deleted now (loosing all settings in the process).
  
  If \p fname is NULL it creates a temporary file.
*/
void Profile::File (const char *fname)
{
  if (temp_file)
    utf8::remove (filename);

  if (fname)
  {
    strcpy (filename, fname);
    temp_file = false;
  }
  else
  {
    wchar_t tmp[_MAX_PATH];
    GetTempFileName (L".", L"INI", 0, tmp);
    strcpy (filename, utf8::narrow (tmp).c_str());
    temp_file = true;
  }
}

/*!
  Assignment operator performs a file copy of the passed object.
*/
Profile& Profile::operator = (const Profile& p)
{
  // Copy the source file to the destination file.
  CopyFile (utf8::widen (p.filename).c_str(), utf8::widen (filename).c_str(), FALSE);
  return *this;
}

/*!
  Return \b true if specified key exists in the INI file.

  \pre  key != NULL <br>
        section != NULL
*/
bool Profile::HasKey( const char *key, const char *section ) const
{
  assert (key);
  assert (section);

  char buffer[80];
  return (GetString (buffer, sizeof(buffer), key, section) != 0);
}

/*!
  \param key      key name
  \param section  section name
  \param defval   default value if key is missing
  \pre key != NULL <br>
       section != NULL
*/
int Profile::GetInt (const char *key, const char *section, int defval) const
{
  assert (key);
  assert (section);

  /*We don't use GetPrivateProfileInt because it returns
  an UNSIGNED number which isn't very nice.*/

  char buffer[80];
  if (!GetString (buffer, sizeof(buffer), key, section))
   return defval;
  return atoi (buffer);
}

/*!
  \param key      key name
  \param section  section name
  \param defval   default value if key is missing
  \pre key != NULL <br>
       section != NULL
*/
double Profile::GetDouble (const char *key, const char *section, double defval) const
{
  char value[80];

  assert (key);
  assert (section);

  if (!GetString (value, sizeof(value), key, section))
   return defval;
  return atof (value);
}

/*!
  \param key      key name
  \param value    key value
  \param section  section name
  \return         true if successful, false otherwise

  \pre key != NULL <br>
       section != NULL
*/
bool Profile::PutInt (const char *key, long value, const char *section)
{
  char buffer[35];

  assert (key);
  assert (section);

  _itoa (value, buffer, 10);
  return PutString (key, buffer, section);
}


/*!
  \param key      key name
  \param value    key value
  \param section  section name
  \param dec      number of decimals
  \return         true if successful, false otherwise

  \pre key != NULL <br>
       section != NULL
*/
bool Profile::PutDouble (const char *key, double value, const char *section, int dec)
{
  char buffer[80];

  assert (key);
  assert (section);

  sprintf (buffer, "%.*lf", dec, value);
  return PutString (key, buffer, section);
}

/*!
  Font is specified by a string containing the following comma-separated values:
  height, width, escapement, orientation, weight, italic, underline, strikeout,
  charset, precision, clip precision, quality, pitch and family, face name.

  \param key      key name
  \param section  section name
  \param defval   default value if key is missing
  \pre key != NULL <br>
       section != NULL
*/
HFONT Profile::GetFont (const char *key, const char *section, HFONT defval) const
{
  LOGFONT lfont;
  char value[255];
  char *szptr = value;

  assert (key);
  assert (section);

  GetString (value, sizeof(value), key, section);
  if (*szptr)
  {
    lfont.lfHeight = atoi (szptr);
    szptr = strchr (szptr, ',');
  }
  else if (defval)
    return defval;
  else
    return (HFONT)GetStockObject(SYSTEM_FONT);

  if (szptr && *szptr)
  {
    lfont.lfWidth = atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfWidth = 0;
  if (szptr && *szptr)
  {
    lfont.lfEscapement = atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfEscapement = 0;
  if (szptr && *szptr)
  {
    lfont.lfOrientation = atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfOrientation = 0;
  if (szptr && *szptr)
  {
    lfont.lfWeight = atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfWeight = FW_NORMAL;
  if (szptr && *szptr)
  {
    lfont.lfItalic = (BYTE)atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfItalic = 0;
  if (szptr && *szptr)
  {
    lfont.lfUnderline = (BYTE)atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfUnderline = 0;
  if (szptr && *szptr)
  {
    lfont.lfStrikeOut = (BYTE)atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfStrikeOut = 0;
  if (szptr && *szptr)
  {
    lfont.lfCharSet = (BYTE)atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfCharSet = ANSI_CHARSET;
  if (szptr && *szptr)
  {
    lfont.lfOutPrecision = (BYTE)atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
  if (szptr && *szptr)
  {
    lfont.lfClipPrecision = (BYTE)atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  if (szptr && *szptr)
  {
    lfont.lfQuality = (BYTE)atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfQuality = DEFAULT_QUALITY;
  if (szptr && *szptr)
  {
    lfont.lfPitchAndFamily = (BYTE)atoi (++szptr);
    szptr = strchr (szptr, ',');
  }
  else
    lfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  if (szptr && *szptr)
    wcscpy (lfont.lfFaceName, utf8::widen(++szptr).c_str() );
  else
    wcscpy( lfont.lfFaceName, L"Courier" );
  return CreateFontIndirect (&lfont);
}

/*!
  Color is assumed to be in the same format as written by PutColor i.e. 
  R G B numbers separated by spaces.

  \param key      key name
  \param section  section name
  \param defval   default value if key is missing

  \pre key != NULL <br>
       section != NULL
*/
COLORREF Profile::GetColor (const char *key, const char *section, COLORREF defval) const
{
  int R=0, G=0, B=0;
  char value[80];

  assert (key);
  assert (section);

  if (GetString (value, sizeof(value), key, section)
   && sscanf (value, "%i%i%i", &R, &G, &B) == 3)
    return RGB (R,G,B);
  else
    return defval;
}

/*!
  Color is saved as RGB numbers separated by spaces.

  \param key      key name
  \param section  section name
  \param c        color specification
  \return         true if successful, false otherwise

  \pre key != NULL <br>
       section != NULL
*/
bool Profile::PutColor (const char *key, COLORREF c, const char *section)
{
  char buffer[80];

  assert (key);
  assert (section);

  sprintf (buffer, "%i %i %i", GetRValue(c), GetGValue(c), GetBValue(c));
  return PutString (key, buffer, section);
}


/*!
  The format is the same as that interpreted by GetFont

  \param key      key name
  \param section  section name
  \param font     key value
  \return         true if successful, false otherwise

  \pre key != NULL <br>
       section != NULL
*/
bool Profile::PutFont (const char *key, HFONT font, const char *section)
{
  LOGFONT lfont;
  char buffer[256];

  assert (key);
  assert (section);

  if (!GetObject (font, sizeof(lfont), &lfont))
    return false;
  sprintf_s (buffer, sizeof(buffer), "%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%S",
    lfont.lfHeight,
    lfont.lfWidth,
    lfont.lfEscapement,
    lfont.lfOrientation,
    lfont.lfWeight,
    lfont.lfItalic,
    lfont.lfUnderline,
    lfont.lfStrikeOut,
    lfont.lfCharSet,
    lfont.lfOutPrecision,
    lfont.lfClipPrecision,
    lfont.lfQuality,
    lfont.lfPitchAndFamily,
    lfont.lfFaceName);
  return PutString (key, buffer, section);
}

/*!
  True values can be specified by any of "on", "yes", "true" or "1".
  Anything else is considered as FALSE. Strings are case-insensitive.

  \param key      key name
  \param section  section name
  \param defval   default value if key is missing

  \pre key != NULL <br>
       section != NULL

*/
bool Profile::GetBool (const char *key, const char *section, bool defval) const
{
  char buffer[80];

  assert (key);
  assert (section);

  if (!GetString (buffer, sizeof(buffer), key, section))
    return defval;
  return (!_stricmp (buffer, "on") 
       || !_stricmp (buffer, "yes") 
       || !_stricmp (buffer, "true")
       || (atoi (buffer) == 1));
}

/*!
  Boolean values are encodes as "On" "Off" strings.

  \param key      key name
  \param section  section name
  \param value    key value
  \return         true if successful, false otherwise
  \pre key != NULL <br>
       section != NULL
*/
bool Profile::PutBool (const char *key, bool value, const char *section)
{
  return PutString (key, (value)?"On":"Off", section);
}

/*!
  Copy a whole section form another INI file.
  \param  from_file   source INI file
  \param  from_sect   source section
  \param  to_sect     destination section
  \return             true if successful, false otherwise

  If \p to_sec is NULL the destination section is the same as the source section.

  The previous content of destination section is erased.

  \pre from_sect != NULL
*/
bool Profile::CopySection (const Profile& from_file, const char *from_sect, const char *to_sect)
{
  assert (from_sect);
  char buffer[INI_BUFFERSIZE];

  //trivial case: same file, same section
  if (strcmpi (filename, from_file.File ()) == 0 && (to_sect == NULL || strcmp (from_sect, to_sect) == 0))
    return true;

  //if file doesn't exist create it now
  if (!utf8::access (filename, 0))
  {
    FILE *fp = ini_openwrite (filename);
    if (fp == NULL)
      return false;
    fputs ("\xEF\xBB\xBF\r\n", fp); //write BOM mark
    fclose (fp);
  }

  if (!to_sect)
    to_sect = from_sect;

  //locate [from section]
  int len = (int)strlen (from_sect);
  FILE *fpfrom = ini_openread (from_file.filename);
  if (fpfrom == NULL)
    return false;
  fgets (buffer, INI_BUFFERSIZE, fpfrom);
  while (!feof (fpfrom))
  {
    if (buffer[0] == '[' && strncmp (&buffer[1], from_sect, len) == 0)
      break;
    fgets (buffer, INI_BUFFERSIZE, fpfrom);
  }
  if (feof (fpfrom))
  {
    fclose (fpfrom);  // from_sect not found;
    return true;
  }

  //locate [to section]
  len = (int)strlen (to_sect);
  FILE *fp = ini_openread (filename);
  if (fp == NULL)
  {
    fclose (fpfrom);
    return false;
  }
  ini_tempname (buffer, filename, sizeof (buffer));

  FILE *fpt = ini_openwrite (buffer);
  if (fpt == NULL)
  {
    fclose (fpfrom);
    fclose (fp);
    return false;
  }
  fgets (buffer, INI_BUFFERSIZE, fp);
  while (!feof (fp))
  {
    if (buffer[0] == '[' && strncmp (&buffer[1], to_sect, len) == 0)
      break;
    fputs (buffer, fpt);
    fgets (buffer, INI_BUFFERSIZE, fp);
  }

  // write [to section]
  writesection (buffer, to_sect, fpt);
  fgets (buffer, INI_BUFFERSIZE, fpfrom);
  while (!feof (fpfrom))
  {
    if (buffer[0] == '[')
      break;      //until end of from section
    fputs (buffer, fpt);
    fgets (buffer, INI_BUFFERSIZE, fpfrom);
  }
  fclose (fpfrom);

  //skip the rest of old [to section]
  fgets (buffer, INI_BUFFERSIZE, fp);
  while (!feof (fp))
  {
    if (buffer[0] == '[')
      break;
    fgets (buffer, INI_BUFFERSIZE, fp);
  }
  // write the rest to tmp
  while (!feof (fp))
  {
    fputs (buffer, fpt);
    fgets (buffer, INI_BUFFERSIZE, fp);
  }

  return close_rename (fp, fpt, filename);
}

/*!
  \param section  section name

  \pre section != NULL
*/
bool Profile::HasSection (const char *section)
{
  char buffer[256];  //Doesn't matter if buffer is small. We want to check only
              //if there are any entries
  assert (section);

  return GetKeys (buffer, sizeof(buffer), section) != 0;
}

/*!
  \param key      key name
  \param section  section name
  \return         true if successful, false otherwise

  \pre key != NULL <br>
       section != NULL
*/
bool Profile::DeleteKey( const char *key, const char *section )
{
  assert (key);
  assert (section);

  return ini_puts (key, NULL, section, filename );
}

/*!
  \param section  section name

  \pre section != NULL
*/
bool Profile::DeleteSection( const char *section )
{
  return ini_puts (NULL, NULL, section, filename);
}

/*!
  Key names are returned as null-terminated strings followed by one final null.

  \param keys     buffer for returned keys
  \param sz       size of buffer
  \param section  section name
  \return number of keys in section

  \pre keys !=NULL <br>
       section != NULL
*/
int Profile::GetKeys( char *keys, int sz, const char *section )
{
  char buffer[INI_BUFFERSIZE];

  assert (section);

  FILE* fp;
  if (buffer == NULL ||sz <= 0)
    return 0;

  if (!(fp = ini_openread(filename)))
    return 0;

  int idx = 0;
  char *p = keys;
  int blen = sz-2;

  /* Move through file 1 line at a time until the section is matched or EOF.
   */
  int slen = (int)strlen(section);
  char *sp, *ep;
  do
  {
    if (!fgets(buffer, INI_BUFFERSIZE, fp))
    {
      fclose(fp);
      return 0;
    }
    sp = skipleading(buffer);
    ep = strchr(sp, ']');
  } while (*sp != '[' || ep == NULL || (((int)(ep-sp-1) != slen || strnicmp(sp+1,section,slen) != 0)));

  /* Now that the section has been found start enumerating entries.
   * Stop when reaching section end.
   */
  int cnt = 0;
  while (fgets(buffer, INI_BUFFERSIZE,fp) && *(sp = skipleading(buffer)) != '[')
  {
    if (*sp == ';' || *sp == '#') //ignore comment lines
      continue;
    ep = strchr(sp, '=');         //Parse out the equal sign
    if (!ep)
      continue;                     //ignore malformed or blank lines
    cnt++;
    *ep = 0;
    if (blen > 1)
    {
      strncpy (p, sp, blen);
      int l = min (blen, (int)strlen(p));
      p += l;
      *p++ = 0;
      blen -= l+1;
    }
    idx++;
  }
  *p = 0;
  fclose(fp);
  return cnt;
}

/*!
  \param value    buffer for returned string
  \param len      length of buffer
  \param key      key name
  \param section  section name
  \param defval   default value
  \return         length of returned string

  \pre value != NULL <br>
       key != NULL <br>
       section != NULL <br>
       defval != NULL
*/
int Profile::GetString (char *value, int len, const char *key, const char *section, const char *defval ) const
{
  FILE *fp;
  int found = 0;

  if (!value || len <= 0 || !key)
    return 0;
  fp = ini_openread (filename);
  if (fp) 
  {
    found = getkeystring (fp, section, key, value, len);
    fclose(fp);
  }
  if (!found)
    strncpy (value, defval, len);
  return (int)strlen (value);
}

/*!
  \param value    key value
  \param key      key name
  \param section  section name
  \return         true if successful, false otherwise

  \pre value != NULL <br>
       key != NULL <br>
       section != NULL <br>
*/
bool Profile::PutString( const char *key, const char *value, const char *section )
{
  return ini_puts (key, value, section, filename);
}

/*!
  Section names are returned as null-terminated strings followed by one 
  final null.

  \param sects    buffer for returned keys
  \param sz       size of buffer
  \return number of sections found
  \pre sects !=NULL

  The number of sections returned is the total number of sections in the INI
  file even if the buffer is too small to hold sections' names.
*/

int Profile::GetSections (char *sects, int sz)
{
  assert (sects);
  assert (sz >= 0);

  char buffer [INI_BUFFERSIZE];
  FILE *fp = ini_openread (filename);
  char *p = sects;
  int cnt = 0;
  sz--; //leave space for final null
  if (fp)
  {
    char *sp, *ep;
    while (fgets(buffer, INI_BUFFERSIZE, fp))
    {
      sp = skipleading(buffer);
      if (*sp++ == '['
        && (ep = strchr(sp, ']')))
      {
        if (sz > 1)
        {
          int l = (int)min (ep-sp, sz-1);
          strncpy (p, sp, l);        
          p += l;
          sz -= l;
          *p++ = 0;
          sz--;
        }
        cnt++;
      }
    }
    fclose (fp);
  }
  else
    *p++ = 0;
  *p = 0; //terminating null
  return cnt;
}

/*
  Write a string in the INI file. This is the back engine used by all Get... functions.

  \param  key       name of key to write
  \param  value     key value
  \param  section   section name
  \param  filename  name of INI file
  \return           true if successful, false otherwise

  All parameters are UTF8 encoded.
*/
static bool ini_puts (const char *key, const char *value, const char *section, char *filename)
{
  FILE* rfp;
  FILE* wfp;

  fpos_t mark;
  char *sp, *ep;
  char buffer[INI_BUFFERSIZE];
  int len, match, flag, cachelen;

  if (!(rfp = ini_openread(filename))) 
  {
    /* If the .ini file doesn't exist, make a new file */
    if (key != NULL && value != NULL) 
    {
      if (!(wfp = ini_openwrite (filename)))
      {
        TRACE ("Cannot write to new file \"%s\"", filename);
        return false;
      }
      fputs ("\xEF\xBB\xBF\r\n", wfp); //write BOM mark
      writesection(buffer, section, wfp);
      writekey(buffer, key, value, wfp);
      fclose(wfp);
    }
    return true;
  }

  /* If parameters 'key' and 'value' are valid (so this is not an "erase" request)
   * and the setting already exists and it already has the correct value, do
   * nothing. This early bail-out avoids rewriting the INI file for no reason.
   */
  if (key != NULL && value != NULL) 
  {
    fgetpos(rfp, &mark);
    match = getkeystring(rfp, section, key, buffer, sizeof(buffer));
    if (match && strcmp (buffer, value) == 0) 
    {
      fclose(rfp);
      return true;
    }
    /* key not found, or different value -> proceed (but rewind the input file first) */
    fsetpos(rfp, &mark);
  } /* if */

  ini_tempname(buffer, filename, INI_BUFFERSIZE);
  if (!(wfp = ini_openwrite(buffer))) 
  {
    TRACE ("Cannot write to \"%s\"", filename);
    fclose (rfp);
    return false;
  }

  fgetpos(rfp, &mark);
  cachelen = 0;

  /* Move through the file one line at a time until a section is
   * matched or until EOF. Copy to temp file as it is read.
   */
  len = (section != NULL) ? (int)strlen(section) : 0;
  if (len > 0) 
  {
    do 
    {
      if (!fgets (buffer, INI_BUFFERSIZE, rfp)) 
      {
        /* Failed to find section, so add one to the end */
        flag = cache_flush(buffer, &cachelen, rfp, wfp, &mark);
        if (key!=NULL && value!=NULL) 
        {
          if (!flag)
            fputs ("\r\n", wfp);  /* force a new line behind the last line of the INI file */
          writesection(buffer, section, wfp);
          writekey(buffer, key, value, wfp);
        }
        return close_rename(rfp, wfp, filename);  /* clean up and rename */
      }
      /* Copy the line from source to dest, but not if this is the section that
       * we are looking for and this section must be removed
       */
      sp = skipleading(buffer);
      ep = strchr(sp, ']');
      match = (*sp == '[' && ep != NULL && (int)(ep-sp-1) == len && strnicmp(sp + 1,section,len) == 0);
      if (!match || key != NULL) 
      {
        if (!cache_accum(buffer, &cachelen, INI_BUFFERSIZE)) 
        {
          cache_flush(buffer, &cachelen, rfp, wfp, &mark);
          fgets (buffer, INI_BUFFERSIZE, rfp);
          cache_accum(buffer, &cachelen, INI_BUFFERSIZE);
        }
      }
    } while (!match);
  }
  cache_flush(buffer, &cachelen, rfp, wfp, &mark);
  /* when deleting a section, the section head that was just found has not been
   * copied to the output file, but because this line was not "accumulated" in
   * the cache, the position in the input file was reset to the point just
   * before the section; this must now be skipped (again)
   */
  if (key == NULL) 
  {
    fgets(buffer, INI_BUFFERSIZE, rfp);
    fgetpos (rfp, &mark);
  }

  /* Now that the section has been found, find the entry. Stop searching
   * upon leaving the section's area. Copy the file as it is read
   * and create an entry if one is not found.
   */
  len = (key!=NULL) ? (int)strlen(key) : 0;
  for( ;; ) 
  {
    if (!fgets(buffer, INI_BUFFERSIZE, rfp)) 
    {
      /* EOF without an entry so make one */
      flag = cache_flush(buffer, &cachelen, rfp, wfp, &mark);
      if (key!=NULL && value!=NULL) 
      {
        if (!flag)
          fputs("\r\n", wfp);  /* force a new line behind the last line of the INI file */
        writekey(buffer, key, value, wfp);
      }
      return close_rename(rfp, wfp, filename);  /* clean up and rename */
    }
    sp = skipleading(buffer);
    ep = strchr(sp, '='); /* Parse out the equal sign */
    if (ep == NULL)
      ep = strchr(sp, ':');
    match = (ep != NULL && (int)(skiptrailing(ep,sp)-sp) == len && strnicmp(sp,key,len) == 0);
    if ((key != NULL && match) || *sp == '[')
      break;  /* found the key, or found a new section */
    /* copy other keys in the section */
    if (key == NULL) 
    {
      fgetpos (rfp, &mark);  /* we are deleting the entire section, so update the read position */
    } 
    else 
    {
      if (!cache_accum(buffer, &cachelen, INI_BUFFERSIZE)) 
      {
        cache_flush(buffer, &cachelen, rfp, wfp, &mark);
        fgets(buffer, INI_BUFFERSIZE, rfp);
        cache_accum(buffer, &cachelen, INI_BUFFERSIZE);
      }
    }
  }
  /* the key was found, or we just dropped on the next section (meaning that it
   * wasn't found); in both cases we need to write the key, but in the latter
   * case, we also need to write the line starting the new section after writing
   * the key
   */
  flag = (*sp == '[');
  cache_flush(buffer, &cachelen, rfp, wfp, &mark);
  if (key != NULL && value != NULL)
    writekey(buffer, key, value, wfp);
  /* cache_flush() reset the "read pointer" to the start of the line with the
   * previous key or the new section; read it again (because writekey() destroyed
   * the buffer)
   */
  fgets (buffer, INI_BUFFERSIZE, rfp);
  if (flag) 
  {
    /* the new section heading needs to be copied to the output file */
    cache_accum(buffer, &cachelen, INI_BUFFERSIZE);
  } 
  else 
  {
    /* forget the old key line */
    fgetpos (rfp, &mark);
  }
  /* Copy the rest of the INI file */
  while (fgets (buffer, INI_BUFFERSIZE, rfp)) 
  {
    if (!cache_accum(buffer, &cachelen, INI_BUFFERSIZE)) 
    {
      cache_flush(buffer, &cachelen, rfp, wfp, &mark);
      fgets (buffer, INI_BUFFERSIZE, rfp);
      cache_accum(buffer, &cachelen, INI_BUFFERSIZE);
    }
  }
  cache_flush(buffer, &cachelen, rfp, wfp, &mark);
  return close_rename(rfp, wfp, filename);  /* clean up and rename */
}

/*!
  Read a key string.
  \param  fp      input file
  \param  section section name
  \param  key     input key
  \param  val     key value buffer
  \param  size    length of value buffer

  \return 1 if key was found; 0 otherwise
*/
static int getkeystring(FILE *fp, const char *section, const char *key, char *val, int size)
{
  char *sp, *ep;
  int len;
  char buffer[INI_BUFFERSIZE];
  assert (fp != NULL);

  
  /* Move through file 1 line at a time until the section is matched or EOF.
   */
  len = (int)strlen(section);
  do 
  {
    if (!fgets(buffer, INI_BUFFERSIZE, fp))
      return 0;
    sp = skipleading(buffer);
    ep = strchr(sp, ']');
  } while (*sp != '[' || ep == NULL || (((int)(ep-sp-1) != len || strnicmp(sp+1,section,len) != 0)));

  /* Now that the section has been found, find the entry.
   * Stop searching upon leaving the section's area.
   */
  assert(key != NULL);
  len = (int)strlen(key);
  bool found = false;
  do 
  {
    if (!fgets(buffer, INI_BUFFERSIZE,fp) || *(sp = skipleading(buffer)) == '[')
      return 0;
    sp = skipleading(buffer);
    ep = strchr(sp, '='); /* Parse out the equal sign */
    if (ep == NULL)
      ep = strchr(sp, ':');
    if (!ep)
      continue;

    found = (skiptrailing(ep,sp) - sp) == len && !strnicmp(sp,key,len);
  } while (*sp == ';' || *sp == '#' || ep == NULL || !found);

  if (found) 
  {
    /* Copy up to 'size' chars to buffer */
    assert(ep != NULL);
    assert(*ep == '=' || *ep == ':');
    sp = skipleading(ep + 1);
    sp = cleanstring(sp);  /* Remove a trailing comment */
    strncpy(val, sp, size);
    return 1;
  }
  return 0;   /* key not found*/

}

static char* skipleading (char *str)
{
  assert(str != NULL);
  while (*str > 0 && *str <= ' ')
    str++;
  return str;
}

static char* skiptrailing (char* str, char* base)
{
  assert(str != NULL);
  assert(base != NULL);
  while (str > base && *(str-1) > 0 && *(str-1) <= ' ')
    str--;
  return str;
}

static char *striptrailing(char *str)
{
  char *ptr = skiptrailing(strchr(str, '\0'), str);
  assert(ptr != NULL);
  *ptr = '\0';
  return str;
}

static char* cleanstring (char* string)
{
  bool isstring;
  char* ep;

  assert(string != NULL);

  /* Remove a trailing comment */
  isstring = false;
  for (ep = string; *ep != '\0' && ((*ep != ';' && *ep != '#') || isstring); ep++) 
  {
    if (*ep == '"') 
    {
      if (*(ep + 1) == '"')
        ep++;                 /* skip "" (both quotes) */
      else
        isstring = !isstring; /* single quote, toggle isstring */
    } 
    else if (*ep == '\\' && *(ep + 1) == '"') 
      ep++;                   /* skip \" (both quotes */
  }
  assert(ep != NULL && (*ep == '\0' || *ep == ';' || *ep == '#'));
  *ep = '\0';                 /* terminate at a comment */
  striptrailing(string);
  return string;
}


/* 
  Get a temporary file name to copy to. Use the existing name, but with
  the last character set to a '~'.
*/
static void ini_tempname (char* dest, const char* source, int maxlength)
{
  char *p;

  strncpy(dest, source, maxlength);
  p = dest + strlen(dest);
  *(p - 1) = '~';
}

static void writesection (char* buffer, const char* section, FILE *fp)
{
  if (section != NULL && strlen (section) > 0) 
  {
    buffer[0] = '[';
    strncpy (buffer + 1, section, INI_BUFFERSIZE - 4);  /* -1 for '[', -1 for ']', -2 for '\r\n' */
    strcat (buffer, "]\r\n");
    fputs (buffer, fp);
  }
}

static void writekey (char* buffer, const char* key, const char* value, FILE *fp)
{
  char *p;
  p = strncpy(buffer, key, INI_BUFFERSIZE - 3);  /* -1 for '=', -2 for '\r\n' */
  p += strlen(p);
  *p++ = '=';
  strncpy(p, value, INI_BUFFERSIZE - (p - buffer) - 2); /* -2 for '\r\n' */
  p += strlen(p);
  strcpy (p, "\r\n"); /* copy line terminator*/
  fputs (buffer, fp);
}

static int cache_accum (const char* string, int* size, int max)
{
  int len = (int)strlen(string);
  if (*size + len >= max)
    return 0;
  *size += len;
  return 1;
}

static int cache_flush (char* buffer, int* size, FILE* rfp, FILE* wfp, fpos_t* mark)
{
  int pos = 0;
  assert (rfp);
  assert (wfp);

  fsetpos (rfp, mark);
  assert (buffer != NULL);
  buffer[0] = '\0';
  assert (size != NULL);
  while (pos < *size) 
  {
    fgets (buffer + pos, INI_BUFFERSIZE - pos, rfp);
    pos += (int)strlen (buffer + pos);
    assert(pos <= *size);
  }
  if (buffer[0] != '\0')
    fputs (buffer, wfp);
  fgetpos (rfp, mark);  /* update mark */
  *size = 0;
  /* return whether the buffer ended with a line termination */
  return (buffer[pos-1] == '\n');
}

/*!
  Close input and output files and renames the output (temporary) file to
  input file name. Previous input file is deleted.

  Had an issue with Kaspersky anti-virus that seems to keep opened the old input file
  making the rename operation to fail. Solved by adding a few retries before 
  failing.
*/
static bool close_rename(FILE* rfp, FILE* wfp, const char* filename)
{
  const int RETRIES = 50;
  int i;
  char tmpname[_MAX_PATH];
  fclose(rfp);
  fclose(wfp);
  ini_tempname(tmpname, filename, sizeof(tmpname));

  //retry a few times to allow for stupid anti-virus
  i = 0;
  while (i++ < RETRIES && utf8::remove(filename))
    Sleep(0);

  if (i >= RETRIES)
    return false;

  i = 0;
  while (i++ < RETRIES && !utf8::rename(tmpname, filename))
    Sleep(0);
  return (i < RETRIES);
}

#ifdef MLIBSPACE
}
#endif

