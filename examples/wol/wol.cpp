#include <mlib/mlib.h>

/*
  WOL - Wake-on-LAN utility

  The program sends a "magic packet" to a network device that supports
  WOL functionality. The magic packet consists of 6 bytes of 0xFF followed by
  the MAC address of the destination device, repeated 16 times.

  For more information see: https://en.wikipedia.org/wiki/Wake-on-LAN

  Usage:
    wol <mac_address>

  The MAC address can be in format xx:xx:xx:xx:xx:xx or xx-xx-xx-xx-xx-xx
*/

// parse MAC address
bool parse_mac (const char* str, unsigned char* mac)
{
  int len = (int)strlen (str);
  int i = 0;
  while (len)
  {
    if (!mlib::hexbyte (mac[i++], str))
      return false;
    if (i == 6)
      break;
    len -= 2;
    str += 2;
    if (*str != ':' && *str != '-')
      return false;
    else
      str++;
  }
  return (i == 6);
}

int main (int argc, char** argv)
{
  auto app = std::filesystem::path (argv[0]).stem ().string ();
  if (argc < 2)
  {
    std::cout << "Wake-on-LAN utility. This program sends a magic packet to a "
      "destination device that support WOL.\n"
      "Usage: \n" 
      << '\t' << app  << " <mac_address> \n\n"
      << "where <mac_address> can be 'xx:xx:xx:xx:xx:xx' or 'xx-xx-xx-xx-xx-xx'\n";
    exit (0);
  }

  unsigned char mac[6];

  if (!parse_mac (argv[1], mac))
  {
    std::cerr << app << " -- Invalid MAC address: " << argv[1] << std::endl;
    exit (1);
  }

  //create magic packet
  unsigned char magic[102];
  int i = 0;
  while (i < 6)
    magic[i++] = 0xff;
  
  while (i < 102)
  {
    memcpy (&magic[i], mac, sizeof (mac));
    i += sizeof (mac);
  }

  //send magic packet
  mlib::sock s (SOCK_DGRAM);
  s.broadcast (true);
  s.sendto (mlib::inaddr (INADDR_BROADCAST, 9), magic, sizeof (magic));

  return 0;
}