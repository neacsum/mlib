/*!
  \file nmea.h Definition of NMEA-0183 parsing functions

  (c) Mircea Neacsu 2017
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

namespace mlib {

bool nmea_checksum (const char *buf);

int dbs (const char *buf, double *depth);
int dbt (const char *buf, double *depth);
int dpt (const char *buf, double *depth, double *offset, double *range);
int gga (const char *buf, double *lat, double *lon, double *time, double *height,
         double *undul, double *dop, int *sat, int *mode, double *age, int *station);
int ggk (const char *buf, double *lat, double *lon, double *time, double *height,
         double *dop, int *sat, int *mode);
int gll (const char *buf, double *lat, double *lon, double *time, int *mode);
int gns (const char *buf, double *time, double *lat, double *lon, int *mode, int *sat,
         double *dop, double *height, double *age, int *station);
int gsa (const char *buf, int *hmode, int *fmode, int *sv, double *pdop, double *hdop, double *vdop);
int gst (const char *buf, double *time, double *rms, double *smaj, double *smin,
         double *orient, double *stdlat, double *stdlon, double *stdh);
int gsv (const char *buf, int *tmsg, int *msg, int *count, int *sv, int *az, int *elev, int *snr);
int gxp (const char *buf, double *lat, double *lon, double *time, int *wp);
int hdg (const char *buf, double *head, double *dev, double *var);
int hdm (const char *buf, double *head);
int hdt (const char *buf, double *head);
int llq (const char *buf, double *time, double *x, double *y, int *mode, int *sat, double *dop, double *height);
int pashr (const char *buf, double *time, double *hdg, double *pitch, double *roll,
           double *heave, double *roll_std, double *pitch_std, double *hdg_std,
           int *flag_h, int *flag_i);
int psathpr (const char *buf, double *time, double *head, double *pitch, double *roll, char *type);
int ptnlggk (const char *buf, double *lat, double *lon, double *time, double *height,
             double *dop, int *sat, int *mode);
int ptnlqa (const char *buf, double *sigman, double *sigmae, double *smaj,
            double *smin, double *orient);
int rmc (const char *buf, double *lat, double *lon, double *time, double *speed, double *head, int *date, int *mode);
int ttm (const char *buf, double *utc, int *num, char *name, double *dist, double *brg,
         int *relbrg, double *speed, double *cog, int *relcog, double *cpa, double *tcpa,
         int *stat);
int vtg (const char *buf, double *speed, double *head);
int zda (const char *buf, double *time, unsigned short *day, unsigned short *month, unsigned short *year);

}
