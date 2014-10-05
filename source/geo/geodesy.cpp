/*!	
  \file GEODESY.CPP - Geodesy class implementation

  (c) Coastal Oceanographics 1994, 1996
  (c) HYPACK, Inc. 2000-2010

*/
#include <hypack/defs.h>
#include <hypack/gethydir.h>
#include <string.h>
#include <assert.h>

#include "geodesy.h"
#include "mercator.h"   //Mercator projection
#include "lambert.h"    //Lambert Conformal Conical projection
#include "tranmerc.h"   //Transverse Mercator
#include "stereo.h"     //Oblique Stereographic projection
#include "hom.h"        //Hotine Oblique Mercator projection
#include "rso.h"        //Rectified Skew Orthomorphic projection
#include "ocy.h"        //Oblique Cylindrical projection
#include "azimeqd.h"    //Azimuthal Equidistant projection
#include "albers.h"     //Albers Equal Area
#include "plate.h"      //Plate Carre
#include "cassini.h"    //Cassini-Solder
#include "polycon.h"    //Polyconic
#include "aza.h"        //Azimuthal Equal Area

//#include <seccl.h>
#include <hypack/trace.h>
#include "mathval.h"

//Indexes in strid array
#define IDX_HUNIT   0
#define IDX_VUNIT   1
#define IDX_XAXIS   2
#define IDX_YAXIS   3

extern double pb[12];
extern double pc[12];

 PROJPARAMS demo_proj = {
  6378137.,       //a
  298.257223563,  //f
  GEOPROJ_DEM,    //projid
  1.,             //unit
  1.,             //scale
  -70*M_PI/180.,  //reflon
  45*M_PI/180.,   //reflat
  0.,             //northpar
  0.,             //southpar
  0.,             //azimuth of skew
  500000.,        //false easting
  500000.         //false northing
};

#define NUNITS  15

//lookup table for unit indexes
double units[NUNITS] = {
  1.0,              //Meter
  0.3048006096,     //US Survey Foot
  0.3048,           //Intl Foot
  0.914402,         //Yard
  20.11678249,      //Chain
  1852,             //Nautical Mile
  1.8288,           //Fathom
  0.3047972654,     //Clarke Foot
  0.3047995142,     //Indian Foot
  0.91439841,       //Sears Yard
  0.91439855,       //Indian Yard
  0.2011678249,     //Link (Benoit)
  0.2011661950,     //Link (Clarke)
  0.211676512,      //Link (Sears)
  20.1167651216     //Chain (Sears)
};

static int findID (double unit);

static errfacility geo_errors ("Geodesy");
errfacility* Geodesy::errors = &geo_errors;

#define GERR(A) errc(A, ERROR_PRI_ERROR, Geodesy::errors)

//IDs for coordinate axis
#define ID_XAXIS   16
#define ID_YAXIS   17

/*!
  \class Geodesy

  This is the main object handled by the GEO32.DLL. It is exposed as an
  opaque HGEO handle for use by the "flat" API.

  It is defined by the following elements:
  - Ellipsoid
  - Projection
  - Local grid adjustment
  - Datum shift parameters
  - Geoid model
  - Chart datum model
*/

