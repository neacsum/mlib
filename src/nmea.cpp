/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <string.h>
#include <stdlib.h>

namespace mlib::nmea {

// some handy parsing macros
#define NEXT_TOKEN(A, B)                                                                           \
  if ((A = ctx.token ()) == NULL)                                                                  \
  return B
#define NEXT_VALIDTOKEN(A, B)                                                                      \
  if ((A = ctx.token ()) == NULL || !strlen (A))                                                   \
  return B
#define IFPAR(par, exp)                                                                            \
  if (par)                                                                                         \
  *par = (exp)


/*! \addtogroup NMEA-0183
 @{
*/

// parsing context for NMEA-0183 parser
class parse_context
{
public:
  parse_context (const char* buf);
  ~parse_context ();
  char* token ();

private:
  char* lcl;     // local copy of string to parse
  char* toparse; // parsing position in local string
  char saved;    // char replaced with NULL
};

/// Constructor for a parsing context
inline parse_context::parse_context (const char* buf)
{
  toparse = lcl = strdup (buf);
  saved = 0;
};

inline parse_context::~parse_context ()
{
  free (lcl);
};

/*
    Return next token of a NMEA sentence.

    Tokens are delimited by ',', \<CR\>, '*' or end of string.
    We cannot use strtok directly because we want to leave the string unchanged
    and we don't want to skip over consecutive empty fields.

*/
char* parse_context::token ()
{
  char* ret;
  if (saved)
  {
    *toparse = saved;
    if (saved != ',')
      return NULL;
    toparse++;
  }
  ret = toparse;
  while (*toparse && *toparse != ',' && *toparse != '\r' && *toparse != '*')
    toparse++;

  saved = *toparse;
  *toparse = 0;
  return ret;
}

/*!
  Compute the checksum of a NMEA sentence

  \return
  - false = if an incorrect checksum was found or the sentence doesn't start with '$' or '!'
  characters.
  - true  = if the checksum is correct or inexistent.
*/

bool checksum (const char* buf)
{
  char cks;
  char hex_cks[2];
  static const char hex_digits[] = "0123456789ABCDEF";

  if (*buf != '$' && *buf != '!')
    return false;
  buf++;
  cks = 0;
  while (*buf && *buf != '*' && *buf != 0x0d)
  {
    cks ^= *buf++;
  }
  if (*buf == 0x0d)
    return true; /* No checksum in sentence */
  else if (*buf != '*')
    return false; /* Neither <CR> nor checksum field */
  else
  {
    buf++;
    hex_cks[0] = hex_digits[cks >> 4];
    hex_cks[1] = hex_digits[cks & 0x0f];
    return (*buf++ == hex_cks[0] && *buf == hex_cks[1]);
  }
}

/*!
  NMEA-0183 DBS sentence.

  $ttDBS,depf,f,depm,M,depF,F

  \note
    If more than one depth is specified depth in meters takes precedence
    over depth in feet and depth in feet takes precedence over depth in
    fathoms.

    Returned value is always in meters.
*/
int dbs (const char* buf, double* depth)
{
  double feet = 0.0, meters = 0.0, fathoms = 0.0;
  bool ft, mt, fh;
  ft = mt = fh = false;
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "DBS", 3))
    return 0;
  NEXT_TOKEN (tok, 0);
  if (*tok)
  {
    feet = atof (tok);
    ft = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'f')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (*tok)
  {
    meters = atof (tok);
    mt = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'M')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (*tok)
  {
    fathoms = atof (tok);
    fh = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'F')
    return 0;

  if (depth)
  {
    if (mt)
      *depth = meters;
    else if (ft)
      *depth = feet * 0.3048;
    else if (fh)
      *depth = fathoms * 1.8288;
    else
      *depth = 0.;
  }
  return 3;
}

/*!
  NMEA-0183 DBT sentence

  $ttDBT,depf,f,depm,M,depF,F

  \note
    If more than one depth is specified, depth in meters takes precedence
    over depth in feet and depth in feet takes precedence over depth in
    fathoms.

    Returned value is always in meters.

    This sentence is obsolete.
*/
int dbt (const char* buf, double* depth)
{
  double feet = 0.0, meters = 0.0, fathoms = 0.0;
  bool ft, mt, fh;
  ft = mt = fh = false;
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "DBT", 3))
    return 0;
  NEXT_TOKEN (tok, 0);
  if (*tok)
  {
    feet = atof (tok);
    ft = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'f')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (*tok)
  {
    meters = atof (tok);
    mt = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'M')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (*tok)
  {
    fathoms = atof (tok);
    fh = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'F')
    return 0;

  if (depth)
  {
    if (mt)
      *depth = meters;
    else if (ft)
      *depth = feet * 0.3048;
    else if (fh)
      *depth = fathoms * 1.8288;
    else
      *depth = 0.;
  }
  return 2;
}

