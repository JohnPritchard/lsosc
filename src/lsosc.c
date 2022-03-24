/* "@(#) $Id: lsosc.c,v 1.7 2015/06/09 14:23:23 jpritcha Exp $" */
/*
  This is lsosc.c

  Copyright (C) 2000  J.D.Pritchard <j.pritchard@eso.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <getopt.h>
#include <libgen.h>
#include <locale.h>

#include "liblsosc.h"

void usage( char ** argv )
{
  /*
    How this programme (should) work...
    1) You give it the <T0> <P> <RA> <Dec> <phase-i> <phase-f> <jd-i> <jd-f> of the MCEB
    2) For each day between jd-i & jd-f it...
      (i)  Calculates sunrise and sunset...
     (ii)  Calculates the time of day at which the object is within the safty zones (E & W of pier)
    (iii)  Calculates the interval within that determined in (ii) during which the object is within the specified phase range.
  */

  printf ("Usage : %s [-h|--help|-v] [-N|--now]\n",basename(argv[0]));
  printf ("             [-c <RA,Dec[,Epoch]>]  [-j <start-date,end-date>] [-l <text>]\n");
  printf ("             [-C <RAdeg,Decdeg[,Epoch]>]\n");
  printf ("             [-A <min-Altitude>]  [-E <T0,P>]  [-p <phase-i,phase-f>]\n");
  printf ("             [-e <expdur(sec)>]  [-m]\n");
  printf ("             [-o <preExpOverhead(sec),postExpOverhead(sec)>]\n");
  printf ("             [-s <Sun-Alt-BEoN>]  [-T <telescope>] [-t <name>,<lat>,<long>,<alt>] [-X <max-Airmass>]\n");
  printf ("             [-d <Min-Moon-Dist>] [-U <UTC-Time[,UTC-Date]>]\n");
  printf ("             [-L <LSTmin,LSTmax>]\n");
  printf ("             [-J <MJD>]\n");
  printf ("             [-P|--PParAng] [-Z|--PAltAz] [-z|--PZenDist] [-N|--now]\n");
  printf ("             [-a|--ZAZ <deg>]\n\n");
  printf ("Note:\n");
  printf (" *)  <RA,Dec[,Epoch]> should be specified in hh:mm:ss.s,dd:mm:ss.s[,yyyy.y]\n");
  printf (" *)  <RAdeg,Decdeg[,Epoch]> should be specified in ddd.dd,dd.dd[,yyyy.y]\n");
  printf ("format. If no coordinates are specified Sunset and Sunrise [SSSR] and Begining\n");
  printf ("and End of Night [BEoN] are calcualted. If co-ordinates, but no Epoch are\n");
  printf ("specified 2000.0 is assumed.\n");
  printf (" *)  <start-date,end-date> can be specified as either calendar dates (in\n");
  printf ("either yyyy-mm-dd or yyyy/mm/dd format) or as Julian day numbers (real). If no\n");
  printf ("interval is specified, calcualations are made for the current day (i.e. the\n");
  printf ("nearest midnight).\n");
  printf (" *)  Sun-Alt-BEoN should be negative for below the horizon (default is -13.0\n");
  printf ("degrees).\n");
  printf (" *)  Valid <telescope> are LSO, 2p2, NTT, 3p6, DK154, ESO152, VLT, UT1, UT2,\n");
  printf ("UT3, UT4, MJUO, and LasCampanas==OGLE. The default is LSO.\n");
  printf (" *)  Using -m more than once increases the amount of moon info provided.\n");
  printf (" *)  <UTC-Time[,UTC-Date]> in format HH:MM:SS.SS,YYYY-MM-DD only, YYYY/MM/DD\n");
  printf ("is NOT supported here.\n");
  printf (" *)  -d option not fully implemented... USE AT YOUR OWN RISK!!!\n");
  printf (" *)  -a|--ZAZ <deg> zenith avoidance zone, <deg> degrees!!!\n\n");
  printf ("A command-line observation-planning tool specifically for the Danish-1.54m\n");
  printf ("telescope at ESO, La Silla, Chile which takes into account the saftey zone of\n");
  printf ("the telescope and instrument and also providing for optional, user-specified\n");
  printf ("airmass. Also useful for planning observations at some other telescopes.\n");
  printf ("Under development, new features and new telescopes will be added with time.\n");
}

void
versioninfo( char **argv )
{
  // $Revision: 1.7 $ and $Date: 2015/06/09 14:23:23 $ are CVS keywords
  /*
  char *rev={"$Revision: 1.7 $"};
  char *date={"$Date: 2015/06/09 14:23:23 $"};
  char *rstr,*dstr;
  rstr=strtok(rev,"$Revision: ");
  printf("%s\n",rstr);
  rstr=strtok(NULL," ");
  printf("%s\n",rstr);
  dstr=strtok(date,"$Date: "); dstr=strtok(NULL," ");
  printf("%s - Version %s, %s"
  */
  printf("%s - Version , $Revision: 1.7 $, $Date: 2015/06/09 14:23:23 $\n\n",basename(argv[0]));
  printf("By J.D.Pritchard <j.pritchard@eso.org>\n");
  printf("Based on the skycalc.c and skycalendar.c codes by John Thorstensen\n");
}