/*!
  Create an object from the GEOPARAM structure. Local adjustment, datum shift,
  geoid model and chart datum are all NULL.

  \param  pp Geodetic parameters
*/
Geodesy::Geodesy(  PROJPARAMS& pp ):
	projection (0),
	geoid (0),
  datum (0),
  surface (0),
  ktd (0),
  msl (0),
	ell_conv( false ),
	dx( 0. ), dy( 0. ), dz( 0. ),
	dsc( 1. ),
	drx( 0. ), dry( 0. ), drz( 0. ),
  h_corr (0.),
  v_unit (pp.unit),
  cdl (0),
  cdl_mode (CDM_NONE),
  epsg_code (-1),
  geopar_code (-1)
{
  TRACE (__FUNCTION__ " entered");
	if ( pp.f_1 == 0. )
		throw GERR (GEOERR_FLAT);
	if ( pp.unit == 0. )
		throw GERR (GEOERR_UNIT);

  switch( pp.projid )
  {
  case GEOPROJ_LCC:
    projection = new Lambert( pp );
    break;
  case GEOPROJ_MER:
    projection = new Mercator( pp );
    break;
  case GEOPROJ_TME:
    projection = new TransverseMercator( pp );
    break;
  case GEOPROJ_OST:
    projection = new Stereographic( pp );
    break;
  case GEOPROJ_OCY:
    projection = new ObliqueCylindrical( pp );
    break;
  case GEOPROJ_HOM:
    projection = new Hotine( pp );
    break;
  case GEOPROJ_RSO:
    projection = new RSO( pp );
    break;
  case GEOPROJ_AZD:
    projection = new AzimuthEqDist( pp );
    break;
  case GEOPROJ_CME:		// CMAP Corp's
    projection = new SMerc( pp );
    break;
  case GEOPROJ_ALA:
    projection = new Albers( pp );
    break;
  case GEOPROJ_CAS:
    projection = new Cassini( pp );
    break;
  case GEOPROJ_PST:
    projection = new PolarStereo( pp );
    break;
  case GEOPROJ_POL:
    projection = new Polyconic( pp );
    break;
  case GEOPROJ_AZA:
    projection = new AZA( pp );
    break;
  default:
    throw GERR (GEOERR_PROJ);
  }

	//initialize local grid adjustment params
	xform.type = GEOADJ_NONE;
	xform.scale = 1.;
	xform.origx = xform.origy = xform.alfa = xform.dx = xform.dy = 0.;

  //init strid array
  strid[IDX_HUNIT] = 
  strid[IDX_VUNIT] = findID (pp.unit);
  strid[IDX_XAXIS] = ID_XAXIS;
  strid[IDX_YAXIS] = ID_YAXIS;
};

/*!
  Destructor.

  Delete projection, datum shift, geoid model and chart datum objects.
*/
Geodesy::~Geodesy()
{
  TRACE (__FUNCTION__ " entered");
	delete projection;
	delete geoid;
  delete datum;
  delete surface;
  delete msl;
  delete ktd;
}

/*!
  Convert from latitude/longitude to XY coordinates.

  Uses the projection to get "world" XY coordinates than converts to local
  coordinates using World2Local() function.

  \param x    pointer to resulting X coordinate (in Hunits)
  \param y    pointer to resulting Y coordinate (in Hunits)
  \param lat  latitude (in radians)
  \param lon  longitude (in radians)
*/
errc Geodesy::GeoXY( double *x, double *y, double lat, double lon )
{
  int result = projection->GeoXY( x, y, lat, lon );
  if ( !result )
  {
	  World2Local( *x, *y );
    return ERR_SUCCESS;
  }
  return GERR(result);
}

/*!
  Convert from XY coords to geographic

  The XY coordinates are first converted to "world" coordinates using 
  Local2World() function, than world coordinates are converted to lat/lon.

  \param x    X coordinate (in Hunits)
  \param y    Y coordinate (in Hunits)
  \param lat  pointer to resulting latitude (in radians)
  \param lon  pointer to resulting longitude (in radians)
*/
errc Geodesy::XYGeo( double x, double y, double *lat, double *lon )
{
	Local2World( x, y );
	return projection->XYGeo( x, y, lat, lon );
}

/*!
  Set datum transformation parameters.

  If the geodesy used a dynamic datum transformation model, it is deleted now.

  \param diff Datum shift parameters
*/
void Geodesy::SetEllDiff( const ELLDIFF &diff )
{
	dx = diff.dx;
	dy = diff.dy;
	dz = diff.dz;
	dsc = diff.dsc;
	drx = diff.drx;
	dry = diff.dry;
	drz = diff.drz;
	ell_conv = (dx != 0.) || (dy != 0.) || (dz != 0.)
          || (drx != 0.)|| (dry!= 0.) ||(drz != 0.);

  //delete previous dynamic datum transformation model
  delete datum;
  datum = NULL;
}

/*!
  Return Datum shift parameters.

  If the geodasy uses a dynamic datum transformation model, the returned datum shift
  structure has the scale factor set to -1.

  \param diff Returned datum shift parameters
*/
void Geodesy::GetEllDiff( ELLDIFF& diff )
{
  if (datum)
  {
    diff.dx = diff.dy = diff.dz =
    diff.drx = diff.dry = diff.drz = 0;
    diff.dsc = -1;
  }
  else
  {
	  diff.dx = dx;
	  diff.dy = dy;
	  diff.dz = dz;
	  diff.dsc = dsc;
	  diff.drx = drx;
	  diff.dry = dry;
	  diff.drz = drz;
  }
}

