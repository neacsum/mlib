#include <utpp/utpp.h>
#include <mlib/bitstream.h>
#include <sstream>

SUITE (bitstreams)
{
  struct stream_test
  {
    std::stringstream ss;
  };

  TEST_FIXTURE (stream_test, four_bits_stream)
  {
    mlib::bitstream bs{ ss, 4 };
    bs.mwrite (3, 4);
    bs.mwrite (4, 3);
    bs.flush ();
    ss.seekg (0, std::ios::beg);
    int v = bs.mread (4);
    CHECK_EQUAL (3, v);
    v = bs.mread (4);
    CHECK_EQUAL (4, v);
  }

  TEST_FIXTURE (stream_test, eight_bit_stream)
  {
    mlib::bitstream bs{ ss, 8 };
    //write some random bits
    bs.mwrite (3, 2);
    bs.mwrite (4, 3);
    bs.write (1);
    bs.write (0);
    bs.write (1);
    bs.mwrite ('A', 8);
    bs.flush ();
    CHECK_EQUAL (2, ss.str ().length ()); //we wrote 2 bytes
    ss.seekg (0, std::ios::beg);
    int v = bs.mread (8);
    CHECK_EQUAL (0b11100101, v);  //first byte
    CHECK_EQUAL ('A', bs.mread (8));
  }

  class NMEAstream : public mlib::bitstream
  {
  public:
    NMEAstream (std::iostream& is) : bitstream (is, 6) {};

  protected:
    void decode (unsigned char &bits, char chr);
    void encode (unsigned char bits, char& chr);
  };

  void NMEAstream::decode (unsigned char &bits, char chr)
  {
    chr += 0x28;
    if (chr < 0)
      chr += 0x20;
    else
      chr += 0x28;
    bits = (unsigned char)(chr & 0x3f);
  }

  void NMEAstream::encode (unsigned char bits, char& chr)
  {
    static const char tbl[] =
      "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVW'abcdefghijklmnopqrstuvw";
    bits &= 0x3f;
    chr = tbl[bits];
  }

  //Decoding sample string from NMEA standard (page 83)
  TEST_FIXTURE (stream_test, NMEA_dec)
  {
    NMEAstream ns (ss);
    ss.str ("1P000Oh1IT1svTP2r:43grwb0Eq4");
    CHECK_EQUAL (0b000001, ns.mread (6));
    CHECK_EQUAL (true, ns.read ());
    CHECK_EQUAL (false, ns.read ());
    CHECK_EQUAL (0b000000000000000000000001111111, ns.mread (30));
    CHECK_EQUAL (0, ns.mread (4));
    CHECK_EQUAL (0b00000101, ns.mread (8));
    CHECK_EQUAL (0b1001100100, ns.mread (10));
    CHECK_EQUAL (false, ns.read ());
    CHECK_EQUAL (0b0000111101111111010010010000, ns.mread (28));
    CHECK_EQUAL (0b000001011101000101000010000, ns.mread (27));
    CHECK_EQUAL (0b001110111111, ns.mread (12));
    CHECK_EQUAL (0b101011111, ns.mread (9));
    CHECK_EQUAL (0b110101, ns.mread (6));
    CHECK_EQUAL (0, ns.mread (2));
    CHECK_EQUAL (0, ns.mread (5));
    CHECK_EQUAL (1, ns.mread (2));
    CHECK_EQUAL (1, ns.mread (2));
    CHECK_EQUAL (0b01111001000100, ns.mread (14));
  }

  //same test in opposite direction
  TEST_FIXTURE (stream_test, NMEA_enc)
  {
    NMEAstream ns (ss);
    ns.mwrite (0b000001, 6);
    ns.write (true);
    ns.write (false);
    ns.mwrite (0b000000000000000000000001111111, 30);
    ns.mwrite (0, 4);
    ns.mwrite (0b00000101, 8);
    ns.mwrite (0b1001100100, 10);
    ns.write (false);
    ns.mwrite (0b0000111101111111010010010000, 28);
    ns.mwrite (0b000001011101000101000010000, 27);
    ns.mwrite (0b001110111111, 12);
    ns.mwrite (0b101011111, 9);
    ns.mwrite (0b110101, 6);
    ns.mwrite (0, 2);
    ns.mwrite (0, 5);
    ns.mwrite (1, 2);
    ns.mwrite (1, 2);
    ns.mwrite (0b01111001000100, 14);
    ns.flush ();
    CHECK_EQUAL ("1P000Oh1IT1svTP2r:43grwb0Eq4", ss.str ());
  }
}
