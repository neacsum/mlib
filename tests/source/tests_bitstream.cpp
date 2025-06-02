#include <utpp/utpp.h>
#include <mlib/mlib.h>
#pragma hdrstop
#include <sstream>

using namespace mlib;

SUITE (bitstreams)
{
  struct stream_test
  {
    std::stringstream ss;
  };

  TEST_FIXTURE (stream_test, four_bits_stream)
  {
    bitstream bs{ ss, 4 };
    bs.write (3, 4);
    bs.write (4, 3);
    bs.flush ();
    ss.seekg (0, std::ios::beg);
    int v = bs.read (4);
    CHECK_EQUAL (3, v);
    v = bs.read (4);
    CHECK_EQUAL (4, v);
  }

  TEST_FIXTURE (stream_test, eight_bit_stream)
  {
    bitstream bs{ ss, 8 };
    //write some random bits
    bs.write (3, 2);
    bs.write (4, 3);
    bs.write (1);
    bs.write (0);
    bs.write (1);
    bs.write ('A', 8);
    bs.flush ();
    CHECK_EQUAL (2, ss.str ().length ()); //we wrote 2 bytes
    ss.seekg (0, std::ios::beg);
    int v = bs.read (8);
    CHECK_EQUAL (0b11100101, v);  //first byte
    CHECK_EQUAL ('A', bs.read (8));
  }

  class NMEAstream : public bitstream
  {
  public:
    NMEAstream (std::iostream& is) : bitstream (is, 6) {};

  protected:
    unsigned char decode (char chr) override;
    char encode (unsigned char bits);
  };

  unsigned char NMEAstream::decode (char chr)
  {
    chr += 0x28;
    if (chr < 0)
      chr += 0x20;
    else
      chr += 0x28;
    return (unsigned char)(chr & 0x3f);
  }

  char NMEAstream::encode (unsigned char bits)
  {
    static const char tbl[] =
      "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVW'abcdefghijklmnopqrstuvw";
    bits &= 0x3f;
    return tbl[bits];
  }

  //Decoding sample string from NMEA standard (page 83)
  TEST_FIXTURE (stream_test, NMEA_dec)
  {
    NMEAstream ns (ss);
    ss.str ("1P000Oh1IT1svTP2r:43grwb0Eq4");
    CHECK_EQUAL (0b000001, ns.read (6));
    CHECK_EQUAL (true, ns.read ());
    CHECK_EQUAL (false, ns.read ());
    CHECK_EQUAL (0b000000000000000000000001111111, ns.read (30));
    CHECK_EQUAL (0, ns.read (4));
    CHECK_EQUAL (0b00000101, ns.read (8));
    CHECK_EQUAL (0b1001100100, ns.read (10));
    CHECK_EQUAL (false, ns.read ());
    CHECK_EQUAL (0b0000111101111111010010010000, ns.read (28));
    CHECK_EQUAL (0b000001011101000101000010000, ns.read (27));
    CHECK_EQUAL (0b001110111111, ns.read (12));
    CHECK_EQUAL (0b101011111, ns.read (9));
    CHECK_EQUAL (0b110101, ns.read (6));
    CHECK_EQUAL (0, ns.read (2));
    CHECK_EQUAL (0, ns.read (5));
    CHECK_EQUAL (1, ns.read (2));
    CHECK_EQUAL (1, ns.read (2));
    CHECK_EQUAL (0b01111001000100, ns.read (14));
  }

  //same test in opposite direction
  TEST_FIXTURE (stream_test, NMEA_enc)
  {
    NMEAstream ns (ss);
    ns.write (0b000001, 6);
    ns.write (true);
    ns.write (false);
    ns.write (0b000000000000000000000001111111, 30);
    ns.write (0, 4);
    ns.write (0b00000101, 8);
    ns.write (0b1001100100, 10);
    ns.write (false);
    ns.write (0b0000111101111111010010010000, 28);
    ns.write (0b000001011101000101000010000, 27);
    ns.write (0b001110111111, 12);
    ns.write (0b101011111, 9);
    ns.write (0b110101, 6);
    ns.write (0, 2);
    ns.write (0, 5);
    ns.write (1, 2);
    ns.write (1, 2);
    ns.write (0b01111001000100, 14);
    ns.flush ();
    CHECK_EQUAL ("1P000Oh1IT1svTP2r:43grwb0Eq4", ss.str ());
  }
}
