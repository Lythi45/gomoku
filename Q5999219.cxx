#include "ptoc.h"

/* Gomoku-Computerspieler von Sabine Wolf, q5999219@bonsai.fernuni-hagen.de, Version vom 7.9.02 */

#define __q5999219_implementation__


#include "Q5999219.h"


const integer minfeldsize = 5;
const integer maxfeldsize = 30;
const integer deffeldsize = 13;

/*Die maximale Anzahl potentieller ZÅge, unter denen ausgewÑhlt wird.*/
const integer maxauswahl = 100;

/*Die ZustÑnde die ein Feld haben kann.*/

enum tstein {leer,weiss,schwarz,rand1, last_tstein};

/*Die Groesse, die das Feld haben kann.*/

typedef unsigned char tfeldsize;
const int min_tfeldsize = minfeldsize;
const int max_tfeldsize = maxfeldsize;

/*Eine Koordinate des Feldes*/

typedef unsigned char tfeldkoord;
const int min_tfeldkoord = 0;
const int max_tfeldkoord = maxfeldsize+1;

/*Das Feldarray, das die Belegung des Spielfeldes mit Steinen speichert.*/

typedef matrix<min_tfeldkoord,max_tfeldkoord,min_tfeldkoord,max_tfeldkoord,tstein> tfeld;

/*Ein Zug mit seiner Position und seinem Status.*/
struct tzug {
          tfeldkoord x;
          tfeldkoord y;
          tstatus state;
};
/*Array zur Speicherung der ZÅge*/
typedef array<0,maxfeldsize*maxfeldsize,tzug> tazug;
typedef tazug* tzazug;

typedef signed char offset;
const int min_offset = -1;
const int max_offset = 1;

struct trichtung {
               offset x;
               offset y;
};

struct tfeldpos {
              tfeldkoord x;
              tfeldkoord y;
};
struct tbewertung {
              tfeldkoord x;
              tfeldkoord y;
              longint bewe;
};

/*In solchen Arrays werden mîgliche Zugkandidaten gespeichert.*/

typedef array<0,maxauswahl,tbewertung> tsbewertunga;
typedef array<0,maxfeldsize*maxfeldsize,tbewertung> tbewertunga;

/*Die 4 mîglichen Grundrichtungen*/

enum tausrichtung {waagerecht,senkrecht,vorschraeg,rueckschraeg, last_tausrichtung};

/*In diesem Array tragen sich Steine in ihre Umgebung ein, damit jedes Feld Åber seine
Umgebung Bescheid wei·.*/
matrix<min_tfeldkoord,max_tfeldkoord,min_tfeldkoord,max_tfeldkoord,0,last_tausrichtung,0,1,integer> umgebung;

/*Zweierpotenzen fÅr die 4 Richtungen.*/
array<0,last_tausrichtung,word> pfak;

/*In dieses Array tragen sich Steine in ihre nÑhere Nachbarschaft ein, damit ein Feld schnell
wei·, ob es in der Nachbarschaft eines Steines liegt.*/
matrix<min_tfeldkoord,max_tfeldkoord,min_tfeldkoord,max_tfeldkoord,word> nachbarn;

/*Die Nummer des aktuellen Zuges und die Anzahl des gespeicherten ZÅge.
Im normalen Spiel sind beide Werte gleich, nur nach ZugrÅcknahme ist aktZug<endZug.*/
integer aktzug,endzug;

/*Das Array, das die ZÅge speichert.*/
tzazug zug;

tfeldsize feldsizex,feldsizey;
/*Das Spielfeldarray, in dem die Steine eingetragen werden.*/
tfeld feld;

/*EINS_AM_ZUG->Weiss,ZWEI_AM_ZUG->Schwarz*/
array<0,last_tstatus,tstein> steinzuordnung;

/*EINS_AM_ZUG->ZWEI_AM_ZUG,ZWEI_AM_ZUG->EINS_AM_ZUG*/
array<0,last_tstatus,tstatus> flip;

unsigned char spielstufe;

array<0,last_tausrichtung,trichtung> rv;
tfeldpos anfangspos,endpos;

/*Setzt einen Stein in das Feldarray und addiert den um die Entfernung mal 2
nach links geshifteten Steinwert(1 fÅr Wei·,2 fÅr Schwarz) auf die entsprechenden
Umgebungswerte der in alle 8 Richtungen liegenden Nachbarn bis zu einer Entfernung
von 5 Feldern. Zur schnellen öberprÅfung, ob ein Stein in der nÑheren Nachbarschaft
eines anderen liegt, wird der Stein noch ohne Unterscheidung der Steinfarbe
auf den Nachbarschaftswert der nÑheren Nachbarn addiert, jede der 8 Richtungen hat
eine entsprechende Bitposition dafÅr, fÅr die unmittelbaren Nachbarn wird das ganze nochmal um 8 Bits nach links geschoben,
so da· 16 Bits fÅr 16 Nachbarn verwendet werden.
*/

