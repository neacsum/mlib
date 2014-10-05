#pragma once
/*!
  \file geostruct.h - Structures and enumerations used by geodetic library
*/

///Error codes
enum geoerr {
  GEOERR_OK,        ///< No error
  GEOERR_UNKN,      ///< Unknown error
  GEOERR_HGEO,      ///< Invalid geodesy handle
  GEOERR_FLAT,      ///< Invalid ellipsoid flattening
  GEOERR_UNIT,      ///< Invalid unit to meters conversion factor
  GEOERR_PROJ,      ///< Invalid projection
  GEOERR_SNGL,      ///< Singularity
  GEOERR_PARM,      ///< Invalid projection parameters
  GEOERR_UNDL,      ///< Undulation value not available
  GEOERR_GEOM,      ///< Bad geoid model
  GEOERR_DOMAIN,    ///< Domain error
  GEOERR_NCONV,     ///< Non convergence
  GEOERR_NONAME,    ///< Local name not found
  GEOERR_DEMO,      ///< Demo mode geodesy
  GEOERR_VDAT,      ///< Missing VDatum file
  GEOERR_LIM,       ///< Outside model limits
  GEOERR_INVEPSG,   ///< Invalid EPSG code
  GEOERR_KTD        ///< Invalid or missing KTD file
};

/// Projection identifiers
enum geoproj {
  GEOPROJ_DEM,     ///< Demo mode (Platte Carre)
  GEOPROJ_LCC,     ///< Lambert conformal conical
  GEOPROJ_MER,     ///< Mercator
  GEOPROJ_TME,     ///< \ref TransverseMercator "Transverse Mercator"
  GEOPROJ_OST,     ///< Oblique stereographic
  GEOPROJ_OCY,     ///< Oblique cylindrical (Swiss and EOV systems)
  GEOPROJ_HOM,     ///< \ref ObliqueMercator "Hotine Oblique Mercator" (Alaska)
  GEOPROJ_RSO,     ///< Rectified Skew Orthomorphic
  GEOPROJ_AZD,     ///< Azimuthal Equidistant
  GEOPROJ_CME,     ///< \ref SMerc "CMAP Mercator"
  GEOPROJ_ALA,     ///< \ref Albers "Albers Equal Area"
  GEOPROJ_CAS,     ///< Cassini-Soldner
  GEOPROJ_PST,     ///< Polar Stereographic
  GEOPROJ_AZA,     ///< Azimuthal Equal Area
  GEOPROJ_GNO,     ///< Gnomonic (not implemented)
  GEOPROJ_LEA,     ///< Lambert Equal Area (not implemented)
  GEOPROJ_ORT,     ///< Orthographic (not implemented)
  GEOPROJ_POL,     ///< Polyconic
  GEOPROJ_OME      ///< Oblique Mercator
};

/*!
  Geodetic parameters needed to create a geodesy object.
*/
typedef struct tag_PROJPARAMS
{
  double a;             ///< Semi major axis in meters
  double f_1;           ///< One over flattening
  geoproj projid;       ///< Projection identifier

  double unit;          ///< Conversion factor from work unit to meters
  double scale;         ///< Scale factor
  double reflon;        ///< Longitude of reference point in radians
  double reflat;        ///< Latitude of reference point in radians
  double northpar;      ///< North parallel in radians
  double southpar;      ///< South parallel in radians
  double azskew;        ///< Azimuth of skew in radians
  double feast;         ///< False easting in work unit
  double fnorth;        ///< False northing in work unit
} PROJPARAMS;

/*!
Datum transformation structure used by SetEllDiff() and
GetEllDiff() functions.
*/
typedef struct tag_ELLDIFF
{
  double dx;            ///< Translation along X axis in meters
  double dy;            ///< Translation along Y axis in meters
  double dz;            ///< Translation along Z axis in meters
  double dsc;           ///< Scale factor
  double drx;           ///< Rotation around X axis in radians
  double dry;           ///< Rotation around Y axis in radians
  double drz;           ///< Rotation around Z axis in radians
} ELLDIFF;

