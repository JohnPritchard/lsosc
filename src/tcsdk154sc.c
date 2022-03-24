/*
  This is tcsdk154sc.c

  tcsdk154sc is a cut down version of dk154sc for use with the TCS
  at the Danish 154 telescope.

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

#include "liblsosc.h"

/*
  From /home/t2p2web/2p2T/D1p5M/RepsFinal/Final/DSZC/DSZC.tex.
  See Also http://www.ls.eso.org/lasilla/Telescopes/2p2T/D1p5M/RepsFinal/Final/DSZC.ps.gz

================================================================================
The DSZC program will

\begin{enumerate}

        \item Check that the values for proper motion and epoch are valid.

        \item Precess the coordinates to the current epoch, and correct
        the coordinates for proper motion.

        \item Calculate the hour angle of the target position.

        \item Determine which safety map (east or west of pier) is to
        be used.

\end{enumerate}


The DSZC program needs always to have a return code of 0 (else tcl/tk
will abort the GUI).  DSZC will print out a string, and these strings
will correspond to the following conditions:

\begin{itemize}

        \item {\bf --3} if the epoch is invalid.

        \item {\bf --2} if either of the proper motion values
        are invalid.

        \item {\bf --1} if the preset will put the telescope out of the
        safety zone.

        \item {\bf 0}  if the preset is possible, but the
        telescope is close to the western limit (the telescope
        will shortly be in the limits if it continues to
        track).

        \item {\bf 1}  if the telescope has first to preset to the
        zenith so as to avoid passing through an unsafe zone.

        \item {\bf 2}  if the telescope has first to preset to the
        zenith so as to avoid passing through an unsafe zone, and
        then will be close to the western limit after the final
        preset.

        \item {\bf 3} if the telescope can make the preset without
        passing through an unsafe zone and will not be close to the
        western limit.

\end{itemize}
================================================================================

We can add...

        \item {\bf --6} Current pointing is outside SaftyMap.

        \item {\bf --5} if the Dec is invalid.

        \item {\bf --4} if the RA is invalid.

*/


void usage( char ** argv )
{
  /*
    How this programme (should) work...
    1) You give it the <Current-RA> <Current-Dec> <Desired-RA> <Desired-Dec> <E/W>
    2) It outputs a simple OK if cordinates are currently within the Danish-1.54
      Safty Map, otherwise it outputs a diagnostic.
  */

  printf ("Usage : %s <Current-RA> <Current-Dec> <Desired-RA> <Desired-Dec> <Equinox> <Proper Motion RA> <Proper Motion Dec> <E/W>\n\
\n\
Note:\n\
 *)  Current RA & Dec should be specified in hh:mm:ss.s & sdd:mm:ss.s formats.\n\
 *)  Desired RA & Dec should be specified in hhmmss.s & ddmmss.s formats.\n\
 *)  Equinox yyyy.y\n\
 *)  Proper Motions n.n. Both should be in arcsec/year\n\
 *)  <E/W> should be either E or W.\n\
",argv[0]);
}

void
versioninfo( char **argv )
{
  printf("%s - Version 0.1, 2001-Aug-22\n\
\n\
By J.D.Pritchard <j.pritchard@eso.org>\n\
Based on the skycalc.c and skycalendar.c codes by John Thorstensen\n\
",argv[0]);
}