void eintragen(tfeldkoord x,tfeldkoord y,tstein stein)
{
   integer xx,yy,t,val;
   tausrichtung ausr;


   val=ord(stein);
   feld[x][y]=stein;
   if (stein<rand1) 
      for( ausr=waagerecht; ausr <= rueckschraeg; ausr = succ(tausrichtung,ausr))
      {
         nachbarn[x-rv[ausr].x][y-rv[ausr].y] += pfak[ausr]*256;
         if ((x-rv[ausr].x*2>0) && (y-rv[ausr].y*2>0) &&
            (x-rv[ausr].x*2<=feldsizex)) 
            nachbarn[x-rv[ausr].x*2][y-rv[ausr].y*2] += pfak[ausr];
         nachbarn[x+rv[ausr].x][y+rv[ausr].y] += pfak[ausr]*512;
         if ((x+rv[ausr].x*2>0) &&
            (x+rv[ausr].x*2<=feldsizex) && (y+rv[ausr].y*2<=feldsizey)) 
            nachbarn[x+rv[ausr].x*2][y+rv[ausr].y*2] += pfak[ausr]*2;
      }
   for( t=1; t <= 5; t ++)
   {
      for( ausr=waagerecht; ausr <= rueckschraeg; ausr = succ(tausrichtung,ausr))
      {
         xx=x+rv[ausr].x*t;
         yy=y+rv[ausr].y*t;
         if ((xx>0) && (yy>0) && (xx<=feldsizex) && (yy<=feldsizey)) 
            umgebung[xx][yy][ausr][0] += val;
         xx=x-rv[ausr].x*t;
         yy=y-rv[ausr].y*t;
         if ((xx>0) && (yy>0) && (xx<=feldsizex) && (yy<=feldsizey))
            umgebung[xx][yy][ausr][1] += val;
      }
/*Der Steinwert wird um 2 Bits nach links geschoben.*/
      val=val*4;
   }
}



/*Diese Prozedur trÑgt einen Stein wieder aus und macht alle von eintragen
vorgenommenen énderungen wieder rÅckgÑngig.*/

void austragen(tfeldkoord x,tfeldkoord y,tstein stein)
{
   integer xx,yy,t,val;
   tausrichtung ausr;


   val=ord(stein);
   feld[x][y]=leer;
   if (stein<rand1) 
      for( ausr=waagerecht; ausr <= rueckschraeg; ausr = succ(tausrichtung,ausr))
      {
         nachbarn[x-rv[ausr].x][y-rv[ausr].y] -= pfak[ausr]*256;
         if ((x-rv[ausr].x*2>0) && (y-rv[ausr].y*2>0) &&
            (x-rv[ausr].x*2<=feldsizex)) 
            nachbarn[x-rv[ausr].x*2][y-rv[ausr].y*2] -= pfak[ausr];
         nachbarn[x+rv[ausr].x][y+rv[ausr].y] -= pfak[ausr]*512;
         if ((x+rv[ausr].x*2>0) &&
            (x+rv[ausr].x*2<=feldsizex) && (y+rv[ausr].y*2<=feldsizey)) 
            nachbarn[x+rv[ausr].x*2][y+rv[ausr].y*2] -= pfak[ausr]*2;
      }

   for( t=1; t <= 5; t ++)
   {
      for( ausr=waagerecht; ausr <= rueckschraeg; ausr = succ(tausrichtung,ausr))
      {
         xx=x+rv[ausr].x*t;
         yy=y+rv[ausr].y*t;
         if  ((xx>0) && (yy>0)&& (xx<=feldsizex) && (yy<=feldsizey)) 
            umgebung[xx][yy][ausr][0] -= val;
         xx=x-rv[ausr].x*t;
         yy=y-rv[ausr].y*t;
         if ((xx>0) && (yy>0) &&  (xx<=feldsizex) && (yy<=feldsizey))
            umgebung[xx][yy][ausr][1] -= val;
      }
      val=val*4;
   }
}




void gibstatus ( tstatus& status )

{;
   status = (*zug)[aktzug].state;
}



/*Initialisiert fÅr ein neues Spiel das Feld-, das Nachbarschaft- und
das Umgebungsarray, trÑgt den Rand in das Umgebungsarray ein,
setzt das Zugarray zurÅck und den Spielstatus auf EINS_AM_ZUG.*/

