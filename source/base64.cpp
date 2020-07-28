/*!
  \file base64.cpp Implementation of Base64 encoding/decoding functions

  (c) Mircea Neacsu 2017
*/
#include <mlib/base64.h>

namespace mlib {

using namespace std;

static char enctab[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static char dectab[256];
bool dectab_initialized = false;


/*!
  Decode a Base64 string
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

  int k = 0, no = (int)olen;
  union {
    unsigned char sixbit[4];
    int si;
  };
  si = 0;
  while (ilen)
  {
    int decval = dectab[*in++];
    if (decval)
      sixbit[k++] = decval;
    if (k == 4)
    {
      *oc++ = sixbit[0] << 2 | sixbit[1] >> 4;
      *oc++ = sixbit[1] << 4 | sixbit[2] >> 2;
      *oc++ = sixbit[2] << 6 | sixbit[3];
      k = 0;
      no -= 3;
      si = 0;
    }
    ilen--;
  }

  if (no)
  {
    *oc++ = sixbit[0] << 2 | sixbit[1] >> 4;
    no--;
  }
  if (no)
  {
    *oc++ = sixbit[1] << 4 | sixbit[2] >> 2;
    no--;
  }
  if (no)
    *oc++ = sixbit[2] << 6 | sixbit[3];

  return olen;
}

/*! 
  String-aware overload
  \param in     input string
  \return       decoded string
*/
string base64dec (const string& in)
{
  size_t sz = base64dec (in.c_str(), nullptr);
  std::string out (sz, '\0');
  base64dec (in.c_str(), &out[0]);
  return out;
}

/*!
  Encode input bytes to Base64 producing a null-terminated string
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
  if (!out)
    return olen;

  union {
    unsigned char sixbit[4];
    int si;
  };
  const unsigned char *cin = (const unsigned char *)in;
  int k;

  k = si = 0;
  while (ilen--)
  {
    sixbit[k++] = *cin++;
    if (k == 3)
    {
      *out++ = enctab[sixbit[0] >> 2];
      *out++ = enctab[((sixbit[0] & 3) << 4) | (sixbit[1] >> 4)];
      *out++ = enctab[((sixbit[1] & 0x0f) << 2) | (sixbit[2] >> 6)];
      *out++ = enctab[sixbit[2] & 0x3f];
      k = si = 0;
    }
  }
  if (k)
  {
    *out++ = enctab[sixbit[0] >> 2];
    *out++ = enctab[((sixbit[0] & 3) << 4) | (sixbit[1] >> 4)];
    *out++ = (k >= 2) ? enctab[((sixbit[1] & 0x0f) << 2) | (sixbit[2] >> 6)] : '=';
    *out++ = (k == 3) ? enctab[sixbit[2] & 0x3f] : '=';
  }
  *out++ = 0; //terminating null
  return olen;
}

/*! 
  String-aware overload
  \param in     input string
  \return       decoded string
*/
string base64enc (const string& in)
{
  size_t sz = base64enc (in.data(), nullptr, in.size());
  std::string out (sz, '\0');
  base64enc (in.data(), &out[0], in.size());
  out.pop_back (); //get rid of terminating NULL
  return out;
}

}