/*!
  Set dynamic datum transformation model file

  \param model_file Name of dynamic datum transformation model
*/
void Geodesy::SetEllDiff (const char *model_file)
{
  delete datum;
  datum = NULL;
  dx = dy = dz = drx = dry = drz = 0;
  dsc = 1;
  if (model_file)
  {
    datum = new BinFile( model_file );
    ell_conv = true;
  }
  else
    ell_conv = false;
}


/*!
  Return datum transformation model file

  \param file name of dynamic datum transformation file used
  \param sz   size of file name buffer 
*/
void Geodesy::GetEllDiff (char *file, unsigned int sz)
{
	if ( datum )
		strncpy (file, datum->FileName(), sz-1);
	else
		file[0] = 0;
}

/*!
  Return geodetic parameters

  \param pp Returned geodetic parameters
*/
void Geodesy::GetParameters(  PROJPARAMS& pp ) const
{
	pp.a = projection->a();
	pp.f_1 = 1./projection->f();
	pp.projid = projection->Id();
	pp.unit = projection->Unit();
	pp.scale = projection->ScaleFactor();
	pp.reflon = projection->CentralMeridian();
	pp.reflat = projection->ReferenceLatitude();
	pp.northpar = projection->NorthParallel();
	pp.southpar = projection->SouthParallel();
	pp.azskew = projection->SkewAzimuth();
	pp.feast = projection->FalseEast();
	pp.fnorth = projection->FalseNorth();
}

/*!
  Perform a datum transformation.

  The transformation is performed from the specified \p source ellipsoid to the
  ellipsoid of the geodesy using the 7-parameters Bursa-Wolf formulas and the
  datum transformation parameters specified in ELLDIFF structure.

  \param from Source ellipsoid
  \param lat  Pointer to latitude (in radians)
  \param lon  Pointer to longitude (in radians)
  \param h    Pointer to ellipsoidal height (in meters)
*/
void Geodesy::EllConv( const Ellipsoid& from, double *lat, double *lon, double *h )
{
	if ( !datum && ell_conv && from != *projection )
	{
		//compute ECEF in "from" ellipsoid
		double x1, y1, z1;
		from.Geo2ECEF( *lat, *lon, *h, &x1, &y1, &z1 );
		//Rotate ECEF coordinates
		double x2, y2, z2;
		x2 = dx + dsc*(     x1 + drz*y1 - dry*z1);
		y2 = dy + dsc*(-drz*x1 +     y1 + drx*z1);
		z2 = dz + dsc*( dry*x1 - drx*y1 +     z1);

		//Compute geographic coordinates in our datum
		projection->ECEF2Geo( lat, lon, h, x2, y2, z2 );
	}
}

/*!
  Convert from our ellipsoid to "to" ellipsoid

  The transformation is performed from our ellipsoid to the \p to ellipsoid
  using the 7-parameters Bursa-Wolf formulas and the datum transformation
  parameters specified in ELLDIFF structure.

  \param to   Target ellipsoid
  \param lat  Pointer to latitude (in radians)
  \param lon  Pointer to longitude (in radians)
  \param h    Pointer to ellipsoidal height (in meters)
*/
void Geodesy::InvEllConv( const Ellipsoid& to, double *lat, double *lon, double *h )
{
	if ( !datum && ell_conv && to != *projection )
	{
		//compute ECEF in our ellipsoid
		double x1, y1, z1;
		projection->Geo2ECEF( *lat, *lon, *h, &x1, &y1, &z1 );
		//Rotate ECEF coordinates
		double x2, y2, z2;
		x2 = -dx + (     x1 - drz*y1 + dry*z1)/dsc;
		y2 = -dy + ( drz*x1 +     y1 - drx*z1)/dsc;
		z2 = -dz + (-dry*x1 + drx*y1 +     z1)/dsc;

		//Compute geographic coordinates in "to" ellipsoid
		to.ECEF2Geo( lat, lon, h, x2, y2, z2 );
	}
}