void neuesspiel(boolean& ok)

{
   tfeldkoord x,y;
   tausrichtung ausr;
;
   for( x= 0; x <= feldsizex+1; x ++)
      for( y = 0; y <= feldsizey+1; y ++)
      {
         feld[x][y]=leer;
         nachbarn[x][y]=0;
         for( ausr=waagerecht; ausr <= rueckschraeg; ausr = succ(tausrichtung,ausr))
         {
            umgebung[x][y][ausr][0]=0;
            umgebung[x][y][ausr][1]=0;
         }
      }
   for( x= 0; x <= feldsizex+1; x ++)
      for( y = 0; y <= feldsizey+1; y ++)
      {
         if ((x==0) || (x==feldsizex+1) || (y==0) || (y==feldsizey+1)) 
            eintragen(x,y,rand1);
      }
   aktzug=0;
   endzug=0;
   (*zug)[aktzug].state = eins_am_zug;
   ok=true;
}



/*Setzt einen Stein und schaut, ob sich durch das Setzen ein Sieg oder ein
Unentschieden ergeben hat.*/

void setzestein ( integer x, integer y, boolean& ok )
{
   tausrichtung ausr;
   integer shift;

;

if ((((*zug)[aktzug].state==eins_am_zug) || ((*zug)[aktzug].state==zwei_am_zug)) &&
   (x>0) && (x<=feldsizex) &&
   (y>0) && (y<=feldsizey)) 
{
   if (feld[x][y]==leer) 
   {
   (*zug)[aktzug].x = x;
   (*zug)[aktzug].y = y;
   eintragen(x,y,steinzuordnung[(*zug)[aktzug].state]);
   (*zug)[aktzug+1].state = flip[(*zug)[aktzug].state];
   aktzug += 1;
   endzug = aktzug;

/*Wenn das Spielfeld voll ist, gibt es ein Unentschieden, es sei denn, es wird
noch ein Sieg festgestellt.*/

   if (aktzug==feldsizex*feldsizey) 
      (*zug)[aktzug].state=unentschieden;

/*Es wird geschaut, ob sich durch das Setzen des Steins eine FÅnfer-Reihe ergeben hat.
Die der gesetzte Stein am Anfang, dazwischen oder am Ende einer solchen Reihe stehen kann,
werden alle 5 Positionen abgefragt.*/

   for( ausr = waagerecht; ausr <= rueckschraeg; ausr = succ(tausrichtung,ausr))
   {
      shift=0;
      if ((umgebung[x][y][ausr][1] & 255)==85*ord(feld[x][y])) 
         shift=1;
      if   (((umgebung[x][y][ausr][1] & 63)==21*ord(feld[x][y])) &&
            ((umgebung[x][y][ausr][0] & 3)==ord(feld[x][y])))    
         shift=2;
      if   (((umgebung[x][y][ausr][1] & 15)==5*ord(feld[x][y])) &&
            ((umgebung[x][y][ausr][0] & 15)==5*ord(feld[x][y])))     
         shift=3;
      if   (((umgebung[x][y][ausr][1] & 3)==ord(feld[x][y])) &&
               ((umgebung[x][y][ausr][0] & 63)==21*ord(feld[x][y])))   
            shift=4;
      if  ((umgebung[x][y][ausr][0] & 255)==85*ord(feld[x][y])) 
               shift=5;
      if (shift>0) 
      {
         if (feld[x][y]==weiss) 
            (*zug)[aktzug].state= eins_gewinnt;
         else
            (*zug)[aktzug].state= zwei_gewinnt;

/*Je nach Positon des aktuellen Steines in der Siegreihe wird der Anfangs- und
der Endstein gesetzt.*/

         anfangspos.x=x-rv[ausr].x*(shift-1);
         anfangspos.y=y-rv[ausr].y*(shift-1);
         endpos.x=x+rv[ausr].x*(5-shift);
         endpos.y=y+rv[ausr].y*(5-shift);
      } 
   }
   ok=true;
   }
   else
   ok=false;
   }
   else
   ok=false;
}



/*Der Computer ermittelt den fÅr in gÅnstigsten Stein und setzt diesen. Dabei geht er
je nach Spielstufe mehr oder weniger intelligent vor.*/

void computerzug ( integer& px, integer& py, boolean& ok);

static integer auswahl,ost;

static longint qual;

static tbewertunga nzuege;