int main ( int argc, char **argv )
{
  setlocale(LC_ALL,"C");
  int ed=0,preoh=0,postoh=0,y,m,d,j,icn,i,iew,hr,min,sec,emin,esec,icloser,n_orbits=0;
  double jdmid,jdmidUTC,mjds,fd;
  double T0=0.,T0shift=0.,P=-1.0,phi=0.,phishift=0.,phf=1.,phfshift=0.;
  double jdi,jdf,jdph[2][2],jdph_0,jdph_i[2];
  double ha[2][3],jdha[2],sssr[2][2],ssssr[2];

  double maxX=100.00,alt,az,minAlt=0.,haAtMinAlt[2],tha;
  double altBEoN=-13.0,altSSSR=-0.83;
  double RA,Dec,tRA,tDec,Epoch,Equinox;
  const char *str;
  short  dow;
  struct date_time date,di,df;
  extern char *tzname[2];
  double sZAZ=0.,myZAZ=0.,TinZAZ=0.,ZAZfac=2.0;

  double jdUTC,LSTmin,LSTmax;

  double moon[11],minMoonDist,minMoonDistTonight,maxMoonDistTonight,moond[3];
  short  bdmoon=0;

  double jd,ph[2][2],pht[2][3],safezone[2][2];
  short bfpr=0,bCalcNow=0,bEphem=0,bError=0,bPrintOpts=0,bPhase=0,bQuiet=0;
  short btrec=0;
  short bptrec=0;

  const char *telescope={"DK154"};
  const char *label={0};
  const char *EWtext[]={"EAST","WEST",0};

  double geora, geodec, geodist;  /* geocent for moon, not used here.*/
  double rasun, decsun;
  double stmid, stmidUTC, ramoon, decmoon, distmoon;

  /*
    Danish-1.54, ESO, La Silla

    An email from Heath Jones, 2p2 Team member...
    =============================================
    Date: Fri, 13 Oct 2000 15:59:37 -0400
    From: "2.2 telescope team" <2p2team@eso.org>
    Organization: European Southern Observatory
    To: John Pritchard <j.pritchard@astro.ku.dk>
    CC: Patrick FRANCOIS <fpatrick@eso.org>, James Brewer <jbrewer@astro.ubc.ca>
    Subject: Re D1.54 Safety Zone map

    <<<SNIP>>>

    The exact coordinates of the D154 are 
        70 deg 44' 07"662 W
        29 deg 15' 14"235 S
    The telescope is at an altitude of 2340 m.

    <<<SNIP>>>

  */

// Danish-1.54m::
  double longit = ((70.+44./60.+07.662/3600.)/180.*12.), *plongit;
  double lat = (-29.-15./60.-14.235/3600.), *plat;
  double elev = 2340.00, *pelev;
  short bCS, *pbCS;
  short insz=2, *pinsz;

  /*
    From the gettimeofday man ctime:
SYNOPSIS
       #include <time.h>

       char *asctime(const struct tm *timeptr);

       char *ctime(const time_t *timep);

       struct tm *gmtime(const time_t *timep);

       struct tm *localtime(const time_t *timep);

       time_t mktime(struct tm *timeptr);

       extern char *tzname[2];
       long int timezone;
       extern int daylight;

    From man time
SYNOPSIS
       #include <time.h>

       time_t time(time_t *t);

DESCRIPTION
       time returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.

       If t is non-NULL, the return value is also stored in the memory pointed to by t.

RETURN VALUE
       On success, the value of time in seconds since the Epoch is returned.  On error, ((time_t)-1) is returned, and errno is set appropriately.
  */
  long int timezone;
  extern int daylight;
  struct tm gmt,lt;
  time_t ct,tzoff;

  int c;
  int digit_optind = 0;
  int this_option_optind = optind ? optind : 1;
  int option_index = 0;
  struct option longopts[] =
  {
    /* { name  has_arg  *flag  val } */
    {"BEoNAlt",       1, 0, 's'}, /* Altitude for Begining/End of night calculation */
    {"airmass",       1, 0, 'X'}, /* Maximum Airmass */
    {"altitude",      1, 0, 'A'}, /* Minimum Altitude */
    {"coordinates",   1, 0, 'c'}, /* Coordinates : RA,Dec */
    {"coordinates_in_deg",   1, 0, 'C'}, /* Coordinates : RA,Dec in degrees */
    {"disttomoon",    1, 0, 'd'}, /* Distance to the moon */
    {"ephemeris",     1, 0, 'E'}, /* Ephemeris */
    {"expduration",   1, 0, 'e'}, /* Exposure Duration */
    {"help",          0, 0, 'h'}, /* help */
    {"jd",            1, 0, 'j'}, /* Julian Date interval */
    {"LSTlims",       1, 0, 'L'}, /* LST limits */
    {"label",         1, 0, 'l'}, /* Label */
    {"moon",          0, 0, 'm'}, /* Display moon parameters */
    {"overhead",      1, 0, 'o'}, /* Overhead */
    {"phaseinterval", 1, 0, 'p'}, /* Phase interval */
    {"PParAng",       1, 0, 'P'}, /* Print Paralactic Angle */
    {"PAltAz",        1, 0, 'Z'}, /* Print Altitude & Azimuth */
    {"PZenDist",      1, 0, 'z'}, /* Print Zenith Distance */
    {"now",           0, 0, 'N'}, /* Calculate for "Now" */
    {"UTC",           1, 0, 'U'}, /* Calculate for "UTC" */
    {"MJD",           1, 0, 'J'}, /* Calculate for "MJD" */
    {"ZAZ",           1, 0, 'a'}, /* Zenith avoidance distance */
    { 0, 0, 0, 0 }
  };

  ct=time(NULL)+12*3600;
  lt=*localtime( &ct );
  lt.tm_hour=0;
  lt.tm_min=0;
  lt.tm_sec=0;
  ct=mktime( &lt );
  gmt=*gmtime( &ct );
  tzoff=(gmt.tm_hour)*3600+(gmt.tm_min)*60+(gmt.tm_sec);
  //printf("Local Date at nearest MidNight is %4d-%2.2d-%2.2d\n",(1900+lt.tm_year),(1+lt.tm_mon),lt.tm_mday);
  //printf("GMT Date:Time at nearest MidNight is %4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d\n",(1900+gmt.tm_year),(1+gmt.tm_mon),gmt.tm_mday,gmt.tm_hour,gmt.tm_min,gmt.tm_sec);
  di.y=(1900+gmt.tm_year);di.mo=(gmt.tm_mon+1);di.d=gmt.tm_mday;di.h=gmt.tm_hour,di.mn=gmt.tm_min;di.s=gmt.tm_sec;
  jdi=jdf=date_to_jd( di );
  //printf("Corresponding JD is : %9.1f\n",jdi);
  RA=-1.0;Dec=0.;
  moon[0]=0.;
  Equinox=2000.0;
  minMoonDist=-200.;
  maxMoonDistTonight=-199.;
  jdUTC=-1.;
  LSTmin=-1.;
  LSTmax=99.;

  /*
    Check for how the program was call...
    Recognised options...
      dk154sc
      eso152sc
      2p2sc
      vltsc
  */

  telescope="LSO";
  if ( ! strcmp(basename(argv[0]),"dk154sc" ) ) telescope="DK154";
  if ( ! strcmp(basename(argv[0]),"eso152sc") ) telescope="ESO152";
  if ( ! strcmp(basename(argv[0]),"2p2sc"   ) ) telescope="2p2";
  if ( ! strcmp(basename(argv[0]),"nttsc"   ) ) telescope="NTT";
  if ( ! strcmp(basename(argv[0]),"3p6sc"   ) ) telescope="3p6";
  if ( ! strcmp(basename(argv[0]),"ut1sc"   ) ) telescope="UT1";
  if ( ! strcmp(basename(argv[0]),"ut2sc"   ) ) telescope="UT2";
  if ( ! strcmp(basename(argv[0]),"ut3sc"   ) ) telescope="UT3";
  if ( ! strcmp(basename(argv[0]),"ut4sc"   ) ) telescope="UT4";
  setTelescope( telescope, bptrec, &btrec, &bCS, &insz, &longit, &lat, &elev, EWtext );

  while (1) {
    c = getopt_long (argc, argv, "A:C:J:L:NPT:X:U:Za:c:d:e:E:hj:l:mo:p:qs:t:vz",
                     longopts, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 0:
      printf ("option %s", longopts[option_index].name);
      if (optarg)
        printf (" with arg %s", optarg);
      printf ("\n");
      break;

    case '0':
    case '1':
    case '2':
      if (digit_optind != 0 && digit_optind != this_option_optind)
        printf ("digits occur in two different argv-elements.\n");
      digit_optind = this_option_optind;
      printf ("option %c\n", c);
      break;

    case 'A':
      sscanf(optarg,"%lf",&minAlt);
      maxX=1./sin(minAlt / DEG_IN_RADIAN);
      minAlt=asin(1./maxX) * DEG_IN_RADIAN;
      //printf ("option A with value `%s'  :: Minimum Altitude'\n", optarg);
      break;

    case 'C':
      str=strtok(optarg,",");
      if ( str != NULL ) {
        RA=atof( str )/15.0;
        str=strtok(NULL,",");
        if ( str != NULL ) {
          Dec=atof( str );
          str=strtok(NULL,",");
          if ( str != NULL ) Equinox=atof( str );
        } else {
          RA=-1.;
        }
      }
      break;

    case 'E':
      sscanf(optarg,"%lf,%lf",&T0,&P);
      //printf ("option E with value `%s' :: Ephemeris T0=%20.6f, P=%20.5f\n", optarg,T0,P);
      bEphem=1;
      break;

    case 'J':
      if ( optarg != NULL ) {
        jdUTC=atof(optarg)+2400000.5;
      } else {
        jdUTC = -1.;
      }
      break;

    case 'L':
      str=strtok(optarg,",");
      if ( str != NULL ) {
        LSTmin=get_coorddes( str );
        str=strtok(NULL,",");
        if ( str != NULL ) {
          LSTmax=get_coorddes( str );
        } else {
          LSTmin=-1.;
          LSTmax=99.;
        }
      }
      break;

    case 'N':
      bCalcNow=1;
      break;

    case 'P':
      bPrintOpts+=1;
      break;

    case 'T':
      telescope=optarg;
      bptrec=1;
      btrec=0;
      setTelescope( telescope, bptrec, &btrec, &bCS, &insz, &longit, &lat, &elev, EWtext );
      break;

    case 'U':
      str=strtok(optarg,":");
      if ( str != NULL ) {
        di.h=atoi( str );di.mn=atoi( strtok(NULL,":"));di.s=atof( strtok(NULL,","));
        str=strtok(NULL,"-");
        if ( str != NULL ) {
          //if ( index( str,'-' ) != NULL ) {
          di.y=atoi( str );di.mo=atoi( strtok(NULL,"-"));di.d=atoi( strtok(NULL,","));
          //} else if ( index( str,'/' ) != NULL ) {
          //   di.y=atoi( strtok(NULL,"/") );di.mo=atoi( strtok(NULL,"/"));di.d=atoi( strtok(NULL,","));
          //}
        } else {
          ct=time(NULL);
          lt=*localtime( &ct );
          ct=mktime( &lt );
          gmt=*gmtime( &ct );
          di.y=(1900+gmt.tm_year);di.mo=(gmt.tm_mon+1);di.d=gmt.tm_mday;
          //di.h=gmt.tm_hour,di.mn=gmt.tm_min;di.s=gmt.tm_sec;
        }
        jdUTC = date_to_jd( di );
      } else {
        jdUTC = -1.;
      }
      break;

    case 'X':
      sscanf(optarg,"%lf",&maxX);
      //printf ("option X with value `%s'  :: Maximum Airmass'\n", optarg);
      break;

    case 'Z':
      bPrintOpts+=2;
      break;

    case 'a':
      sscanf(optarg,"%lf",&myZAZ);
      break;

    case 'c':
      str=strtok(optarg,",");
      if ( str != NULL ) {
        RA=get_coorddes( str );
        str=strtok(NULL,",");
        if ( str != NULL ) {
          Dec=get_coorddes( str );
          str=strtok(NULL,",");
          if ( str != NULL ) Equinox=atof( str );
        } else {
          RA=-1.;
        }
      }
      break;

    case 'd':
      sscanf(optarg,"%lf",&minMoonDist);
      break;

    case 'e':
      sscanf(optarg,"%d",&ed);
      break;

    case 'h':
      usage( argv );
      exit(0);
      break;

    case 'j':
      if ( index( optarg,'-' ) != NULL ) {
        di.y=atoi( strtok(optarg,"-") );di.mo=atoi( strtok(NULL,"-"));di.d=atoi( strtok(NULL,","));
        df.y=atoi( strtok(NULL,"-"));df.mo=atoi( strtok(NULL,"-"));df.d=atoi( strtok(NULL,"-"));
        di.h=(int) (12.+longit);di.mn=0;di.s=0;df.h=(int) (12.+longit);df.mn=0;df.s=0;
        jdi=date_to_jd( di );jdf=date_to_jd( df );
      } else if ( index( optarg,'/' ) != NULL ) {
        di.y=atoi( strtok(optarg,"/") );di.mo=atoi( strtok(NULL,"/"));di.d=atoi( strtok(NULL,","));
        df.y=atoi( strtok(NULL,"/"));df.mo=atoi( strtok(NULL,"/"));df.d=atoi( strtok(NULL,"/"));
        di.h=(int) (12.+longit);di.mn=0;di.s=0;df.h=(int) (12.+longit);df.mn=0;df.s=0;
        jdi=date_to_jd( di );jdf=date_to_jd( df );
      } else {
        sscanf(optarg,"%lf,%lf",&jdi,&jdf);
      }
      break;

    case 'l':
      label=optarg;
      break;

    case 'm':
      bdmoon=1;
      moon[0]++;
      break;

    case 'o':
      sscanf(optarg,"%d,%d",&preoh,&postoh);
      //printf ("option o with value `%s' :: Overhead\n", optarg);
      break;

    case 'p':
      sscanf(optarg,"%lf,%lf",&phi,&phf);
      bPhase=1;
      //printf ("option p with value `%s'  :: Phase interval %6.4f--%6.4f'\n", optarg,phi,phf);
      break;

    case 'q':
      bQuiet=1;
      break;

    case 's':
      sscanf(optarg,"%lf",&altBEoN);
      break;

    case 't':
      if ( index( optarg,',' ) != NULL ) {
        telescope=strtok(optarg,",");
        bptrec=1;
        btrec=1;
        bCS=0;
        lat = atof(strtok(NULL,","));
        longit = -1.0*atof(strtok(NULL,","))/180.0*12.0;
        elev = atof(strtok(NULL,","));
        insz=1;
        EWtext[0]="AML ";
//      } else {
//
      }
//      if ( bptrec == 1 ) printf("case[t]:Using telescope %s instead [%f,%f,%f].\n",telescope,lat,-1.*180./12.*longit,elev);
      break;

    case 'v':
      versioninfo( argv );
      exit(0);
      break;

     case 'z':
      bPrintOpts+=4;
      break;

    case '?':
      bError=1;
      break;

    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }

  if ( btrec == 0 ) {
    printf("Warning!!! Telescope %s NOT recognised. ",telescope);
    telescope="LSO";
    printf("Using DEFAULT telescope %s instead [%f,%f,%f].\n",telescope,lat,-1.*180./12.*longit,elev);
  } else {
    if ( bptrec == 1 ) printf("Using telescope %s instead [%f,%f,%f].\n",telescope,lat,-1.*180./12.*longit,elev);
  }
  //printf("RA = %f :: Dec = %f\n", RA, Dec);
   
  /*
  if (( (argc-optind) != 2 ) || ( bError )) {
    usage( argv );
    exit (1);
  }
  */

  if (( bPhase ) && ( ! bEphem )) {
    printf("ERROR!!! You MUST specify -E <T0,P> if you specify a phase range.");
    exit (1);
  }

  if ( phi > phf ) {
    phishift=-1.*phf;
    phfshift=1.-1.*phf;
    T0shift=-1.*P*phishift;
  }

  if (( phi <= 0. ) && ( phf >= 1. )) bfpr=1;

  if ( ! bQuiet ) printint(NULL,0.,0.,0.,compPhase(0.,T0,P),compPhase(0.,T0,P),RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);

  jd=jdi;
  while ( jd <= jdf ) {

    if ( RA >= 0. ) {
      tRA=RA;tDec=Dec;
      Epoch=2000.+(jd-J2000)/365.25;
      precrot(tRA,tDec,Equinox,Epoch,&RA,&Dec);
      minAlt=asin(1./maxX) * DEG_IN_RADIAN;
      if ( ! bCS ) {
        /*
          haAtMinAlt[0] != -1.*haAtMinAlt[1] because we want that the exposure is
          made entirely within the Airmass limit ie must not begin until X<maxX and
          must finish before X>maxX
        */
        safezone[1][0]=MIN(ha_alt(Dec,lat,minAlt),12.);
        safezone[0][0]=-1.*safezone[1][0];
        safezone[0][1]=safezone[0][0];
        safezone[1][1]=safezone[1][0];
        printf("Max Airmass=%6.3f ==> Min Alt=%6.3f ==> HA=",maxX,minAlt);
        myput_coords(safezone[0][0],2);
        printf(" -- ");
        myput_coords(safezone[1][0],2);
        printf(" Corresponding Paralactic Angle range is %6.3f --> %6.3f\n",parang(safezone[0][0],Dec,lat),parang(safezone[1][0],Dec,lat));
//        printf(" Corresponding Paralactic Angle range is %6.3f --> %6.3f\n",parang(-0.1,Dec,lat),parang(0.1,Dec,lat));
        TinZAZ=0.;
        ZAZfac=2.0;
        if (( myZAZ > 0. ) && ( ((lat - tDec)*(lat - tDec)) < (myZAZ*myZAZ) ))
           {
              printf("ZAZ: MINIMUM Distance from ZENITH = %f deg\n",(lat-tDec));
              printf("ZAZ: Target passes through the ZENITH AVOIDANCE ZONE (i.e. within %4.2f deg of the Zenith)\n",myZAZ);
              TinZAZ=sqrt(myZAZ*myZAZ-(lat-tDec)*(lat-tDec))/15.0;
              ZAZfac=1.0;
              printf("ZAZ: Time In ZAZ = ");
              myput_coords(TinZAZ,2);
              printf(", Execution time available = ");
              myput_coords(( safezone[1][0]*ZAZfac - TinZAZ - ed/3600. ),2);
              printf("\n");
           }
         if (( safezone[1][0]*ZAZfac - TinZAZ - ed/3600. ) < 0. )
           {
              printf("ZAZ:EI Airmass constraint does NOT allow for execution time of OB.\n");
           }
      } else {
        calcSafty( telescope, Dec, safezone );
        printf("Safe Zone is: %s [",EWtext[0]);
        myput_coords(safezone[0][0],0);
        printf(",");
        myput_coords(safezone[1][0],0);
        printf("] :: %s [",EWtext[1]);
        myput_coords(safezone[0][1],0);
        printf(",");
        myput_coords(safezone[1][1],0);
        printf("]\n");
        safezone[0][0]=MAX(-1.*ha_alt(Dec,lat,minAlt),safezone[0][0]);
        safezone[1][0]=MIN(+1.*ha_alt(Dec,lat,minAlt),safezone[1][0]);
        safezone[0][1]=MAX(-1.*ha_alt(Dec,lat,minAlt),safezone[0][1]);
        safezone[1][1]=MIN(+1.*ha_alt(Dec,lat,minAlt),safezone[1][1]);
      }
    }

    //mjd=(double) ((int) ( jd - 2400000.5 + ((double) tzoff)/86400. + 0.25 ));
    jdmid=( ((int) jd) + 0.5 + longit/24. );
    stmid = lst(jdmid,longit);

    if ( bCalcNow ) {
      ct=time(NULL);
      lt=*localtime( &ct );
      ct=mktime( &lt );
      gmt=*gmtime( &ct );
      di.y=(1900+gmt.tm_year);di.mo=(gmt.tm_mon+1);di.d=gmt.tm_mday;di.h=gmt.tm_hour,di.mn=gmt.tm_min;di.s=gmt.tm_sec;
      if ( bdmoon ) {
        calcBaEofNight( jdmid, ssssr, moon, lat, longit, elev, -0.83 );
        sfmoon( date_to_jd( di ),-1.,moon,lat,longit,elev );
      }
      printint("Now ",jdmid,date_to_jd( di ),-1.,compPhase(date_to_jd( di ),T0,P),0.,RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);
      bCalcNow=0;
    }
    if ( jdUTC > 0. ) {
      jdmidUTC=( ((int) jdUTC) + 0.5 + longit/24. );
      stmidUTC = lst(jdmidUTC,longit);
      if ( bdmoon ) {
        calcBaEofNight( jdmidUTC, ssssr, moon, lat, longit, elev, -0.83 );
        sfmoon( jdUTC,-1.,moon,lat,longit,elev );
      }
      printint("UTC ",jdmidUTC,jdUTC,-1.,compPhase(jdUTC,T0,P),0.,RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);
      bCalcNow=0;
    }
// Don't know why but calcBaEofNight screws up the value of myZAZ, so we use sZAZ to circumvent... one day should work out what the problem is...
    sZAZ=myZAZ;
    calcBaEofNight( jdmid, ssssr, moon, lat, longit, elev, altSSSR );
    printint("SSSR",jdmid,ssssr[0],ssssr[1],compPhase(ssssr[0],T0,P),compPhase(ssssr[1],T0,P),RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);
    calcBaEofNight( jdmid, ssssr, moon, lat, longit, elev, altBEoN );
    printint("BEoN",jdmid,ssssr[0],ssssr[1],compPhase(ssssr[0],T0,P),compPhase(ssssr[1],T0,P),RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);
    myZAZ=sZAZ;
    /*
    if ( bdmoon ) {
printf("Sunset        :: jdrise %f :: jdset %f :: RA %f :: Dec %f :: frac %f\n",moon[1],moon[2],moon[6],moon[7],moon[8]);
printf("Midnight Moon :: jdrise %f :: jdset %f :: RA %f :: Dec %f :: frac %f\n",moon[1],moon[2],moon[3],moon[4],moon[5]);
printf("Sunrise       :: jdrise %f :: jdset %f :: RA %f :: Dec %f :: frac %f\n",moon[1],moon[2],moon[9],moon[10],moon[11]);
    }
    */

    //printf("minMoonDist = %f\n",minMoonDist);
    if (( minMoonDist < 180. ) && ( minMoonDist > -180. )) {
      moond[0]=180.*(subtend(RA,Dec,moon[6],moon[7]))/PI;
      moond[1]=180.*(subtend(RA,Dec,moon[3],moon[4]))/PI;
      moond[2]=180.*(subtend(RA,Dec,moon[9],moon[10]))/PI;
      minMoonDistTonight=MIN(moond[0],MIN(moond[1],moond[2]));
      //printf("minMoonDistTonight = %f\n",minMoonDistTonight);
      //printf("moond[0] = %f :: moond[1] = %f :: moond[2] = %f\n",moond[0],moond[1],moond[2]);
      if ( minMoonDistTonight < minMoonDist ) {
        //printf("minMoonDistTonight < minMoonDistTonight\n");
        maxMoonDistTonight=MAX(moond[0],MAX(moond[1],moond[2]));
        if ( maxMoonDistTonight < minMoonDist ) {
          // Observations not possible...
        } else if (( moond[0] < moond[1] ) && ( moond[1] < moond[2] )) {
          // Monotonically increasing...
          // Linear interpolation for the revised Beginning of Night
          //printf("Monotonically increasing...\n");
          //printf("ssssr[0][Before]=%f :: ",ssssr[0]);
          if ( minMoonDist < moond[1] ) {
            ssssr[0]=ssssr[0]+(minMoonDist-moond[0])/(moond[1]-moond[0])*(jdmid-ssssr[0]);
          } else {
            ssssr[0]=jdmid+(minMoonDist-moond[1])/(moond[2]-moond[1])*(ssssr[1]-jdmid);
          }
          //printf("ssssr[0][After]=%f\n",ssssr[0]);
        } else if (( moond[0] > moond[1] ) && ( moond[1] > moond[2] )) {
          // Monotonically decreasing...
          // Linear interpolation for the revised End Of Night
          //printf("Monotonically decreasing...\n");
          //printf("ssssr[0][Before]=%f :: ",ssssr[0]);
          if ( minMoonDist < moond[1] ) {
            ssssr[1]=ssssr[0]+(minMoonDist-moond[0])/(moond[1]-moond[0])*(jdmid-ssssr[0]);
          } else {
            ssssr[1]=jdmid+(minMoonDist-moond[1])/(moond[2]-moond[1])*(ssssr[1]-jdmid);
          }
          //printf("ssssr[0][After]=%f\n",ssssr[0]);
        } else {
          // Complicated!!!
          printf("Moon Distance is COMPLICATED!!!\n");
          maxMoonDistTonight=-199.;
        }
      } else {
        maxMoonDistTonight=minMoonDist+1.;
      }
    }

    if (( RA >= 0. ) && ( maxMoonDistTonight > minMoonDist )){
    printf("%f  %f\n",ssssr[0],ssssr[1]);
      ha[0][0]=hainm12top12( ssssr[0], longit, RA );
      ha[1][0]=lst(ssssr[1],longit)-RA;
      ha[1][0]=hainm12top12( ssssr[1], longit, RA );
      if ( ha[1][0] < ha[0][0] ) ha[1][0]=ha[1][0]+24.;
      for (iew=0;iew<insz;iew++) {
        for (i=0;i<2;i++ ) {
          ha[0][(iew+1)]=MAX(MAX(ha[0][0],safezone[0][iew]),LSTmin-RA-12.);
	  printf("%f  %f  %f \n",ha[0][0],safezone[0][iew],LSTmin-RA-12.);
          ha[1][(iew+1)]=MIN(MIN(ha[1][0],safezone[1][iew]),LSTmax-RA-12.);
	  printf("%f  %f  %f \n",ha[1][0],safezone[1][iew],LSTmax-RA-12.);
	  printf("%f  %f\n",ha[0][(iew+1)],ha[1][(iew+1)]);
          if ( ha[0][(iew+1)] < ha[1][(iew+1)] ) {
            jdph[0][iew]=ssssr[0]-(ha[0][0]-ha[0][(iew+1)])/SID_RATE/24.;
            jdph[1][iew]=ssssr[1]-(ha[1][0]-ha[1][(iew+1)])/SID_RATE/24.;
    printf("%f  %f\n",jdph[0][iew],jdph[1][iew]);
            if ( bEphem ) {
              // calculate the enumber of orbits since T0 at jdph[0][iew]
              n_orbits=(jdph[0][iew]-T0)/P;
              printf("n_orbits = %d\n",n_orbits);
              printf("n_orbits + phi = %f\n",n_orbits+phi);
              //jdph_i[0] = jdph[0][iew];
              // Compute the jd during this orbit with phase phi...
              jdph_0=T0+(n_orbits+phi)*P;
              /*
              if ( jdph_i[0] < jdph[0][iew] ) {
                jdph_i[0]=jdph[0][iew];
              }
              */
              while ( jdph_0 < jdph[1][iew] ) {
                printf("jdph_0 = [%f]\n",jdph_0);
                jdph_i[0]=jdph_0;
                if ( jdph_i[0] < jdph[0][iew] ) {
                  jdph_i[0]=jdph[0][iew];
                }
                // check if the jd of the end of the phase interval is less than jdph[1][iew]
                jdph_i[1]=jdph[1][iew];
                if ( bPhase ) {
                  jdph_i[1]=jdph_0+(phf+(phfshift-phishift)-phi)*P;
                  if ( jdph_i[1] > jdph[1][iew] ) {
                    jdph_i[1]=jdph[1][iew];
                  }
                  printf("%f\n",(phf+(phfshift-phishift)-phi));
                  printf("jdph_i = [%f,%f]\n",jdph_i[0],jdph_i[1]);
                }
                jdph_i[1]-=(preoh+ed)/86400.;
                // calcualte the phase at jdph_i[0]...
                if ( jdph_i[0] < jdph_i[1] ) {
                  if ( bdmoon ) sfmoon( jdph_i[0],jdph_i[1],moon,lat,longit,elev );
                  printint(
                    EWtext[iew],jdmid,
                    jdph_i[0],jdph_i[1],
                    compPhase(jdph_i[0],T0,P),compPhase(jdph_i[1],T0,P),
                    RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts
                  );
                }
                //n_orbits++;
                if ( bPhase ) {
                  jdph_0+=P;
                  printf("jdph_i = [%f,%f]\n",jdph_i[0],jdph_i[1]);
                } else {
                  jdph_0=jdph[1][iew]+1.;
                }
              }
              /*
                pht[0][iew]=compPhase(jdph_i[0],(T0+T0shift),P);
                pht[1][iew]=compPhase(jdph_i[1],(T0+T0shift),P);
      printf("%f  %f  %f  %f\n",ph[0][iew],ph[1][iew],pht[0][iew],pht[1][iew]);
                if ( pht[0][iew] < pht[1][iew] ) {
                  if (( pht[0][iew] <= (phf+phfshift) ) && ( pht[1][iew] >= (phi+phishift) )) {
                    jdph[0][iew]=jdph[0][iew]-(pht[0][iew]-MAX(pht[0][iew],(phi+phishift)))*P;
                    jdph[1][iew]=jdph[1][iew]-(pht[1][iew]-MIN(pht[1][iew],(phf+phfshift)))*P;
                    if ( bdmoon ) sfmoon( jdph[0][iew],jdph[1][iew],moon,lat,longit,elev );
                    printint(EWtext[iew],jdmid,jdph[0][iew],jdph[1][iew],compPhase(jdph[0][iew],T0,P),compPhase(jdph[1][iew],T0,P),RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);
                  }
                } else {
                  jdph[1][iew]=0.;
                  if (( pht[0][iew] <= (phf+phfshift) ) && ( 1. >= (phi+phishift) )) {
                    jdph[0][iew]=jdph[0][iew]-(pht[0][iew]-MAX(pht[0][iew],(phi+phishift)))*P;
                    jdph[1][iew]=jdph[0][iew]+(MIN(1.,(phf+phfshift))-MAX(pht[0][iew],(phi+phishift)))*P;
                    if ( bfpr ) {
                      if (( 0. <= (phf+phfshift) ) && ( pht[1][iew] >= (phi+phishift) )) {
                        jdph[1][iew]=jdph[1][iew]-(MAX(0.,(phi+phishift))-MIN(pht[1][iew],(phf+phfshift)))*P;
                      }
                    }
                    if ( bdmoon ) sfmoon( jdph[0][iew],jdph[1][iew],moon,lat,longit,elev );
                    printint(EWtext[iew],jdmid,jdph[0][iew],jdph[1][iew],compPhase(jdph[0][iew],T0,P),compPhase(jdph[1][iew],T0,P),RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);
                  }
                  if ( ( ! bfpr ) && ( 0. <= (phf+phfshift) ) && ( pht[1][iew] >= (phi+phishift) )) {
                    jdph[0][iew]=jdph[0][iew]+(1.-pht[0][iew])*P;
                    jdph[1][iew]=jdph[0][iew]-(MAX(0.,(phi+phishift))-MIN(pht[1][iew],(phf+phfshift)))*P;
                    if ( bdmoon ) sfmoon( jdph[0][iew],jdph[1][iew],moon,lat,longit,elev );
                    printint(EWtext[iew],jdmid,jdph[0][iew],jdph[1][iew],compPhase(jdph[0][iew],T0,P),compPhase(jdph[1][iew],T0,P),RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);
                  }
                }
              }
              */
            } else {
              if ( bdmoon ) sfmoon( jdph[0][iew],jdph[1][iew],moon,lat,longit,elev );
              if ( (jdph[0][iew]+(preoh+ed)/86400.) < jdph[1][iew] )
                if (( jdph[0][iew] < ssssr[0] ) || ( jdph[1][iew] > ssssr[1] )) {
                  printint("NS",jdmid,jdph[0][iew],jdph[1][iew],-1.,-1.,RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);                
                }
                printint(EWtext[iew],jdmid,jdph[0][iew],jdph[1][iew],-1.,-1.,RA,Dec,lat,longit,ed,preoh,postoh,label,moon,bPrintOpts);
            }
          }
          ha[0][0]=ha[0][0]-24.;
          ha[1][0]=ha[1][0]-24.;
        }
        ha[0][0]=ha[0][0]+48.;
        ha[1][0]=ha[1][0]+48.;
      }
    }
    if ( RA >= 0. ) {
      RA=tRA;Dec=tDec;
    }
    jd++;
  }
  exit (0);
}