/*!
  Convert from WGS84 to local datum using ELLDIFF structure or datum
  transformation model.

  \param lat  Pointer to latitude (in radians)
  \param lon  Pointer to longitude (in radians)
  \param h    Pointer to ellipsoidal height (in meters)
*/
void Geodesy::WGS84Conv (double *lat, double *lon, double *h)
{
  if (ell_conv)
  {
    ELLDIFF diff;
    FindWGS84Diff (*lat, *lon, *h, diff);
    double x1, y1, z1, x2, y2, z2;

    Ellipsoid::WGS84.Geo2ECEF (*lat, *lon, *h, &x1, &y1, &z1);

    x2 = diff.dx + diff.dsc*(          x1 + diff.drz*y1 - diff.dry*z1);
    y2 = diff.dy + diff.dsc*(-diff.drz*x1 +          y1 + diff.drx*z1);
    z2 = diff.dz + diff.dsc*( diff.dry*x1 - diff.drx*y1 +          z1);

    projection->ECEF2Geo (lat, lon, h, x2, y2, z2);
  }
}

/*!
  Convert from local datum to WGS84 using reversed ELLDIFF structure or
  datum transformation model.

  \param lat  Pointer to latitude (in radians)
  \param lon  Pointer to longitude (in radians)
  \param h    Pointer to ellipsoidal height (in meters)
*/
void Geodesy::InvWGS84Conv (double *lat, double *lon, double *h)
{
  if (ell_conv)
  {
    ELLDIFF diff;
    FindWGS84Diff (*lat, *lon, *h, diff);
    double x1, y1, z1, x2, y2, z2;

    projection->Geo2ECEF (*lat, *lon, *h, &x1, &y1, &z1);

    x2 = -diff.dx + (          x1 - diff.drz*y1 + diff.dry*z1)/diff.dsc;
    y2 = -diff.dy + ( diff.drz*x1 +          y1 - diff.drx*z1)/diff.dsc;
    z2 = -diff.dz + (-diff.dry*x1 + diff.drx*y1 +          z1)/diff.dsc;

    Ellipsoid::WGS84.ECEF2Geo (lat, lon, h, x2, y2, z2);
  }
}


/*!
  Find datum transformation parameters (dx, dy, dz) from dynamic datum shift file.

  \param lat  Latitude (in radians)
  \param lon  Longitude (in radians)
  \param h    Ellipsoidal height (in meters)
  \param diff Resulting datum transformation parameters
*/
void Geodesy::FindWGS84Diff (double lat, double lon, double h, ELLDIFF& diff)
{
  double delta[3];
  int values;
  if (datum && !datum->Interpolate (lat, lon, &values, delta))
  {
    if (values == 2)
    {
      //returned values are in seconds; convert to radians before adding
      double lat84, lon84;
      lat84 = lat + delta[0]*M_PI/(180.*3600.);
      lon84 = lon + delta[1]*M_PI/(180.*3600.);

      //calculate geocentric coordinates on local ellipsoid
      double x1, y1, z1;
      Geo2ECEF (lat, lon, h, &x1, &y1, &z1);

      //same for WGS84 ellipsoid
      double x2, y2, z2;
      Ellipsoid::WGS84.Geo2ECEF (lat84, lon84, h, &x2, &y2, &z2);

      //find differences
      diff.dx = x1 - x2;
      diff.dy = y1 - y2;
      diff.dz = z1 - z2;
    }
    else
    {
      //LLS file contains direct datum shift data
      diff.dx = delta[0];
      diff.dy = delta[1];
      diff.dz = delta[2];
    }
    diff.drx = diff.dry = diff.drz = 0.;
    diff.dsc = 1.;
  }
  else
  {
    diff.dx = dx;
    diff.dy = dy;
    diff.dz = dz;
    diff.drx = drx;
    diff.dry = dry;
    diff.drz = drz;
    diff.dsc = dsc;
  }
}

/*!
  Set local grid adjustment parameters.

  \param adj  Adjustment parameters
*/
errc Geodesy::SetLocalAdjustment( const ADJPARAMS& adj )
{
	if ( adj.scale != 0 )
  {
		xform = adj;
    if (LOWORD(adj.type) == GEOADJ_HELMERT)
    {
      strid[IDX_XAXIS] = (adj.type & GEOADJ_REVX)?ID_XAXIS+2 : ID_XAXIS;
      strid[IDX_YAXIS] = (adj.type & GEOADJ_REVY)?ID_YAXIS+2 : ID_YAXIS;
    }
    return ERR_SUCCESS;
  }
	else
		return GERR (GEOERR_PARM);
}