/*BesterZug ermittelt den besten Zug, bekommt dafÅr mitgeteilt,
bis zu welcher Tiefe ZÅge kalkuliert werden dÅrfen,,welche Farbe
der Computer und welche der Gegner hat,wie hoch die
Bewertungsschwelle ist, ab der gute Felder als solche erkannt und
wie stark zu verlÑngernde gegenÅber zu blockierenden Ketten gewichtet
werden sollen.*/

static longint besterzug (integer& px,integer& py,
                        integer zugtiefe,
                        tstein cpst,tstein spst,
                        integer schwelle,
                        integer gewicht )

{
    integer sumbewe,bewe,faktor,ri;
   tstein st;
   tausrichtung ausr;
   tfeldkoord x,y,siegx,siegy;
   integer xx,yy,xxx,yyy,t,
   nausw,nnach,sieg,ssieg;
   tsbewertunga zuege;
   boolean kett3,block3,block4;
   integer skett1,sblock1,
   skett2,sblock2,
   skett3,skett3b,
   sblock3,sblock3b,
   sblock4;
   longint maxqual;


   longint besterzug_result;
   nnach=0;
   sieg=0;
   siegx=1;
   siegy=1;
   sblock3=0;
   skett3=0;
   sblock4=0;
   for( y=1; y <= feldsizey; y ++)
      for( x=1; x <= feldsizex; x ++)
         if ((feld[x][y]==leer) && (nachbarn[x][y]>0) && (sieg<4000)) 

/*Wenn ein Feld leer ist und in der Umgebung eines anderen Steines liegt
(direkt drann oder mit einem Feld Abstand), wird es nÑher untersucht, ob
sich eigene Ketten bilden oder gegnerische Ketten abblocken lassen. Eine
gegebene Kette zu verlÑngern zÑhlt das dreifache wie das Abblocken einer
solchen Kette. LÑngere Ketten geben mehr Punkte, vîllig freie Ketten mehr
als die, die mit einem Stein schon geblockt werden kînnen.*/

         {
            sumbewe=0;
            block3=false;
            kett3=false;
            block4=false;
            skett3b=0;
            sblock3b=0;
            skett1=0;
            sblock1=0;
            skett2=0;
            sblock2=0;
            for( st=weiss; st <= schwarz; st = succ(tstein,st))
            {
               ost=ord(st);
               if (st==cpst) 
                  faktor=gewicht;
               else
                  faktor=1;
               for( ausr = waagerecht; ausr <= rueckschraeg; ausr = succ(tausrichtung,ausr))
                  for( ri=0; ri <= 1; ri ++)
                  {
                     bewe=0;

        /*oo?..*/      if (((umgebung[x][y][ausr][ri] & 15) == 5*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == 0)) 
                        bewe=1;
        /*oo.?.*/      if (((umgebung[x][y][ausr][ri] & 63) == 20*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == 0)) 
                        bewe=1;
        /*oo..?*/      if ((umgebung[x][y][ausr][ri] & 255) == 80*ost) 
                        bewe=1;

        /*...o?.*/     if (((umgebung[x][y][ausr][ri] & 255) == ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == 0)) 
                        bewe=1;
        /*..o?..*/     if (((umgebung[x][y][ausr][ri] & 63) == ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == 0)) 
                        bewe=1;
        /*.o?...*/     if (((umgebung[x][y][ausr][ri] & 15) == ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 63) == 0)) 
                        bewe=1;
        /*.o.?..*/     if (((umgebung[x][y][ausr][ri] & 63) == 4*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == 0)) 
                        bewe=1;
        /*.o..?.*/     if (((umgebung[x][y][ausr][ri] & 255) == 16*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == 0)) 
                        bewe=1;

        /*.oo?..*/     if (((umgebung[x][y][ausr][ri] & 63) == 5*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == 0)) 
                        bewe=9;
        /*..oo?.*/     if (((umgebung[x][y][ausr][ri] & 255) == 5*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == 0)) 
                        bewe=9;
        /*.oo.?.*/     if (((umgebung[x][y][ausr][ri] & 255) == 20*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == 0)) 
                        bewe=9;
        /*.o.o?.*/     if (((umgebung[x][y][ausr][ri] & 255) == 17*ost) &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == 0))
                        bewe=9;
        /*..o?o.*/     if (((umgebung[x][y][ausr][ri] & 63) == ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == ost))
                        bewe=9;
        /*..o?o..*/    if (((umgebung[x][y][ausr][ri] & 63) == ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 63) == ost))
                     {     /*wegen Symmetrie nur in einer Richtung werten.*/
                       if (ri==0)  bewe=9; else bewe=0;
                     }
        /*.o.?o.*/     if (((umgebung[x][y][ausr][ri] & 63) == 4*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == ost))
                        bewe=9;
        /*o.o?o*/	     if (((umgebung[x][y][ausr][ri] & 63) == 17*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == ost)) 
                        bewe=9*5;
        /*.o.o?o*/     if (((umgebung[x][y][ausr][ri] & 255) == 17*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == ost)) 
                        bewe=0;
        /*.o?o.o*/     if (((umgebung[x][y][ausr][ri] & 15) == ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 63) == 17*ost)) 
                        bewe=9*5;
        /*.ooo?*/	     if ((umgebung[x][y][ausr][ri] & 255) == 21*ost)  
                        bewe=9*5;
        /*ooo?.*/	     if (((umgebung[x][y][ausr][ri]  & 63) == 21*ost) &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == 0)) 
                        bewe=9*5;
        /*ooo.?*/	     if ((umgebung[x][y][ausr][ri]  & 255) == 84*ost) 
                        bewe=9*5;
        /*oo?o.*/	     if (((umgebung[x][y][ausr][ri] & 15) == 5*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == ost)) 
                        bewe=9*5;
        /*oo?.o*/	     if (((umgebung[x][y][ausr][ri] & 15) == 5*ost) &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == 4*ost))
                        bewe=9*5;
        /*oo.?o*/	     if (((umgebung[x][y][ausr][ri] & 63) == 20*ost)  &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == ost))
                        bewe=9*5;
        /*.ooo.?*/     if ((umgebung[x][y][ausr][ri]  & 1023) == 84*ost) 
                        bewe=0;

        /*o.o?o.o*/    if (((umgebung[x][y][ausr][ri] & 63) == 17*ost) &&
                        ((umgebung[x][y][ausr][1-ri] & 63) == 17*ost)) 
                        bewe=81;
        /*o.oo?.o*/    if (((umgebung[x][y][ausr][ri] & 255) == 69*ost) &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == 4*ost)) 
                        bewe=81;
        /*.ooo?.*/     if (((umgebung[x][y][ausr][ri] & 255) == 21*ost) &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == 0)) 
                        bewe=81;
        /*.oo?o.*/     if (((umgebung[x][y][ausr][ri] & 63) == 5*ost) &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == ost)) 
                        bewe=82;
        /*folgende 2 Positionen sind nur zum Blocken.*/
        /*.oo.o?*/     if (((umgebung[x][y][ausr][ri] & 1023) == 81*ost) &&
                        (st!=cpst)) 
                        bewe=81;
        /*.o.oo?*/     if (((umgebung[x][y][ausr][ri] & 1023) == 69*ost) &&
                        (st!=cpst)) 
                        bewe=81;
        /*oooo?*/	     if ((umgebung[x][y][ausr][ri] & 255) == 85*ost) 
                        bewe=1000;
        /*ooo?o*/	     if (((umgebung[x][y][ausr][ri] & 15) == 5*ost) &&
                        ((umgebung[x][y][ausr][1-ri] & 15) == 5*ost)) 
                        bewe=1000;
        /*oo?oo*/	     if (((umgebung[x][y][ausr][ri] & 63) == 21*ost) &&
                        ((umgebung[x][y][ausr][1-ri] & 3) == ost)) 
                        bewe=1000;

        /*.oooo?*/     if (((umgebung[x][y][ausr][ri] & 1023) == 85*ost) && (st!=cpst) && (sieg!=3000)) 
                     {
                       sieg=-4000;
                       siegx=x;
                       siegy=y;
                     }

        /*Wenn eine 4er-Kette verlÑngert werden kann,
         bedeutet das den sofortigen Sieg, Blocken ist zwingend.*/
                     if (bewe==1000) 
                     {
                       nnach=0;
                       if (st==cpst) 
                       {
                         sieg=4000;
                         siegx=x;
                         siegy=y;
                       }
                       else
                         block4=true;
                     }
         /*Eine freie Dreierreihe mu· geblockt oder verlÑngert werden.*/
                     if (bewe>=81) 
                     {
                       if (st==cpst) 
                         kett3=true;
                       else
                         block3=true;
                     }

         /*Halboffene 3er-Ketten werden gezÑhlt.*/
                     if (bewe==9*5) 
                     {
                       if (st==cpst) 
                         skett3b += 1;
                       else
                         sblock3b += 1;
                     }

         /*Offene 2er-Ketten werden gezÑhlt.*/
                     if (bewe==9) 
                     {
                       if (st==cpst) 
                         skett2 += 1;
                       else
                         sblock2 += 1;
                     }
         /*Offene einzelne Steine werden gezÑhlt.*/
                     if (bewe==1) 
                     {
                       if (st==cpst) 
                         skett1 += 1;
                       else
                         sblock1 += 1;
                     }
                     sumbewe += bewe*faktor;
                 }
              }

              if (sieg<4000) 
                {

                ssieg=0;
                if ((sblock3b>1) || ((sblock3b>0) && (sblock2>0))) 
                {
                   block3=true;
                   sblock3 += 1;
                }

       /*ZÑhle zu verlÑngernde 3er-Kettenenden.*/
              if (kett3) 
              {
                 skett3 += 1;

       /*Wenn die besten ZÅge eine freie 3er-Kette verlÑngern, bedeutet das auch den Sieg.*/
                 if ((sieg>=-3000) && (skett3>0)) 
                    ssieg=3000;
              }
              if ((skett1>1) && (sumbewe<9*faktor))
                 sumbewe += 8*faktor;
              if ((sblock1>1) && (sumbewe<9)) 
                 sumbewe += 8;

       /*ZÑhle zu blockierende 3er-Kettenenden.*/
              if (block3) 
              {
                 sblock3 += 1;
       /*Wenn es zwei zu blockende 3er-Ketten gibt, es also mehr als 3 Enden gibt,
        die zu blockieren sind, bedeutet das die Niederlage.*/
                 if ((sieg==0) && (sblock3>3)) 
                    ssieg=-3000;

              }

       /*ZÑhle zu blockierende 4er-Kettenenden.*/
              if (block4) 
              {
                 sblock4 += 1;
       /*Wenn es mehr als ein zu blockierendes 4er-Ketten-Ende gibt,
        so bedeutet da· eine Niederlage.*/
                 if (sblock4>1) 
                    ssieg=-4000;
              }

       /*Wenn ein Stein 2 offene Zweierketten oder halboffene 3er-Ketten verlÑngert, so
       bedeutet das den Sieg, wenn es nichts mit hîherer PrioritÑt gibt.*/
              if ((sieg==0) && (skett2+skett3b>1) && ((spielstufe==2) || (skett3b>0))) 
                 ssieg=2000;

       /*Wenn ein Stein 2 halboffene 3er-Ketten verlÑngert, bedeutet auch das den Sieg.*/
              if ((sieg>=-3000) && (skett3b>1)) 
                 ssieg=3000;

              if ((ssieg!=0) && (sblock4==0)) 
              {
                 sieg=ssieg;
                 siegx=x;
                 siegy=y;
              }
            }

            if ((sblock2>1) && (sumbewe<81) && (spielstufe==2)) 
               sumbewe += 9*4;

            if ((block3) && (sumbewe<81) && (spielstufe==1)) 
               sumbewe += 9*4;

            if (((sblock3>0) && (sieg==2000)) || ((sblock4>0) && (sieg==3000))) 
              sieg=0;

       /*Wenn die Bewertung eines Feldes >= der momentanen Schwelle ist, wird des Feld in
        die Liste der mîglichen ZÅge eingetragen.*/
            if (sumbewe>=schwelle) 
            {
        /*Wenn die Rekursionstiefe erreicht ist, werden nur die besten ZÅge eingetragen.*/
               if (zugtiefe==0) 
               {
                  if (sumbewe>schwelle) 
                  {
                     nnach=0;
                     schwelle=sumbewe;
                  }
               }
         /*Ansonsten werden ZÅge einer bestimmten Gruppe eingetragen. Wenn es z.B. einen
          Feld mit einer Bewertung von mind. 81 gibt, hei·t das, das dieses Feld unbedingt
          geblockt werden mu·, Felder mit einer niedrigeren Bewertung sind damit irrelevant.*/
               else
               {
                  while (sumbewe>=schwelle*9) 
                  {
                     nnach=0;
                     schwelle=schwelle*9;
                  }
               }

               if (nnach<maxauswahl) 
               {
                  zuege[nnach].x=x;
                  zuege[nnach].y=y;
                  zuege[nnach].bewe=sumbewe;
               }
               else
               {
               /*Reicht der Platz im Array nicht aus, wird ein Feld Åberschrieben mit
                einer gewissen Wahrscheinlichkeit.*/
                 auswahl=Random(nnach);
                 if (auswahl<maxauswahl) 
                 {
                   zuege[auswahl].x=x;
                   zuege[auswahl].y=y;
                   zuege[auswahl].bewe=sumbewe;
                 }
               }
               nnach += 1;
            }

         }
