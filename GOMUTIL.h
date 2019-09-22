#ifndef __GOMUTIL_h__
#define __GOMUTIL_h__


/*gibt das Vorzeichen einer Zahl als -1,0 oder 1 zurueck*/

integer sgn(integer x);

/*Gibt ein integer als string zurueck.*/

string intstr(integer i );


/*Gibt den kleineren wert zurueck.*/

integer min(integer a,integer b);


/*Gibt den kleineren Real-Wert zurueck.*/

real minr(real a,real b);


/*Gibt den groesseren wert zurueck.*/

integer max(integer a,integer b);
real maxr(real a,real b);

#endif