/*!
  Return current local grid adjustment parameters.

  \param adj  Returned adjustment parameters
*/
void Geodesy::GetLocalAdjustment( ADJPARAMS& adj ) const
{
	adj = xform;
}

/*!
  Set vertical unit

  Set a vertical unit defined by its conversion factor from meters.

  \param val Conversion factor from meters to Vunit
*/
void Geodesy::VUnit (double val)
{
  v_unit = (val==0.)? projection->Unit() : val;
  strid[IDX_VUNIT] = findID (v_unit);
}

/*!
  Return the string resurce ID for an element.

*/
unsigned int Geodesy::GetStringID (geonam element)
{
  switch (element)
  {
  case GEONAM_HUNIT_ABBREV:
    return strid[IDX_HUNIT];
  case GEONAM_VUNIT_ABBREV:
    return strid[IDX_VUNIT];
  case GEONAM_XAXIS_ABBREV:
    return strid[IDX_XAXIS];
  case GEONAM_YAXIS_ABBREV:
    return strid[IDX_YAXIS];
  case GEONAM_HUNIT_SINGULAR:
    return strid[IDX_HUNIT]+32;
  case GEONAM_HUNIT_PLURAL:
    return strid[IDX_HUNIT]+64;
  case GEONAM_VUNIT_SINGULAR:
    return strid[IDX_VUNIT]+32;
  case GEONAM_VUNIT_PLURAL:
    return strid[IDX_VUNIT]+64;
  case GEONAM_XAXIS:
    return strid[IDX_XAXIS]+32;
  case GEONAM_YAXIS:
    return strid[IDX_YAXIS]+32;
  }
  return 0;
}

static int findID (double u)
{
  for (int i=0; i<NUNITS; i++)
    if (u == units[i])
      return i+1;
  return 0;
}

/*!
  Set the vertical calculation mode.

  \param vc    vertical configuration
*/
errc Geodesy::SetChartDatum (VERT_CONFIG& vc)
{
  char fname[256];

  //remove any previous models
  delete geoid;
  delete surface;
  delete msl;
  delete ktd;
  geoid = surface = msl = ktd = 0;
  h_corr = cdl = 0.;
  cdl_mode = CDM_NONE;
  
  switch (vc.mode)
  {
  case CDM_NONE:
    break;

  case CDM_GEOKTD:
    try {
      if (!strrchr (vc.geoid, '\\'))
      {
        //fill default path
        gethydir (fname);
        strcat (fname, "\\datum\\");
      }
      else
        *fname = 0;
      strcat (fname, vc.geoid);
      geoid = new BinFile (fname);
    } catch (errc& x) {
      x.deactivate ();
      return GERR (GEOERR_GEOM);
    }
    h_corr = vc.ohc; 

    // FALL THROUGH

  case CDM_KTD:
    try {
      ktd = new KTDFile (vc.ktd);
    } catch (errc& x) {
      x.deactivate ();
      return GERR (GEOERR_KTD);
	  }
    break;

  case CDM_GEOVDAT:
    try {
      if (!strrchr (vc.geoid, '\\'))
      {
        //fill default path
        gethydir (fname);
        strcat (fname, "\\datum\\");
      }
      else
        *fname = 0;
      strcat (fname, vc.geoid);
      geoid = new BinFile (fname);
	  } catch (errc& x) {
      x.deactivate ();
      return GERR (GEOERR_GEOM);
	  }
    h_corr = vc.ohc;

	  try {
      if (!strrchr (vc.vdatum, '\\'))
      {
        //fill default path
        gethydir (fname);
        strcat (fname, "\\datum\\vdatum\\");
      }
      else
        *fname = 0;
      strcat (fname, vc.vdatum);
      char *pend = fname+strlen(fname);
      strcat (fname, "\\tss.gtx");
      msl = new GtxFile (fname);
      if (vc.surface && strcmpi (vc.surface, "tss"))
      {
        strcpy (pend, "\\");
        strcat (pend, vc.surface);
        strcat (pend, ".gtx");
		    surface = new GtxFile( fname );
      }
    }
	  catch ( errc& x) {
      x.deactivate ();
      return GERR (GEOERR_VDAT);
	  }
    break;

  case CDM_GEOCDL:
    try {
      geoid = new BinFile (vc.geoid);
	  } catch (errc& x) {
      x.deactivate ();
      return GERR (GEOERR_GEOM);
	  }
    h_corr = vc.ohc;

    // FALL THROUGH

  case CDM_CDL:
    cdl = vc.cdl;
    break;
  }

  cdl_mode = vc.mode;
  return ERR_SUCCESS;
}