/*Wurden keine guten Felder gefunden, so wird zufÑllig ein Feld ausgewÑhlt,
das in der Nachbarschaft eines Steines liegt.*/

   if (nnach==0)  
   {
      for( y=1; y <= feldsizey; y ++)
         for( x=1; x <= feldsizex; x ++)
            if ((feld[x][y]==leer) && (nachbarn[x][y]>255)) 
                     {
                        nzuege[nnach].x=x;
                        nzuege[nnach].y=y;
                        nnach += 1;
                     }
      auswahl=Random(nnach);
      px=nzuege[auswahl].x;
      py=nzuege[auswahl].y;
      besterzug_result=0;
   }
   else
   {
/*Wenn die Stellung Sieg oder Niederlage bedeutet, wird ein sehr hoher Wert
 fÅr Sieg und ein sehr niedriger Wert fÅr Niederlage zurÅckgegeben.*/
      if (sieg!=0) 
      {
         px=siegx;
         py=siegy;
         besterzug_result=sieg;
      }
      else
/*Wenn die Zugtiefe erreicht ist, wird zufÑllig ein Feld aus der Liste
ausgewÑhlt, da alle diese Felder gleich gut bewertet wurden.*/
         if (zugtiefe==0) 
         {
            auswahl=Random(nnach);
            px=zuege[auswahl].x;
            py=zuege[auswahl].y;
            besterzug_result=zuege[auswahl].bewe;
         }
         else
/*Ansonsten wird jedes Feld aus der Liste erstmal gesetzt und nach dem besten Zug
gesucht, mit dem die andere Steinfarbe auf diesen Zug reagieren kînnte.
Die Felder, die in der Summe aus der zweifachen eigenen Bewertung minus der Bewertung
aus der besten mîglichen gegnerischen Reaktion darauf, am besten abschneiden werden genommen.
Der Faktor zwei ergab sich empirisch beim Spielen der Stufe 2 gegen Stufe 0.
*/
         {
            maxqual=-5000;
            nausw=0;

            for( t=0; t <= nnach-1; t ++)
            {
               xx=zuege[t].x;
               yy=zuege[t].y;
               eintragen(xx,yy,cpst);
               qual=zuege[t].bewe*longint(2)*longint(zugtiefe)-besterzug(xxx,yyy,zugtiefe-1,spst,cpst,1,3);
               if (qual>=maxqual) 
               {
                  if (qual>maxqual) 
                  {
                     maxqual=qual;
                     nausw=0;
                  }
                  zuege[nausw].x=xx;
                  zuege[nausw].y=yy;
                  nausw += 1;
               }

/* Der spekulativ gesetze Stein wird wieder zurÅckgenommen.*/
               austragen(xx,yy,cpst);

            }

/*Aus den besten Feldern wird einer ausgewÑhlt.*/
            auswahl=Random(nausw);
            px=zuege[auswahl].x;
            py=zuege[auswahl].y;
            besterzug_result=maxqual;
         }
     }

   return besterzug_result;
}

