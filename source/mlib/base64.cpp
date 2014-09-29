#include <mlib/base64.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

static char enctab[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static char dectab[256];
bool dectab_initialized = false;


/*!
  Decode a base-64 string
  \param in     input string
  \param out    output decoded bytes
  \return       size of decoded memory area

  If output pointer is NULL the function only returns the required size of output block.
*/
size_t base64dec (const char *in, void *out)
{
  //check if input length is valid
  size_t ilen = strlen (in);
  if (ilen % 4 != 0)
    return 0;

  //find output length
  size_t olen = ilen / 4 * 3;
  if (in[ilen - 1] == '=')
  {
    olen--;
    ilen--;
  }
  if (in[ilen - 1] == '=')
  {
    olen--;
    ilen--;
  }

  if (!out)
    return olen;

  unsigned char *oc = (unsigned char *)out;
  if (!dectab_initialized)
  {
    memset (dectab, 0, 256);
    for (int i = 0; i < 64; i++)
      dectab[enctab[i]] = i;
    dectab_initialized = true;
  }

  int k = 0, no = olen;
  union {
    unsigned char sextet[4];
    int si;
  };
  si = 0;
  while (ilen)
  {
    int decval = dectab[*in++];
    if (decval)
      sextet[k++] = decval;
    if (k == 4)
    {
      *oc++ = sextet[0] << 2 | sextet[1] >> 4;
      *oc++ = sextet[1] << 4 | sextet[2] >> 2;
      *oc++ = sextet[2] << 6 | sextet[3];
      k = 0;
      no -= 3;
      si = 0;
    }
    ilen--;
  }

  if (no)
  {
    *oc++ = sextet[0] << 2 | sextet[1] >> 4;
    no--;
  }
  if (no)
  {
    *oc++ = sextet[1] << 4 | sextet[2] >> 2;
    no--;
  }
  if (no)
    *oc++ = sextet[2] << 6 | sextet[3];

  return olen;
}

/*!
  Encode input bytes to base 64 producing a null-terminated string
  \param in     input bytes
  \param out    output encoded string
  \param ilen   number of bytes to encode
  \return       length of encoded string including terminating null character

  If output pointer is NULL the function only returns the length of resulting
  string.
 */
size_t base64enc (const void* in, char* out, size_t ilen)
{
  size_t olen = 4*((ilen+2)/3)+1;
  union {
    unsigned char sextet[4];
    int si;
  };
  const unsigned char *cin = (const unsigned char *)in;
  if (!out)
    return olen;

  int k = 0;
  si = 0;
  while (ilen)
  {
    sextet[k++] = *cin++;
    ilen--;
    if (k == 3 || !ilen)
    {
      *out++ = enctab[sextet[0] >> 2];
      *out++ = enctab[((sextet[0] & 3) << 4) | (sextet[1] >> 4)];
      *out++ = (k >= 2) ? enctab[((sextet[1] & 0x0f) << 2) | (sextet[2] >> 6)] : '=';
      *out++ = (k == 3) ? enctab[sextet[2] & 0x3f] : '=';
      k = 0;
      si = 0;
    }
  }
  *out++ = 0; //terminating null
  return olen;
}

#ifdef MLIBSPACE
};
#endif