int main ( int argc, char **argv )
{

  /* Variables... */
  /* Lots  of unused (legacy) variables from dk154sc */
  int ed=0,preoh=0,postoh=0,y,m,d,j,icn,i,iew,hr,min,sec,emin,esec,icloser;
  double rsec;
  double jdmid,mjds,fd;
  double T0=0.,T0shift=0.,P=-1.0,phi=0.,phishift=0.,phf=1.,phfshift=0.,jdi,jdf,jdph[2][2],ha[2][3],jdha[2],sssr[2][2],ssssr[2];

  double maxX=100.,alt,az,minAlt=0.,haAtMinAlt[2],tha;
  double altBEoN=-13.0;
  double RA,Dec,tRA,tDec,Epoch,Equinox;
  const char *str;
  short  dow;
  struct date_time date,di,df;
  extern char *tzname[2];

  double jd,ph[2][2],pht[2][3],safezone[2][2];
  /*short bfpr=0,bCS=1,bCalcNow=1,bEphem=0,bError=0,bPhase=0,bQuiet=0; */
  short insz=2;
  short btrec=0;

  const char *telescope={"DK154"};
  const char *label={0};
  const char *EWtext[]={"EAST","WEST",0};


  /* Variables Specific to tcsdk154sc */
  const char *EorW;
  int exstat,iEorW,cwl;
  double HA,RAc,Decc,HAc,pma,pmd;
  /*
    Critical values taken from Safety Map...
    For HA we allow a 5min margin of safety.
    For Dec we all a 5 degree margin of safety.
  */
  double HACritical[4]={-0.450,0.400,(-0.450+5./60.),(0.400-5./60.)},DecCritical[2]={(-41.3+5.),(-46.8+5.)};

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

  /* Danish-1.54m:: */
  double longit = ((70.+44./60.+07.662/3600.)/180.*12.);
  double lat = (-29.-15./60.-14.235/3600.);
  double elev = 2340.00;

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

  /* ********************************************************************** */
  /* Command line arguments... */

  if ( argc != 9 ) {
    usage( argv );
    exit (8);
  }

  exstat=0;
  RAc=get_coorddes( argv[1] );
  /*
  RAc=atof( argv[1] );
  hr=RAc/10000.;
  min=(RAc-hr*10000.)/100.;
  rsec=RAc-hr*10000.-min*100.;
  RAc=hr+min/60.+rsec/3600.;
  if (( RAc < 0. ) || (RAc > 24.)) {
    exstat=exstat+16;
    printf("Error!!! Current RA must be between 000000. and 240000.\n");
  }
  */

  Decc=get_coorddes( argv[2] );
  /*
  Decc=atof( argv[2] );
  hr=Decc/10000.;
  min=(Decc-hr*10000.)/100.;
  rsec=Decc-hr*10000.-min*100.;
  Decc=hr+min/60.+rsec/3600.;
  if (( Decc < -90. ) || (Decc > 90.)) {
    exstat=exstat+32;
    printf("Error!!! Current Dec must be between -900000. and 900000.\n");
  }
  */

  RA=atof( argv[3] );
  hr=RA/10000.;
  min=(RA-hr*10000.)/100.;
  rsec=RA-hr*10000.-min*100.;
  RA=hr+min/60.+rsec/3600.;
  if (( RA < 0. ) || (RA > 24.)) {
    exstat=exstat+64;
    /* printf("Error!!! Desired RA must be between 000000. and 240000.\n"); */
    printf("-4");
    exit(0);
  }

  Dec=atof( argv[4] );
  hr=Dec/10000.;
  min=(Dec-hr*10000.)/100.;
  rsec=Dec-hr*10000.-min*100.;
  Dec=hr+min/60.+rsec/3600.;
  if (( Dec < -90. ) || (Dec > 90.)) {
    exstat=exstat+128;
    /* printf("Error!!! Desired Dec must be between -900000. and 900000.\n"); */
    printf("-5");
    exit(0);
  }

  Equinox=atof( argv[5] );
  /* Check validity of Equinox... */
  /*
  if ( ( Equinox < ????.? ) || ( Equinox > ????.? ) ) {
    printf("-3");
    exit(0);
  }
  */

  /* Proper Motion... */
  pma=atof( argv[6] )/(15. * cos(Dec / DEG_IN_RADIAN));  /* Conversion to secs per year */
  pmd=atof( argv[7] );
  /* Check validity of PMs... */
  /*
  if ( ( ( pma < ?.? ) || ( pma > ?.? ) ) || ( ( pmd < ?.? ) || ( pmd > ?.? ) ) ) {
    printf("-2");
    exit(0);
  }
  */

  EorW=argv[8];
  if (( EorW[0] != 'E' ) && ( EorW[0] != 'W' )) {
    exstat=exstat+256;
    /* printf("Error!!! Pier MUST be either E or W.\n"); */
  }

  if ( exstat > 0 ) {
    usage( argv );
    /* Apparently exit status must always be zero otherwise tcl/tk will abort the GUI.  */
    /* exit (exstat); */
    exit( 0 );
  }
  /* ********************************************************************** */

  /* Get Current */
  ct=time(NULL);
  lt=*localtime( &ct );
  ct=mktime( &lt );
  gmt=*gmtime( &ct );
  di.y=(1900+gmt.tm_year);di.mo=(gmt.tm_mon+1);di.d=gmt.tm_mday;di.h=gmt.tm_hour,di.mn=gmt.tm_min;di.s=gmt.tm_sec;
  jd=date_to_jd( di );

  tRA=RA;tDec=Dec;
  Epoch=2000.+(jd-J2000)/365.25;
  precrot(tRA,tDec,Equinox,Epoch,&RA,&Dec);
  RA  =  RA + pma * (Epoch-Equinox) / 3600.;
  Dec = Dec + pmd * (Epoch-Equinox) / 3600.;

  /* Are we EAST or WEST? */
  iEorW=0;
  if ( EorW[0] == 'W' ) iEorW=1;
  /* Hour Angle at current & Desired pointings */
  HAc=hainm12top12(jd,longit,RAc);
  HA=hainm12top12(jd,longit,RA);
  /* Calculate the SAFETY Map for CURRENT pointing and check for safety */
  calcSafty( telescope, Decc, safezone );
  if (( HAc < safezone[0][iEorW] ) || ( HAc > safezone[1][iEorW] )) {
  /* printf("Warning!!! CURRENT pointing outside Safety Zone.\n"); */
    printf("-6");
    exstat=exstat+1;
  } else {
  /* Calculate the SAFETY Map for DESIRED Cordinates and check for safety */
    calcSafty( telescope, Dec, safezone );
    if (( HA > safezone[0][iEorW] ) && ( HA < safezone[1][iEorW] )) {
    /* Check if DESIRED pointing is close to WESTERN limit... */
      cwl=0;
      if ( ( safezone[1][iEorW] - HA ) < 0.5 ) cwl=1;
      /* 
	 Check it is safe to move from CURRENT --> DESIRED pointings.
	 Here we assume the telescope will move in a straight line from CURRENT --> DESIRED pointings in HA-Dec space
	 RA and Dec drives slew at such a rate so that they are both moving for the same length time (more or less), hence the
	 above assumption seem sound.
	 Therefore we define "Critical" values of the HA and Dec for the two Pier sides...
	 Then the first check is to see if the CURRENT & DESIRED HAs are on opposite sides of HACritical
	 The second check is to see if the intersection of the line from CURRENT --> DESIRED with the line HA=HACritical is at a Dec
	 less than the DecCritical
	 If both of these conditions are true then the telescope will run into the safety zone whilst moving from the two independantly SAFE pointings.
	 *********************************************************************************
	 Since March 2004 DFOSC downgrade, this following check is no longer necessary...
	 *********************************************************************************
	 if (( ((HA-HACritical[iEorW])*(HAc-HACritical[iEorW])) < 0. ) && (((Dec-Decc)/(HA-HAc)*HACritical[2+iEorW]+Decc-(Dec-Decc)/(HA-HAc)*HAc) < DecCritical[iEorW]) ) {
	 //printf("Warning!!! Initial and Final pointings SAFE, but go VIA ZENITH\n");
	 exstat=exstat+2;
	 printf("%1.1d",(1+cwl));
	 } else {
	 //printf("OK\n");
	 printf("%1.1d",(3-cwl*3));
	 }
      */
      printf("%1.1d",(3-cwl*3));
    } else {
    /* printf("Warning!!! DESIRED pointing out of Safety Zone.\n"); */
      printf("-1");
      exstat=exstat+4;
    }
  }
  /* Apparently exit status must always be zero otherwise tcl/tk will abort the GUI.  */
  /* exit (exstat); */
  exit( 0 );
}