void computerzug ( integer& px, integer& py, boolean& ok)

{
   tstein computerstein,spielerstein;
   tstatus state;

;

   state =  (*zug)[aktzug].state;

/*Nur wenn das Spiel lÑuft, kann der Computer einen Zug machen.*/

   if ((state!=eins_am_zug) && (state!=zwei_am_zug)) 
      ok = false;
   else
   {
      ok=true;

      computerstein = steinzuordnung[state];
      spielerstein  = steinzuordnung[flip[state]];

/*Beim ersten Zug wird fÅr die Spielstufen 0 und 1 ein zufÑlliges Feld
ausgewÑhlt, fÅr die Stufe zwei eines in der Mitte.*/

      if (aktzug==0) 
      {
         if (spielstufe<2) 
         {
            px = Random(feldsizex)+1;
            py = Random(feldsizey)+1;
         }
         else
         {
            px = (feldsizex - Random(2)) / 2 + 1;
            py = (feldsizey - Random(2)) / 2 + 1;
         }
      }
      else
      {
         switch (spielstufe) {

/*Nur Viererketten werden erkannt.*/
           case 0 : qual=besterzug(px,py,0,computerstein,spielerstein,1000,2); break;

/*Dreier und Viererketten werden erkannt.*/
           case 1 : qual=besterzug(px,py,2,computerstein,spielerstein,81,2); break;

/*auch kuerzere Ketten werden bewertet.*/
           case 2 : qual=besterzug(px,py,2,computerstein,spielerstein,1,3); break;

         }
      }

/*Der ausgewÑhlte Stein wird gesetzt.*/
      setzestein(px,py,ok);

   }
}



