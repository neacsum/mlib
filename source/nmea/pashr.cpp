#include "nmeac.h"
#include <mlib/convert.h>

/**
  PASHR sentence.

  $PASHR,UTC-Time,HeadingTRUE°,T,ROLL°,PITCH°,HEAVE(m),accuracyRoll°, accuracyPitch°,accuracyHeading°

  Newer Ashtech proprietary sentence for attitude sensors. Also used
  by Applanix POS M/V systems.

  - Flag accuracy heading 
    - 0 = no aiding
    - 1 = GPSAiding
    - 2 = GPS&GAMS aiding
  - Flag IMU 
    - 0 = satisfactory
    - 1 = IMUOUT

  \code
  $PASHR,145719.272,252.41,T,1.22,0.48,0.01,0.090,0.090,0.116,2,1*11
  $PASHR,141424.923,45.36,T,-0.57,-0.63,0.02,0.086,0.086,0.025,1,1*28
  \endcode
  
  \note Hemisphere has a different description:
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
*/
int pashr (const char *buf, double *time, double *hdg, double *pitch, double *roll,
           double *heave, double *roll_std, double *pitch_std, double *hdg_std,
           int *flag_h, int *flag_i )
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok, "$PASHR", 6))
    return 0;
  NEXT_TOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (hdg, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);          //T
  NEXT_TOKEN (tok, 0);
  IFPAR (roll, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (pitch, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (heave, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (roll_std, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (pitch_std, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (hdg_std, atof (tok)*D2R);

  NEXT_TOKEN (tok, 0);
  IFPAR (flag_h, atoi (tok));
  NEXT_TOKEN (tok, 1);
  IFPAR (flag_i, atoi (tok));
  return 3;
}