/*!
  NMEA-0183 DPT sentence.

  $ttDPT,dep,offset,range
*/
int dpt (const char* buf, double* depth, double* offset, double* range)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "DPT", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (depth, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (offset, atof (tok));
  NEXT_TOKEN (tok, 2);
  IFPAR (range, atof (tok));
  return 3;
}

/**
  NMEA-0183 GGA sentence.

  $ttGGA,hhmmss,xxxx.xx,N,xxxxx.xx,W,q,s,dop,msl,M,und,M[,age,station]

  \return
    version of standard to which sentence conforms or 0 if invalid statement.
  \note
    Height is ellipsoidal.
*/
int gga (const char* buf, double* lat, double* lon, double* time, double* height, double* undul,
         double* dop, int* sat, int* mode, double* age, int* station)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "GGA", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S')
    *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W')
    *lon *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (mode, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (sat, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (dop, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (height, atof (tok));
  NEXT_TOKEN (tok, 0); //'M'
  NEXT_TOKEN (tok, 0);
  double val = atof (tok);
  IFPAR (undul, val);
  if (height)
    *height += val;
  NEXT_TOKEN (tok, 0); //'M'
  NEXT_TOKEN (tok, 2);
  IFPAR (age, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (station, atoi (tok));
  return 3;
}

/*!
  NMEA-0183 GGK sentence.

  $ttGGK,hhmmss,mmddyy,xxxx.xx,N,xxxxx.xx,W,q,s,dop,geo,M

  \return
    - 1 if valid statement
    - 0 if invalid statement

  \note
    This is not a true NMEA standard sentence, however it is generated by quite a few
    GPS units.
*/
int ggk (const char* buf, double* lat, double* lon, double* time, double* height, double* dop,
         int* sat, int* mode)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "GGK", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0); // date
  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S')
    *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W')
    *lon *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (mode, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (sat, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (dop, atof (tok));
  NEXT_TOKEN (tok, 1); // height is optional
  if (height)
  {
    // POS MV sends ellipsoidal height prefixed with "EHT"
    if (!strncmp (tok, "EHT", 3))
      tok += 3;
    *height = atof (tok);
  }
  return 1;
}

/*!
  NMEA-0183 GLL sentence.

  $ttGLL,lat,N,lon,W[,time,valid,[mode]]

  \return
    - 0 if invalid statement
    - 1 old NMEA 1.x standard (missing time and mode)
    - 2 newer version 2 (missing mode field)
    - 3 version 3
*/
int gll (const char* buf, double* lat, double* lon, double* time, int* mode)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "GLL", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S')
    *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W')
    *lon *= -1.;
  NEXT_TOKEN (tok, 1);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  NEXT_TOKEN (tok, 2);
  if (mode)
  {
    if (*tok == 'A')
      *mode = 1;
    else if (*tok == 'D')
      *mode = 2;
    else
      *mode = 0;
  }
  return 3;
}

/**
  NMEA-0183 GNS sentence.

  $--GNS,hhmmss.ss,llll.lll,a,yyyyy.yyy,a,c--c,xx,x.x,x.x,x.x,x.x,x.x*hh

  \return
    version of standard to which sentence conforms or 0 if invalid statement.
  \note
    Height is ellipsoidal.
*/
int gns (const char* buf, double* time, double* lat, double* lon, int* mode, int* sat, double* dop,
         double* height, double* age, int* station)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "GNS", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S')
    *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W')
    *lon *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (mode, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (sat, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (dop, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (height, atof (tok));
  NEXT_TOKEN (tok, 0); //'M'
  NEXT_TOKEN (tok, 0);
  if (height)
  {
    double undul = atof (tok);
    *height += undul;
  }
  NEXT_TOKEN (tok, 0); //'M'
  NEXT_TOKEN (tok, 2);
  IFPAR (age, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (station, atoi (tok));
  return 3;
}

/*!
  NMEA-0183 GSA sentence

  GNSS DOP and Active Satellites

  GNSS receiver operating mode, satellites used in the navigation solution reported by the GGA or
GNS sentence, and DOP values. \verbatim
    $--GSA,a,x,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,x.x,x.x,x.x<CR><LF>
           | | |                                 |  |   |   |
           | | |                                 |  |   |   +-- VDOP
           | | |                                 |  |   +------ HDOP
           | | |                                 |  +---------- PDOP
           | | |_________________________________|
           | |            |
           | |            +--- ID numbers of satellites used in solution
           | +---------------- Mode: 1 = Fix not available, 2 = 2D, 3 = 3D
           +------------------ Mode: M = Manual, forced to operate in 2D or 3D mode
                                     A = Automatic, allowed to automatically switch 2D/3D
\endverbatim
*/
int gsa (const char* buf, int* hmode, int* fmode, int* sv, double* pdop, double* hdop, double* vdop)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "GSA", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  IFPAR (hmode, (*tok == 'A') ? 1 : (*tok == 'M') ? 2 : 0);
  NEXT_TOKEN (tok, 0);
  IFPAR (fmode, atoi (tok));
  if (sv)
  {
    for (int i = 0; i < 12; i++)
    {
      NEXT_TOKEN (tok, 0);
      sv[i] = atoi (tok);
    }
  }
  NEXT_TOKEN (tok, 0);
  IFPAR (pdop, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (hdop, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (vdop, atof (tok));
  return 3;
}

/*!
  NMEA-0183 GST sentence.

  $ttGST,time,rms,semimaj,semimin,orient,stdlat,stdlon,stdh
*/
int gst (const char* buf, double* time, double* rms, double* smaj, double* smin, double* orient,
         double* stdlat, double* stdlon, double* stdh)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "GST", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (rms, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (smaj, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (smin, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (orient, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (stdlat, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (stdlon, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (stdh, atof (tok));
  return 3;
}

/*!
  NMEA-0183 GSV sentence - Satellites in View

\verbatim
  $xxGSV,t,n,c,id,el,az,sn,id,az,el,sn,id,az,el,sn,id,az,el,sn*hh
         | | | |  |  |  |  |          ||          ||__________+- satellite 4
         | | | |  |  |  |  |          ||__________+------------- satellite 3
         | | | |  |  |  |  |__________+------------------------- satellite 2
         | | | |  |  |  +------- SNR satellite 1 (0-99)
         | | | |  |  +---------- Azimuth satellite 1 (0-359)
         | | | |  +------------- Elevation satellite 1 (0-90)
         | | | +---------------- SV number satellite 1
         | | +------------------ Total number of satellites in view
         | +-------------------- Message number
         +---------------------- Total number of messages
\endverbatim
*/
int gsv (const char* buf, int* tmsg, int* msg, int* count, int* sv, int* az, int* elev, int* snr)
{
  parse_context ctx (buf);

  int nmsg;
  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "GSV", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  IFPAR (tmsg, atoi (tok));
  NEXT_TOKEN (tok, 0);
  nmsg = atoi (tok) - 1;
  if (nmsg < 0 || nmsg > 8)
    return 0;
  IFPAR (msg, nmsg + 1);
  NEXT_TOKEN (tok, 0);
  IFPAR (count, atoi (tok));
  for (int i = 0; i < 4; i++)
  {
    NEXT_TOKEN (tok, 1);
    if (sv)
      sv[nmsg * 4 + i] = atoi (tok);
    NEXT_TOKEN (tok, 0);
    if (elev)
      elev[nmsg * 4 + i] = atoi (tok);
    NEXT_TOKEN (tok, 0);
    if (az)
      az[nmsg * 4 + i] = atoi (tok);
    NEXT_TOKEN (tok, 0);
    if (snr)
      snr[nmsg * 4 + i] = atoi (tok);
  }

  return 1;
}

/*!
  NMEA-0183 GXP sentence.

  $ttGXP,time,lat,N,lon,a,wp

  \note
    This sentence is obsolete.
*/
int gxp (const char* buf, double* lat, double* lon, double* time, int* wp)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "GXP", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S')
    *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W')
    *lon *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (wp, atoi (tok));
  return 1;
}

/*!
  NMEA-0183 HDG sentence.

  $ttHDG,hdg,dev,E/W,var,E/W

    magnetic = hdg + dev
    true = magnetic + var
*/
int hdg (const char* buf, double* head, double* dev, double* var)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "HDG", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (head, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (dev, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  if (dev && *tok == 'W')
    *dev *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (var, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  if (var && *tok == 'W')
    *var *= -1.;
  return 3;
}

/*!
  NMEA-0183 HDM sentence.

  $ttHDM,xxx.x,M

  \note
    This sentence is obsolete
*/
int hdm (const char* buf, double* head)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "HDM", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (head, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'M')
    return 0;
  return 2;
}

/*!
  NMEA-0183 HDT sentence
    $ttHDT,xxx.x,T

  \note
    This sentence is obsolete
*/
int hdt (const char* buf, double* head)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "HDT", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (head, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'T')
    return 0;
  return 2;
}

/**
  Leica LLQ sentence.

  $GPLLQ,hhmmss.ss,mmddyy,eeeeee.eee,M,nnnnnn.nnn,M,g,ss,q.q,z.z,M*hh
  - hhmmss.ss   UTC time of position
  - mmddyy      UTC date
  - eeeeee.ee   Grid Easting, meters
  - M           Meter (fixed text "M")
  - nnnnnn.nn   Grid Northing, meters
  - M           Meter (fixed text "M")
  - g           GPS Quality
                - 0 = fix not available or invalid
                - 1 = No realtime position, navigation fix
                - 2 = realtime position, ambiguities not fixed
                - 3 = realtime position, ambiguities fixed
  - ss          Number of satellites used in computation
  - q.q         Coordinate Quality
  - z.z         Altitude above/below mean sea level for
                position of marker. Note, if no orthometric
                height is available the local ell. height will be
                exported.
  - M           Meter (fixed text "M")
  - hh          Checksum
*/
int llq (const char* buf, double* time, double* x, double* y, int* mode, int* sat, double* dop,
         double* height)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "LLQ", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  NEXT_TOKEN (tok, 0); // date
  IFPAR (x, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (*tok != 'M')
    return 0;
  NEXT_TOKEN (tok, 0);
  IFPAR (y, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (*tok != 'M')
    return 0;
  NEXT_TOKEN (tok, 0);
  IFPAR (mode, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (sat, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (dop, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (height, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (*tok != 'M')
    return 0;
  return 1;
}

/*!
  NMEA-0183 Ashtech proprietary PASHR sentence.

  $PASHR,UTC-Time,HeadingTRUE,T,ROLL,PITCH,HEAVE(m),accuracyRoll, accuracyPitch,accuracyHeading

  Newer Ashtech proprietary sentence for attitude sensors. Also used
  by Applanix POS M/V systems.

  - Flag accuracy heading
    - 0 = no aiding
    - 1 = GPSAiding
    - 2 = GPS&GAMS aiding
  - Flag IMU
    - 0 = satisfactory
    - 1 = IMUOUT

\verbatim
  $PASHR,145719.272,252.41,T,1.22,0.48,0.01,0.090,0.090,0.116,2,1*11
  $PASHR,141424.923,45.36,T,-0.57,-0.63,0.02,0.086,0.086,0.025,1,1*28
\endverbatim

  \note Hemisphere has a different description:
\verbatim
      $PASHR,hhmmss.ss,HHH.HH,T,RRR.RR,PPP.PP,heave,rr.rrr,pp.ppp,hh.hhh,QF*CC<CR><LF>

  where:
    hhmmss.ss         UTC time
    HHH.HH            Heading value in decimal degrees
    T                 True heading (T displayed if heading is relative to true north)
    RRR.RR            Roll in decimal degrees (- sign will be displayed when applicable)
    PPP.PP            Pitch in decimal degrees (- sign will be displayed when applicable)
    heave             Heave, in meters
    rr.rrr            Roll standard deviation in decimal degrees
    pp.ppp            Pitch standard deviation in decimal degrees
    hh.hhh            Heading standard deviation in decimal degrees
    QF                Quality Flag
                        0 = No position
                        1 = All non-RTK fixed integer positions
                        2 = RTK fixed integer position
\endverbatim
*/
int pashr (const char* buf, double* time, double* hdg, double* pitch, double* roll, double* heave,
           double* roll_std, double* pitch_std, double* hdg_std, int* flag_h, int* flag_i)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok, "$PASHR", 6))
    return 0;
  NEXT_TOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (hdg, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0); // T
  NEXT_TOKEN (tok, 0);
  IFPAR (roll, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (pitch, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (heave, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (roll_std, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (pitch_std, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (hdg_std, atof (tok) * D2R);

  NEXT_TOKEN (tok, 0);
  IFPAR (flag_h, atoi (tok));
  NEXT_TOKEN (tok, 1);
  IFPAR (flag_i, atoi (tok));
  return 3;
}

/*!
  NMEA-0183 Hemisphere GNSS proprietary sentence

  $PSAT,HPR,time,heading,pitch,roll,type

  type:
    - N = GPS derived heading
    - G = gyro heading
*/
int psathpr (const char* buf, double* time, double* head, double* pitch, double* roll, char* type)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok, "$PSAT", 5))
    return 0;
  NEXT_TOKEN (tok, 0);
  if (memcmp (tok, "HPR", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (head, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  if (strlen (tok))
    IFPAR (pitch, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (strlen (tok))
    IFPAR (roll, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (type, tok[0]);
  return 3;
}

/*!
  NMEA-0183 Trimble proprietary PTNLGGK sentence.
\verbatim
  $PTNL,GGK,hhmmss.ss,mmddyy,llll.ll,a,yyyyy.yy,a,xx,xx,x.x,EHTxx.x,M*hh

  - hhmmss.ss   = UTC of Position Fix
  - mmddyy      = UTC date
  - llll.ll     = Latitude
  - a           = Hemisphere (N/S)
  - yyyyy.yy    = Longitude
  - a           = E/W
  - xx          = GPS Quality Indicator
                   - 0: Fix not available or invalid
                   - 1: Autonomous GPS fix
                   - 2: RTK float solution
                   - 3: RTK fix solution
                   - 4: Differential, code phase only solution (DGPS)
                   - 5: SBAS solution (WAAS/EGNOS/MSAS)
                   - 6: RTK float or RTK location 3D Network solution
                   - 7: RTK fixed 3D Network solution
                   - 8: RTK float or RTK location 2D in a Network solution
                   - 9: RTK fixed 2D Network solution
                   - 10: OmniSTAR HP/XP solution
                   - 11: OmniSTAR VBS solution
                   - 12: Location RTK solution
                   - 13: Beacon DGPS
  - xx          = Number of Satellites in use
  - x.x         = PDOP
  - xx.x        = Ellipsoidal height.
                = Altitude above/below mean sea level for
                  position of marker. Note, if no orthometric
                  height is available the local ell. height will be
                  exported.
  - M           = Units of altitude meters (fixed text "M")
  - hh          = Checksum

    $PTNL,GGK,172814.00,071296,3723.46587704,N,12202.26957864,W,3,06,1.7,EHT-6.777,M*48
\endverbatim

*/
int ptnlggk (const char* buf, double* lat, double* lon, double* time, double* height, double* dop,
             int* sat, int* mode)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok, "$PTNL", 5))
    return 0;
  NEXT_TOKEN (tok, 0);
  if (memcmp (tok, "GGK", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0); // date
  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S')
    *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W')
    *lon *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (mode, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (sat, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (dop, atof (tok));
  NEXT_TOKEN (tok, 1);
  if (memcmp (tok, "EHT", 3))
    return 0;
  IFPAR (height, atof (tok + 3));
  return 1;
}

/*!
  NMEA-0183 Trimble proprietary PTNL,QA sentence.

  \note This sentence is obsolete.
*/
int ptnlqa (const char* buf, double* sigman, double* sigmae, double* smaj, double* smin,
            double* orient)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok, "$PTNL", 5))
    return 0;
  NEXT_TOKEN (tok, 0);
  if (memcmp (tok, "QA", 2))
    return 0;
  NEXT_TOKEN (tok, 0); // time??
  NEXT_TOKEN (tok, 0);
  IFPAR (sigman, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (sigmae, atof (tok));
  NEXT_TOKEN (tok, 0); // what?
  NEXT_TOKEN (tok, 0);
  double unit = atof (tok);
  if (sigman)
    *sigman *= unit;
  if (sigmae)
    *sigmae *= unit;
  NEXT_TOKEN (tok, 0);
  IFPAR (smaj, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (smin, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (orient, atof (tok));
  return 1;
}

/*!
  NMEA-0183 RMC sentence.

  $ttRMC,time,stat,lat,N/S,lon,E/W,speed,head,date,magvar,E/W,mode
*/
int rmc (const char* buf, double* lat, double* lon, double* time, double* speed, double* head,
         int* date, int* mode)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "RMC", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0); // status
  if (*tok && *tok != 'A' && *tok != 'V')
    return 0;
  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S')
    *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DM2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W')
    *lon *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (speed, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (head, atof (tok) * D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (date, atoi (tok));
  NEXT_TOKEN (tok, 0); // skip magnetic variation
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'E' && *tok != 'W')
    return 0;
  NEXT_TOKEN (tok, 2);
  IFPAR (mode, (*tok == 'A')   ? 1
               : (*tok == 'D') ? 2
               : (*tok == 'P') ? 3
               : (*tok == 'R') ? 4
               : (*tok == 'F') ? 5
                               : 0);
  return 3;
}

/*!
  NMEA-0183 TTM sentence (Tracked Target message).

  $xxTTM,num,dist,brg,relbrg,speed,course,relcog,cpa,tcpa,units,name,status,ref,hhmmss.ss,acq*hh
  - num = target number
  - dist = target distance relative to own ship
  - brg = target bearing
  - relbrg = 'T' for true bearing, 'R' for relative
  - speed = target speed
  - course = target course
  - relcog = true/relative course
  - cpa = distance to closest point of approach
  - tca = time to CPA
  - units = speed/distance units ('K' or 'N' or 'S')
  - name = target name
  - status = 'L' lost, 'Q' query, 'T' tracking
  - ref = reference target
  - hhmmss = UTC time
  - acq = acquisition status
*/
int ttm (const char* buf, double* utc, int* num, char* name, double* dist, double* brg, int* relbrg,
         double* speed, double* cog, int* relcog, double* cpa, double* tcpa, int* stat)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "TTM", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  if (atoi (tok) < 0)
    return 0; // garbage
  IFPAR (num, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (dist, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (brg, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (relbrg, (*tok == 'R' ? 1 : 0));
  NEXT_TOKEN (tok, 0);
  IFPAR (speed, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (cog, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (relcog, (*tok == 'R' ? 1 : 0));
  NEXT_TOKEN (tok, 0);
  IFPAR (cpa, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (tcpa, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (*tok == 'K')
  {
    if (dist)
      *dist *= 1000;
    if (speed)
      *speed /= 3.6;
  }
  else if (*tok == 'N')
  {
    if (dist)
      *dist *= 1852;
    if (speed)
      *speed /= MPS2KNOT;
  }
  NEXT_TOKEN (tok, 0);
  if (name)
    strcpy (name, tok);
  NEXT_TOKEN (tok, 0);
  if (stat)
    *stat = (*tok == 'L') ? 1 : (*tok == 'Q') ? 2 : 0;
  NEXT_TOKEN (tok, 0); // reference target;
  NEXT_TOKEN (tok, 0);
  IFPAR (utc, atof (tok));

  return 1;
}

/*!
  NMEA-0183 VTG sentence.

  $ttVTG,true,T,mag,M,knots,N,kph,K,mode

  True heading takes precedence over magnetic heading. Speed value in knots
  takes precedence over value in km/h.
*/
int vtg (const char* buf, double* speed, double* head)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  bool th, sk;
  th = sk = false;
  if (!tok || memcmp (tok + 3, "VTG", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  if (head)
  {
    *head = atof (tok) * D2R;
    th = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'T')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (head && !th && *tok)
    *head = atof (tok) * D2R;
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'M')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (speed && *tok)
  {
    *speed = atof (tok);
    sk = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'N')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (speed && !sk && *tok)
    *speed = atof (tok) * 0.539957;
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'K')
    return 0;
  return 3;
}

/*!
  NMEA-0183 ZDA sentence.

  $ttZDA,time,day,month,year,loch,locm

  UTC = local +loch + locm/60
*/
int zda (const char* buf, double* time, unsigned short* day, unsigned short* month,
         unsigned short* year)
{
  parse_context ctx (buf);

  char* tok = ctx.token ();
  if (!tok || memcmp (tok + 3, "ZDA", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (!strlen (tok))
    return 0;
  IFPAR (day, (unsigned short)atoi (tok));
  NEXT_TOKEN (tok, 0);
  if (!strlen (tok))
    return 0;
  IFPAR (month, (unsigned short)atoi (tok));
  NEXT_TOKEN (tok, 0);
  if (!strlen (tok))
    return 0;
  IFPAR (year, (unsigned short)atoi (tok));
  return 3;
}

/// @}
} // namespace mlib