/*zieht einen Zug zurÅck, indem der vorherige Zug zum aktuellen gemacht wird
und dessen Stein zurÅckgenommen wird. Ist der aktuelle Zug allerdings der erste,
wird ok wird ok auf false gesetzt.x und y wird auf die Position des zurÅckgenommenen
Steines gesetzt.*/

void zugzurueck ( integer& x, integer& y, boolean& ok )

{;
   if (aktzug == 0) 
      ok = false;
   else
   {
      ok = true;
      aktzug -= 1;
      x = (*zug)[aktzug].x;
      y = (*zug)[aktzug].y;
      austragen(x,y,feld[x][y]);

   }
}




/*zieht einen Zug vor, indem der aktuelle Stein wieder auf das Feld gesetzt
wird und der nÑchste Zug zum aktuellen gemacht wird. Ist der aktuelle Zug
allerdings schon der letzte, wird ok auf false gesetzt. x und y werden auf
die Position des gesetzten Steines gesetzt.*/

void zugvor ( integer& x, integer& y, boolean& ok )

{;
   if (aktzug == endzug) 
      ok = false;
   else
   {
      ok = true;
      x = (*zug)[aktzug].x;
      y = (*zug)[aktzug].y;
      eintragen(x,y,steinzuordnung[(*zug)[aktzug].state]);
      aktzug += 1;

   }
}

