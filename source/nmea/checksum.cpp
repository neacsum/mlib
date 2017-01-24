/**
  Compute the checksum of a NMEA sentence

  \return
  - false = if an incorrect checksum was found or the sentence doesn't start with '$' or '!' characters.
  - true  = if the checksum is correct or inexistent.
*/

bool checksum  (const char *buf)
{
  char cks;
  char hex_cks[2];
  static const char hex_digits[] = "0123456789ABCDEF";

  if( *buf !='$' && *buf != '!')
    return false;
  buf++;
  cks = 0;
  while ( *buf && *buf != '*' && *buf != 0x0d)
  {
    cks ^= *buf++;
  }
  if ( *buf == 0x0d )
    return true;            /* No checksum in sentence */
  else if ( *buf != '*' )
    return false;           /* Neither <CR> nor checksum field */
  else
  {
    buf++;
    hex_cks[0] = hex_digits[cks>>4];
    hex_cks[1] = hex_digits[cks & 0x0f];
    return (*buf++ == hex_cks[0] && *buf == hex_cks[1]);
  }
}

