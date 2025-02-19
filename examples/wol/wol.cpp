#include <mlib/mlib.h>

/*
  WOL - Wake-on-LAN utility

  The program sends a "magic packet" to a network device that supports
  WOL functionality. The magic packet consists of 6 bytes of 0xFF followed by
  the MAC address of the destination device, repeated 16 times.

  For more information see: https://en.wikipedia.org/wiki/Wake-on-LAN

  Usage:
    wol [-h|--help] [-d <host_ip>] <mac_address>

  The MAC address can be in format xx:xx:xx:xx:xx:xx or xx-xx-xx-xx-xx-xx
*/

using mlib::OptParser;

 OptParser cmds{
   "d: host \t destination or broadcast IP address", 
   "h|help \t show help message"
 };

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

void help ()
{
  std::cout 
    << "Wake-on-LAN utility. This program sends a magic packet to a "
       "destination device that supports WOL.\n"
       "Usage: \n"
    << '\t' << cmds.appname () << " [options] <mac_address> \n\n"
    << "Valid options are:\n"
    << "\t" << cmds.synopsis () << "\n\n"
    << "Where:\n"
    << cmds.description () << '\n'
    << "<mac_address> can be 'xx:xx:xx:xx:xx:xx' or 'xx-xx-xx-xx-xx-xx'\n";
}


int main (int argc, char** argv)
{
  //parse command line
  int argmac;
  auto ret = cmds.parse (argc, argv, &argmac);
  if (argc < 2 || cmds.hasopt('h'))
  {
    help ();
    exit (0);
  }

  if (ret != 0 || argmac != argc - 1)
  {
    std::cout << "Syntax error. Valid options are:\n" << cmds.synopsis () << '\n';
    exit (1);
  }

  //parse MAC address
  unsigned char mac[6];
  if (!parse_mac (argv[argmac], mac))
  {
    std::cerr << cmds.appname() << " -- Invalid MAC address: " << argv[argmac] << std::endl;
    exit (1);
  }

  const mlib::sock s (SOCK_DGRAM);

  // set destination or broadcast address
  mlib::inaddr destination;
  DWORD sz;
  WSAIoctl (s, SIO_GET_BROADCAST_ADDRESS, NULL, 0, &destination, sizeof (destination), &sz, NULL, NULL);
  destination.port (9);

  if (cmds.hasopt ('d'))
  {
    std::string dest_str;
    cmds.getopt ('d', dest_str);
    if (destination.host (dest_str) != mlib::erc::success)
    {
      std::cerr << cmds.appname () << " -- Invalid destination address: " << dest_str << std::endl;
      exit (1);
    }
  }
  //create magic packet
  unsigned char magic[102];
  int i = 0;
  while (i < 6)
    magic[i++] = 0xff;
  
  while (i < sizeof(magic))
  {
    memcpy (&magic[i], mac, sizeof (mac));
    i += sizeof (mac);
  }

  //send magic packet
  s.broadcast (true);
  std::cout << "Sending magic packet to " << destination << std::endl;
  s.sendto (destination, magic, sizeof (magic));

  return 0;
}