/*!
  Return currently used vertical configuration.

  \param vc   vertical configuration structure
*/
void Geodesy::GetChartDatum (VERT_CONFIG& vc) const
{
  char temp[256];
  vc.mode = cdl_mode;
  if (geoid && vc.geoid)
  {
    strcpy (temp, geoid->FileName());
    strcpy (vc.geoid, strrchr(temp, '\\')+1);
    vc.ohc = h_corr;
  }
  if (msl && vc.vdatum)
  {
    strcpy (temp, msl->FileName());
    *(strrchr (temp, '\\')) = 0;
    strcpy (vc.vdatum, strrchr (temp, '\\')+1);

    if (vc.surface)
    {
      if (surface)
      {
        strcpy (temp, surface->FileName ());
        *strrchr (temp, '.') = 0;
        strcpy (vc.surface, strrchr (temp, '\\')+1);
      }
      else
        strcpy (vc.surface, "TSS");
    }
  }
  if (ktd && vc.ktd)
    strcpy (vc.ktd, ktd->FileName());
  vc.cdl = cdl;
}


/*!
  Find reduction values from ellipsoidal height to chart datum level

  \param lat  Latitude (in radians)
  \param lon  Longitude (in radians)
  \param sep  Pointer separation value K-N (in meters)
  \param n    Pointer to undulation value (can be NULL)
*/
errc Geodesy::CDL (double lat, double lon, double *sep, double *n)
{
  double undul =0.;
  double x, y, k;
  errc result;

  try {
    switch (cdl_mode)
    {
    case CDM_NONE:
      *sep = 0;
      result = errc (GEOERR_LIM, ERROR_PRI_INFO, Geodesy::errors);
      break;

    case CDM_GEOKTD:
      assert (geoid);
      geoid->Interpolate (lat, lon, NULL, &undul);
      undul += h_corr;
//      undul *= -1;

      //FALL THROUGH
      
    case CDM_KTD:
      assert (ktd);
      GeoXY (&x, &y, lat, lon);
      ktd->Interpolate (y, x, NULL, &k);
      
      //KTD files are in local units
      *sep = VConvert (k, true) - undul;
      break;

    case CDM_GEOVDAT:
      assert (geoid);
      geoid->Interpolate (lat, lon, NULL, &undul);
      undul += h_corr;
//      undul *= -1;
      *sep = calc_vdat (lat, lon) - undul;
      break;

    case CDM_GEOCDL:
      assert (geoid);
      geoid->Interpolate (lat, lon, NULL, &undul);
      undul += h_corr;
//      undul *= -1;

      *sep = cdl - undul;
      break;

    case CDM_CDL:
      *sep = cdl;
      break;
    }
  } catch (errc& x) {
    if (x == IFERR_LIMITS)
      result = errc (GEOERR_LIM, ERROR_PRI_INFO, Geodesy::errors);
    else
      result = x;
  }

  if (n)
    *n = undul;

  return result;
}
/*
  Calculate VDatum separation.

  VDatum separation has 2 components: separation to local mean
  sea level and the local mean sea level to tidal datum. The first one is
  interpolated from the msl file (TSS.GTX) and the second from the vdatum file.

  \param lat  latitude (in radians)
  \param lon  longitude (in radians)
*/
double Geodesy::calc_vdat (double lat, double lon) const
{
  double k1, k2;
  assert (msl);
  msl->Interpolate (lat, lon, NULL, &k1);

  if (surface)
    surface->Interpolate (lat, lon, NULL, &k2);
  else
    k2 = 0;

  return (k1 - k2);
}