/*Setzt die Spielstufe.*/

void setzespielstufe ( integer stufe, boolean& ok )

{;
   if ((stufe<0) || (stufe>2)) 
      ok = false;
   else
   {
      ok = true;
      spielstufe=stufe;
   }
}



/*Gibt die Spielstufe zurÅck.*/

void gibspielstufe ( integer& stufe )

{;
   stufe=spielstufe;
}



/*Setzt die Feldgrî·e.*/

void setzefeldgroesse ( integer x, integer y, boolean& ok )

{;
   if ((x<minfeldsize) || (x>maxfeldsize) || (y<minfeldsize) || (y>maxfeldsize)) 
      ok = false;
   else
   {
      ok = true;
      feldsizex=x;
      feldsizey=y;
      aktzug=0;
      endzug=0;
      (*zug)[aktzug].state = undefiniert;

   }
}



/*Gibt die Feldgrî·e zurÅck.*/

void gibfeldgroesse ( integer& x, integer& y )

{;
   x=feldsizex;
   y=feldsizey;
}



/*Gibt die Gewinnreihe zurÅck.*/

void gibgewinnreihe (integer& x1, integer& y1,
                              integer& x2, integer& y2, boolean& ok)

{;
if (((*zug)[aktzug].state==eins_gewinnt) || ((*zug)[aktzug].state==zwei_gewinnt)) 
   {
   x1=anfangspos.x;
   y1=anfangspos.y;
   x2=endpos.x;
   y2=endpos.y;
   ok = true;
   }
else
   ok = false;
}



/*Gibt meinen Namen zurÅck.*/

void gibnamen ( string& name )

{;
   name = "Sabine Wolf";
}



/*Gibt den Spitznamen des Computerspielers zurÅck.*/

void gibspitznamen ( string& name )

{;
   name="Neck-Breaker";
}

/*Variablen werden initialisiert und auf Defaultwerte gesetzt.*/

class unit_q5999219_initialize {
  public: unit_q5999219_initialize();
};
static unit_q5999219_initialize q5999219_constructor;

unit_q5999219_initialize::unit_q5999219_initialize() {

   feldsizex = deffeldsize;
   feldsizey = deffeldsize;
   spielstufe = 2;


   aktzug = 0;
   endzug = 0;
   zug = new tazug;
   (*zug)[aktzug].state = undefiniert;

   steinzuordnung[eins_am_zug] = weiss;
   steinzuordnung[zwei_am_zug] = schwarz;

   flip[eins_am_zug] = zwei_am_zug;
   flip[zwei_am_zug] = eins_am_zug;
/*Zufall wird initialisiert*/
   Randomize();

/*die Vektoren der vier Richtungen*/

   rv[waagerecht].x = 1;
   rv[waagerecht].y = 0;
   rv[senkrecht].x = 0;
   rv[senkrecht].y = 1;
   rv[vorschraeg].x = 1;
   rv[vorschraeg].y = 1;
   rv[rueckschraeg].x = -1;
   rv[rueckschraeg].y = 1;

/*die den Richtungen zugeordneten Zweierpotenzen*/

   pfak[waagerecht]=1;
   pfak[senkrecht]=4;
   pfak[vorschraeg]=16;
   pfak[rueckschraeg]=64;

}