/// Type of local grid adjustments
enum geoadj {
  GEOADJ_NONE,         ///<  No local grid adjustment
  GEOADJ_HELMERT,      ///<  General Helmert transformation
  GEOADJ_ZEELAND,      ///<  UTM zone 32 to S-34 Zeeland
  GEOADJ_JUTLAND,      ///<  UTM zone 32 to S-34 Jutland
  GEOADJ_KEDAH,        ///<  RSO to Cassini Kedah & Perlis
  GEOADJ_KELANTAN,     ///<  RSO to Cassini Kelantan
  GEOADJ_PAHANGNW,     ///<  RSO to Cassini Pahang (Northwest)
  GEOADJ_PAHANGNE,     ///<  RSO to Cassini Pahang (Northeast)
  GEOADJ_PAHANGSW,     ///<  RSO to Cassini Pahang (Southwest)
  GEOADJ_PAHANGSE,     ///<  RSO to Cassini Pahang (Southeast)
  GEOADJ_PERAKN,       ///<  RSO to Cassini Perak (North)
  GEOADJ_PERAKS,       ///<  RSO to Cassini Perak (South)
  GEOADJ_JOHOR,        ///<  RSO to Cassini Johor
  GEOADJ_MELAKA,       ///<  RSO to Cassini N.Sembilan &Melaka
  GEOADJ_SELANGOR,     ///<  RSO to Cassini Selangor
  GEOADJ_PPINANG,      ///<  RSO to Cassini Pulau Pinang
  GEOADJ_TERENGGANU,   ///<  RSO to Cassini Terengganu
  GEOADJ_REVX = 0x00010000,  ///< Reversed X (easting) axis
  GEOADJ_REVY = 0x00020000,  ///< Reversed Y (northing) axis
};

/*!
Local grid adjustment parameters used by SetAdjustment() and
GetAdjustment() functions
*/
typedef struct tag_ADJPARAMS
{
  geoadj type;          ///< Adjustment type

  double origx;         ///< X coordinate of origin
  double origy;         ///< Y coordinate of origin
  double alfa;          ///< Rotation angle in radians
  double scale;         ///< Scale factor
  double dx;            ///< X translation
  double dy;            ///< Y translation
} ADJPARAMS;

/*!
Methods for reduction from ellipsoid height to chart datum height
*/
enum cdlmode {
  CDM_NONE,             ///< No adjustment
  CDM_KTD,              ///< KTD file
  CDM_GEOKTD,           ///< Geoid model and KTD file
  CDM_GEOVDAT,          ///< Geoid model and VDATUM model
  CDM_GEOCDL,           ///< Geoid model and fixed CDL adjustment
  CDM_CDL               ///< Fixed CDL adjustment
};

/*!
Configuration data for vertical calculations
*/
typedef struct tag_VERT_CONFIG {
  cdlmode mode;         ///< Reduction method
  char* geoid;          ///< Geoid model (fully qualified)
  double ohc;           ///< Orthometric height correction
  char *vdatum;         ///< VDatum model path
  char *surface;        ///< VDatum model surface
  char *ktd;            ///< KTD file name (fully qualified)
  double cdl;           ///< Fixed chart datum level adjustment value
} VERT_CONFIG;

#define CRSKIND_UNKNOWN       0
#define CRSKIND_ENGINEERING   1
#define CRSKIND_GEOGRAPHIC2   2
#define CRSKIND_GEOGRAPHIC3   3
#define CRSKIND_GEOCENTRIC    4
#define CRSKIND_PROJECTED     5
#define CRSKIND_VERTICAL      6
#define CRSKIND_COMPOUND      7

typedef struct tag_EPSGPARAMS {
  size_t size;          ///< Structure size
  int code;             ///< CRS code
  int kind;             ///< One of CRSKIND_... values
  double a;             ///< ellipsoid parameters
  double f;
} EPSGPARAMS;
