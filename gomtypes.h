#ifndef __GOMTYPES_h__
#define __GOMTYPES_h__


#include "q5999219.h"


const integer minfeldsize = 5;
const integer maxfeldsize = 30;

/*Belegungsmoeglichkeiten fuer ein Feld*/

enum tstein {  leer,weiss,schwarz,tip, last_tstein};

/*Wie kann ein Spiel enden*/

enum tergebnis {sieg,unentschieden1, last_tergebnis};

/*Zustaende in denen sich ein Spiel befinden kann*/

enum tspielstate {intro,startvor,start,weissamzug,schwarzamzug,
               zeigsieg,ende,ende2,quitfrage,quit, last_tspielstate};

/*Die Groesse, die das Feld haben kann.*/

typedef unsigned char tfeldsize;
const int min_tfeldsize = minfeldsize;
const int max_tfeldsize = maxfeldsize;

/*Eine Koordinaten des Feldes*/

typedef unsigned char tfeldkoord;
const int min_tfeldkoord = 0;
const int max_tfeldkoord = maxfeldsize+1;

/*Das Feldarray, das die Belegung mit Steines speichert.*/

typedef matrix<min_tfeldkoord,max_tfeldkoord,min_tfeldkoord,max_tfeldkoord,tstein> tfeld;

/*Die beiden Typen von Spielern*/

enum tspieler {mensch,computer, last_tspieler};

/*Zuordnung von Spielertyp zu den Spielsteinfarben.*/

typedef array<0,last_tspielstate,tspieler> tzuordnung;

/*Position auf dem Feld*/

struct tfeldpos {
              integer x;   /*tFeldKoord;*/
              integer z;   /*tFeldKoord;*/
};

/*Urteil ueber das Spiel, wenn es zu ende ist*/

struct tbewertung {
                  tergebnis typ;
                  tstein stein;
};

/*Spielstrukturtyp, der den Zustand, in dem sich das Spiel befindet,
 zusammenfasst.*/

struct tspiel {
                  tfeld feld;
                  integer feldsizex;    /*tFeldSize;*/
                  integer feldsizey;    /*tFeldSize;*/
                  tspielstate spielstate;
                  tstatus status;
                  tzuordnung zuordnung;
                  tbewertung bewertung;
                  integer spielstufe;
                  tfeldpos neugesetzt;
};


#ifdef __gomtypes_implementation__
#undef EXTERN
#define EXTERN
#endif

/*Zuordnung Spielzustand -> Steinfarbe*/
EXTERN array<0,last_tspielstate,tstein> stein;
/*Der jeweilige Gegenspieler*/
EXTERN array<0,last_tspielstate,tspielstate> gegenspieler;
#undef EXTERN
#define EXTERN extern


#endif
