#include "ptoc.h"

/*Sabine Wolf,5999219,q5999219@bonsai.fernuni-hagen.de, Version vom 6.9.0.2*/

/*Diese Unit erzeugt alle grafischen Objekte und reagiert auf den Benutzer.*/

#define __gomgui_implementation__


#include "gomgui.h"


#include "graph.h"
/*#include "simpmaus.h"*/
/*#include "crt.h"*/


const integer miny = -240;
const integer maxy = 240;
const integer minx = -320;
const integer maxx = 320;
const integer mitsx = 320;
const integer mitsy = 239;
const integer vwinkel = 65;
const integer hwinkel = 190;
const integer farbehimmel = 255;
const integer farbeebene = 170;
const integer farberand = 53;
const integer farbefeld = 204;
const integer farbewmarkfeld = 238;
const integer farbesmarkfeld = 170;
const integer steinweiss = 127;
const integer steinschwarz = 40;
const integer steingrau = 86;
const real blendfakdef = 0.667;

typedef short txscreenkoord;
const int min_txscreenkoord = minx;
const int max_txscreenkoord = maxx;
typedef short tyscreenkoord;
const int min_tyscreenkoord = miny;
const int max_tyscreenkoord = maxy;

enum tflaeche { reject,himmel,ebene,rand1,feld, last_tflaeche};

enum tzeichmodus {feldz,steinz,alles, last_tzeichmodus};

/*Typ fuer die Position auf dem Bildschirm*/

struct tpixel {
            integer x;
            integer y;
};

/*Typ fuer die Position auf der Spielfeldebene*/

struct tplanepos {
                  real x;
                  real z;
};

/*Typ fuer die Position eines Punktes im Raum*/

struct traumpos {
              real x;
              real y;
              real z;
};

/*Typ fuer die Perspektiv-Parameter*/

struct tgraphstate {
                  real vx,
                  vy,
                  vz,
                  lx,
                  ly,
                  vxx,
                  vxz,
                  vyx,
                  vyy,
                  vyz,
                  rad;
                  tplanepos basis;
                  integer stomax,
                  stomix,
                  stomaz,
                  stomiz;
                  integer komplett;
                  real fdy,
                  fddy;
};

/*Struktur der Bildresourcen*/

typedef array<0,64000,byte> tbildarray;

typedef tbildarray* tzbildarray;

typedef array<0,350,integer> tzeilenarray;

struct tbild {
           asciiz name;
           tzbildarray bild;
           tzeilenarray zeile;
           integer xsize;
           integer ysize;
           integer xanfang;
           integer yanfang;
};

typedef matrix<0,191,0,287,byte> thintergrund;
typedef thintergrund* tzhintergrund;
typedef matrix<0,28,0,579,byte> tziffa;
typedef tziffa* tzziffa;
typedef matrix<0,49,0,149,byte> tsmily;
typedef tsmily* tzsmily;
typedef real tfarbe;

/*Typen fuer die Ordered-Dither-Matrizen*/

typedef unsigned char t0_7;
const int min_t0_7 = 0;
const int max_t0_7 = 7;
struct tr0_7 {
           t0_7 x,y;
};

/*Der grafische Zustand, in dem alles an Positionen und Parameter gespeichert
 wird, was zur Erzeugung der Grafik wichtig ist*/

tgraphstate grs;

/*Die Farben der Steine*/

array<0,last_tstein,tfarbe> cst;

/*Der Horizontal-,der Vertikalwinkel und der Abstand des Beobachters
 zum Spielfeld*/

real vwi,hwi,hz,dist;

/*Die Feldgroesse, wie sie mit der Maus eingestellt wurde.*/

integer mausfeldsizex,mausfeldsizey;

/*Die Farbe der Felder, das grad markierte Feld ist heller als die anderen*/

matrix<min_tfeldkoord,max_tfeldkoord,min_tfeldkoord,max_tfeldkoord,integer> feldmark;

/*Die benoetigten Bilder*/

tbild ziffernbild,
ziffern2bild,
dotbild,
compspielerbild,
weissschwarzbild,
steuerbild,
cpstbild,
cpsttinybild;
tzhintergrund hintergrund;

/*Vorheriger Spielzustand, um nach einem abgelehnten Quit wieder zum
 richtigen Zustand zurueckzukommen.*/

tspielstate prevstate;
integer prevkomplett;

/*Ordered-Dither-Tabellen*/

matrix<min_t0_7,max_t0_7,min_t0_7,max_t0_7,real> orddith;
array<0,63,tr0_7> orddtab;

/*aktuell markiertes Feld*/

tfeldpos aktfeld,setfeld;

/*Anzahl der bisher gemachten Screenshots.*/

integer screenshotnum;

real blendfak;

tzziffa ziffarray;
tzsmily smily;
integer smilymod;
boolean vvismode,vismode;

integer zugnummer;

tspieler wspieler,sspieler;

boolean idle;

/*PlaneToPixel ermittelt, auf welchem Pixel der Mittelpunkt eines Feldes
 liegt und gibt eine grobe, aber positive Abschaetzung, wieviel Pixel
 das Feld abdeckt.*/

integer planetopixel(    tspiel sp,
                        tfeldpos feld,
                         tpixel& pix    )

{
    real nenn,bx,bz;


   integer planetopixel_result;
   bx=feld.x+grs.basis.x;
   bz=feld.z+grs.basis.z;
   {
   nenn=grs.ly*(bz * grs.vxx * grs.vyy -
                   bx * grs.vxz * grs.vyy +
                   grs.vx * grs.vxz * grs.vyy +
                   grs.vy * grs.vxx * grs.vyz -
                   grs.vy * grs.vyx * grs.vxz -
                   grs.vz * grs.vxx * grs.vyy);
   pix.x=round(-( bx *  grs.vz * grs.vyy -
                   bx *  grs.vy * grs.vyz -
                   bz *  grs.vx * grs.vyy +
                   bz *  grs.vy * grs.vyx) /nenn);
   pix.y=round(-( bx * grs.vy * grs.vxz -
                   bz * grs.vy * grs.vxx) /nenn);

      planetopixel_result = trunc(1500/sqrt(sqr(grs.vy)+sqr(grs.vx-bx)+sqr(grs.vz-bz)));
   }
   return planetopixel_result;
}    /* planetopixel */





/*PixelToPlane ermitteln den Punkt, in dem der Sehstrahl vom Beobachter
 durch das Pixel die Spielebene trifft, und zu welchem Feld dieser Punkt
 gehoert. Die Spielebene erstreckt sich in X- und in Z-Richtung und liegt
 auf Y=-Steinradius, damit sie grade unter den Steinen liegt, deren
 Mittelpunkt auf Y=0 liegt.*/

tflaeche pixeltoplane(   tspiel& sp,
                              real pixx,real pixy,
                              tfeldpos& pos,
                              tplanepos& posp,
                              tplanepos& posr,
                              traumpos& rpos,
                              integer level    )

{
      tflaeche pixeltoplane_result;
      {
   rpos.x = -grs.vx/grs.ly + pixx*grs.vxx + pixy*grs.vyx;
   rpos.y = -grs.vy/grs.ly + pixy*grs.vyy;
   rpos.z = -grs.vz/grs.ly + pixx*grs.vxz + pixy*grs.vyz;

   if (rpos.y<0) 
   {
      posp.x=grs.vx-rpos.x*(grs.vy+grs.rad)/rpos.y;
      posp.z=grs.vz-rpos.z*(grs.vy+grs.rad)/rpos.y;
      if  ((level<4) &&
          (abs(posp.x)<(real)(sp.feldsizex)/2+0.3+grs.fdy+grs.fddy*pixy) &&
          (abs(posp.z)<(real)(sp.feldsizey)/2+0.3+grs.fdy+grs.fddy*pixy) && !
         ((abs(posp.x)<(real)(sp.feldsizex)/2+0.3-grs.fdy-grs.fddy*pixy) &&
          (abs(posp.z)<(real)(sp.feldsizey)/2+0.3-grs.fdy-grs.fddy*pixy))) 
          pixeltoplane_result=reject;
      else
      if  ((abs(posp.x)<(real)(sp.feldsizex)/2+0.3) &&
          (abs(posp.z)<(real)(sp.feldsizey)/2+0.3)) 
          if ((level<4) &&
          (abs(posp.x)<(real)(sp.feldsizex)/2+grs.fdy+grs.fddy*pixy) &&
          (abs(posp.z)<(real)(sp.feldsizey)/2+grs.fdy+grs.fddy*pixy) && !
         ((abs(posp.x)<(real)(sp.feldsizex)/2-grs.fdy-grs.fddy*pixy) &&
          (abs(posp.z)<(real)(sp.feldsizey)/2-grs.fdy-grs.fddy*pixy))) 
          pixeltoplane_result=reject;
      else
      {
         pos.x=round(posp.x-grs.basis.x);
         pos.z=round(posp.z-grs.basis.z);
         if ((pos.x>0) && (pos.x<=sp.feldsizex) &&
            (pos.z>0) && (pos.z<=sp.feldsizey)) 
         {
            posr.x=(posp.x-grs.basis.x)-pos.x;
            posr.z=(posp.z-grs.basis.z)-pos.z;

            pixeltoplane_result=feld;
         }
         else
            pixeltoplane_result=rand1;
      }
      else
         pixeltoplane_result=ebene;
   }
   else
      pixeltoplane_result=himmel;
      }
      return pixeltoplane_result;
}    /* pixeltoplane */





/*PutPixS reduziert die Farbe eines Pixel auf die darstellbaren Farben
 mittels Ordered-Dither-Verfahren und setzt dieses Pixel*/

void putpixs(integer x,integer y,real va)

{
   putpixel(x,y,trunc(va/17+orddith[(x+800) % 7][(y+800) % 7]));
}    /* PutPixS */





/*PutPix rechnet die zentrierten Koordinaten, wie sie fÅr die Grafik
verwendet werden in Screen-Koordinaten um, und ruft PutPixS auf.*/

void putpix(tpixel pix,real va)

{
   putpixs(pix.x+mitsx,mitsy-pix.y,va);
}






/*BerechnePixel berechnet ein Pixel in der Szene mit dem Spielfeld*/

real berechnepixel(tspiel& sp, real pixx,real pixy,tzeichmodus modus,integer level)

{
   tfeldkoord kx,kz;
   real bx,bz;
   real col;
   real a,b,c,d,t,hell;
   real kux,kuy,kuz;
   real ab;
   tflaeche pixtyp;
   tfeldpos feldpos;
   tplanepos planepos,posrest;
   traumpos raum;
   integer indexx,indexz,
   zif,zx,zz;
   real ps,versatz;




   /*Es wird ueberprueft, wo und ob ein Sehstrahl vom Beobachter durch das
    entsprechende Pixel auf dem Bildschirm auf die Spielebene trifft.
    Fuer Himmel und Ebene bekommt das Pixel eine entsprechende Farbe,
    fuer das Spielfeld mit Rand wird genauer geschaut, welchen Farbwert
    das Pixel bekommt.*/

   real berechnepixel_result;
   pixtyp = pixeltoplane(sp,pixx,pixy,feldpos,planepos,posrest,raum,level);

   col=1000;

   if ((! (modus==feldz)) ||
   ((feldpos.x==aktfeld.x) && (feldpos.z==aktfeld.z))) 
   {
            switch (pixtyp) {
              case reject    : col=998; break;
              case himmel    : col=farbehimmel; break;
              case ebene     : col=farbeebene; break;
              case rand1:case feld :
              {
                 /*Fuer jedes Feld- oder Rand-Pixel wird ueberprueft, ob ein
                  auf dem zugehoerigen Feld oder in der Umgebung liegender
                  Stein zu sehen ist, dessen Farbe dann genommen wird*/

                 for( kx = max(0,feldpos.x+grs.stomix); kx <=
                           min(sp.feldsizex+1,feldpos.x+grs.stomax); kx ++)
                    for( kz = max(0,feldpos.z+grs.stomiz); kz <=
                              min(sp.feldsizey+1,feldpos.z+grs.stomaz);  kz ++)
                    {
                       if  (sp.feld[kx][kz]!=leer) 
                       {

                          /*kux,kuy,kuz sind die Koordinaten des
                           Steinmittelpunktes,
                           vx,vy,vz sind die Koordinaten des
                           Beobachters,
                           raum ist die Position des
                           Pixels im Raum, gesucht wird ein Punkt auf
                           dem Sehstrahl vom Beobachter durch den Pixel,
                           dessen Abstand von der Steinmitte gleich dem
                           Radius ist. (Genaugenommen gibt es zwei dieser
                           Punkte, wenn der Strahl durch den Stein geht,
                           es wird aber nur der dem Betrachter naeher
                           liegende gebraucht).*/

                          kux = kx + grs.basis.x;
                          kuz = kz + grs.basis.z;
                          kuy = 0;
                             {
                                a = sqr(raum.x)+sqr(raum.y)+sqr(raum.z);
                                b = 2*((grs.vx-kux)*raum.x+
                                     (grs.vy-kuy)*raum.y+(grs.vz-kuz)*raum.z);
                                c = sqr(grs.vx)+sqr(grs.vy)+sqr(grs.vz)-
                                     2*grs.vx*kux-2*grs.vy*kuy-2*grs.vz*kuz+
                                     sqr(kux)+sqr(kuy)+sqr(kuz)-sqr(grs.rad);
                                d = b*b-4*a*c;
                                ps=-b/(2*a);
                                if ((level<4) && (abs(d)*level<0.0006*ps)) 
                                {
                                   col=998;
                                }
                                else
                                /*Wenn d>0 dann liegt der Stein im Sehstrahl*/
                                if (d > 0) 
                                {
                                   if ((modus==feldz) || ((modus==steinz) && ((kx!=setfeld.x)||(kz!=setfeld.z)))) 
                                      col=999;
                                   else
                                   {
                                   /*t gibt die Position des Punktes auf dem
                                    Sehstrahl an, bei t=0 laege er im
                                    Betrachter, bei t=1 im Pixel*/

                                   t = (-b-sqrt(d))/(2*a);

                                   /*Der Stein ist oben hell,unten dunkel mit
                                    einem cosinusfoermigen Verlauf*/
                                   hell = 1-cos((grs.vy+t*raum.y+grs.rad)/grs.rad/2*3.1415);

                                   /*Die Pixelfarbe ergibt sich aus der
                                    Grundfarbe mal der Helligkeit.
				    Wenn smilymod>0 und dieses Feld=setFeld,
                                    dann wird ein Smiley bzw. ein Kreuz
                                    auf den Stein gezeichnet.*/

                                   if ((smilymod>0) && (setfeld.x==kx) && (setfeld.z==kz)) 
                                   {
                                     if (level<4) 
                                        col=998;
                                     else
                                       if (sp.feld[kx][kz]==weiss) 
                                         col = maxr(0,hell*(cst[sp.feld[kx][kz]])-255+
                                         (*smily)[trunc((grs.vx+t*raum.x-kux)/grs.rad*25+25)][
                                         trunc(-(grs.vz+t*raum.z-kuz)/grs.rad*25-25++smilymod*50)]);
                                       else
                                         col = minr(255,hell*(cst[sp.feld[kx][kz]])+170-
                                         (real)((*smily)[trunc((grs.vx+t*raum.x-kux)/grs.rad*25+25)][
                                         trunc(-(grs.vz+t*raum.z-kuz)/grs.rad*25-25+smilymod*50)]*2)/3);
                                   }
                                   else
                                   col = hell*cst[sp.feld[kx][kz]];
                                   }
                                }
                             }
                       }
                    }

                 /*Wenn kein Stein im Sehstrahl liegt, dann wird weitergeschaut*/

                 if (col==1000) 
                 {
                    if (pixtyp==rand1) 
                       col=farberand;
                    else
                    {

                       /* Es wird geschaut, ob der Sehstrahl auf eine
                         Fuge trifft, Fuge wird vom Rand hin zur Mitte dunkler*/
                       if ((level<4) &&
                          (((abs(posrest.x)>0.47-grs.fdy-grs.fddy*pixy) &&
                            (abs(posrest.x)<0.47+grs.fdy+grs.fddy*pixy)) ||
                            (abs(posrest.x)>0.5-grs.fdy-grs.fddy*pixy))) 
                        col=996;
                        else
                        if ((level<4) &&
                          (((abs(posrest.z)>0.47-grs.fdy-grs.fddy*pixy) &&
                            (abs(posrest.z)<0.47+grs.fdy+grs.fddy*pixy)) ||
                            (abs(posrest.z)>0.5-grs.fdy-grs.fddy*pixy))) 
                           col=997;
                       else
                       {
                       if (abs(posrest.x)>0.47) 
                          col=feldmark[feldpos.x][feldpos.z]+
                               (0.47-abs(posrest.x))*2500;
                       if (abs(posrest.z)>0.47) 
                          col=minr(col,feldmark[feldpos.x][feldpos.z]+
                                        (0.47-abs(posrest.z))*2500);
                       /*Wenn keine Fuge, dann bekommt das Pixel die Farbe
                        fuer ein Feld bzw. ein grade markiertes Feld, wobei
                        noch geschaut wird, ob das Pixel im Schatten eines
                        evtl. drauf liegenden Steines liegt. Der Schatten ist
                        kreisrund mit einem cosinusfoermigen Fall-off am Rand.*/

                       if (col == 1000) 
                       {
                          col=feldmark[feldpos.x][feldpos.z];
                          if  (sp.feld[feldpos.x][feldpos.z] != leer) 
                          {

                             /*Schattenmittelpunkt etwas versetzt zur Mitte*/

                             bx=posrest.x-0.02;
                             bz=posrest.z+0.02;

                             /*Abstand zur Mitte*/

                             ab=sqrt(bx*bx+bz*bz);

                             /*Fall-off zwischen Radius+0.1 und Radius-0.1*/

                             if (ab<grs.rad+0.1) 
                                if (ab<grs.rad-0.1) 
                                   col=col/3;
                                else
                                   col=col/3+col*2/6*
                                        (1-cos((ab-grs.rad+0.1)/0.2*3.1415));
                             }
                          }
                       }
                    }
                 }
              }
              break;
            }
   }

   switch  (trunc(col)) {
   case 998: {
          versatz=0.25/level;
          col=(berechnepixel(sp,pixx-versatz,pixy-versatz,alles,level*2)+
                berechnepixel(sp,pixx-versatz,pixy+versatz,alles,level*2)+
                berechnepixel(sp,pixx+versatz,pixy-versatz,alles,level*2)+
                berechnepixel(sp,pixx+versatz,pixy+versatz,alles,level*2))/4;
        }
        break;
   case 996: {
          versatz=0.25/level;
          col=(berechnepixel(sp,pixx-versatz,pixy,alles,level*2)+
                berechnepixel(sp,pixx+versatz,pixy,alles,level*2))/2;
        }
        break;
   case 997: {
          versatz=0.25/level;
          col=(berechnepixel(sp,pixx,pixy-versatz,alles,level*2)+
                berechnepixel(sp,pixx,pixy+versatz,alles,level*2))/2;
        }
        break;
   }
   berechnepixel_result=col;

   return berechnepixel_result;
}    /* ZeichnePixel */



/*ZeichnePixel zeichnet ein Pixel in der Szene mit dem Spielfeld*/

void zeichnepixel(tspiel& sp, tpixel pix,tzeichmodus modus)
{
   real col;

   col=berechnepixel(sp,pix.x,pix.y,modus,1);
   if (col<256) 
      putpix(pix,col);
}



/*ZeichneFlaeche zeichnet einen Ausschnitt der Szene bzw. die ganze Szene.
 Die Flaeche wird  aufgeteilt in 64 Teile,so da es auch m"glich ist, nur
 einen Teil auf einmal zu zeichnen. Jeder Teil deckt den ganzen Bereich ab,
 hat aber in X- und in Y-Richtung nur jedes 8. Pixel fuer sich. Jeder Teil hat
 einen anderen Versatz, der in ordtab steht. Zeichne ich ein neues Bild
 ergibt sich durch das Zeichen der 64 Teile eine Art Ueberblendeffekt.*/

void zeichneflaeche(tspiel& sp,
                             integer ax,integer ay,integer ex,integer ey,
                             integer von,integer bis,
                             tzeichmodus modus )

{
   integer i,randx;
   tpixel pix;


   mouseoff;
   for( i=von; i <= bis; i ++)
   {
      pix.y = ay-((ay+320) % 8)+orddtab[i].y;
      while (pix.y<ey) 
      {

         /*Wenn vismode=true, dann wird die rechte, obere Screen-Ecke
          freigehalten f¸r das Steuerungsfeld.*/

         if ((pix.y<124) || (! vismode)) 
           randx=320;
         else
           randx=240;

         pix.x = ax-((ax+320) % 8)+orddtab[i].x;

         while (pix.x<min(ex,randx)) 
         {
            zeichnepixel(sp,pix,modus);
            pix.x += 8;
         }
         pix.y += 8;
      }
   }
   mouseon;
}    /* ZeichneFlaeche */





/*ZeichneFeld zeichnet den Bereich, den ein Feld abdeckt.*/

void zeichnefeld(tspiel& sp,
                         tfeldpos pos,
                         tzeichmodus modus)

{
   tpixel pix;
   integer gr;



   /*Es wird eine Flaeche neugezeichnet, die so gross ist, dass das Feld
    auf jeden Fall ganz drinnliegt, es sei denn GrS.komplett ist null,
    weil dann das ganze Bild eh neugezeichnet wird.*/

   if (grs.komplett>0) 
   {
      gr = planetopixel(sp,pos,pix);
      zeichneflaeche(sp,pix.x-gr, pix.y-gr, pix.x+gr,pix.y+gr,0,
      grs.komplett-1,modus);
   }

}    /* ZeichneFeld */





/*BewegeFeld setzt das markierte Feld neu und zeichnet den Bereich,
 der sich geaendert hat neu. (Genauer das Rechteck, das beide
 Feldbereiche einschliesst.)*/

void bewegefeld (tspiel& sp,
                          tfeldpos& von,
                          tfeldpos& nach)

{
   tpixel pixvon,pixnach;
   integer grvon,grnach;


   /*Die Markierung wechselt vom von-Feld zum nach-Feld*/

   grnach = planetopixel(sp,nach,pixnach);
   if (von.x>0) 
   {
      feldmark[von.x][von.z]=farbefeld;
      zeichnefeld(sp,von,feldz);
   }
   von=nach;
   if (sp.spielstate == weissamzug) 
      feldmark[nach.x][nach.z]=farbewmarkfeld;
   else
      feldmark[nach.x][nach.z]=farbesmarkfeld;
   zeichnefeld(sp,nach,feldz);

}    /* BewegeFeld */




/*Testet, ob ein Pixel innerhalb eines Bereiches liegt.*/

boolean innerhalb(tpixel pos,
                   integer ax,integer ay,integer ex,integer ey)
{

   boolean innerhalb_result;
   innerhalb_result=(pos.x>=ax) && (pos.x<ex) &&
              (pos.y>=ay) && (pos.y<ey);

   return innerhalb_result;
}    /* innerhalb */




/*Zeichnet die ganze Szene neu.*/

void zeichnealles(tspiel& sp)
{
   tflaeche pixtyp;
   tfeldpos feldpos;
   tplanepos planepos,planepos2,
   posrest;
   traumpos raum;
   real diff1,diff2;


   /*Aus den Werten fuer den Horizontal- und Vertikalwinkel und dem
    Abstand des Beobachters zum Spielfeldmittelpunkt werden die
    Position des Beobachters und Parameter fuer die Lage der Pixelflaeche
    (eine Art Fenster, durch das der Beobachter auf das Spielfeld schaut)
    berechnet.*/

   mouseoff;
   {
       grs.vy = sin(vwi/180*3.1415)*dist;
       hz = cos(vwi/180*3.1415)*dist;
       grs.vx = hz*sin(hwi/180*3.1415);
       grs.vz = hz*cos(hwi/180*3.1415);
       grs.lx = sqrt(sqr(grs.vx)+sqr(grs.vz));
       grs.ly = sqrt(sqr(grs.vx)+sqr(grs.vy)+sqr(grs.vz));
      grs.vxx =-grs.vz/grs.lx*0.0005;
      grs.vxz = grs.vx/grs.lx*0.0005;
      grs.vyx =-grs.vy/grs.ly*grs.vx/grs.lx*0.0005;
      grs.vyy = grs.lx/grs.ly*0.001666;
      grs.vyz =-grs.vy/grs.ly*grs.vz/grs.lx*0.0005;
      grs.rad = 0.4;
      grs.basis.x=(real)(-sp.feldsizex)/2-0.5;
      grs.basis.z=(real)(-sp.feldsizey)/2-0.5;

      /*Schaut man schraeg auf das Feld, kann es sein, dass im
       Bereich eines Feldes mehrere Steine zu sehen sind, deshalb
       muss ich die Nachbarschaft dann noch mit einbeziehen.*/

      grs.stomax=0;
      grs.stomix=0;
      grs.stomiz=0;
      grs.stomaz=0;

      if (grs.vx/dist> 0.2)  grs.stomax= 1;
      if (grs.vx/dist<-0.2)  grs.stomix=-1;
      if (grs.vz/dist> 0.2)  grs.stomaz= 1;
      if (grs.vz/dist<-0.2)  grs.stomiz=-1;

      /*Es wird ermittelt, um wieviel sich etwa die Feldkoordinaten pro Pixel Ñndern.*/
      pixtyp = pixeltoplane(sp,0,-240,feldpos,planepos,posrest,raum,1);
      pixtyp = pixeltoplane(sp,0,-239,feldpos,planepos2,posrest,raum,1);

      diff1=sqrt(sqr(planepos.x-planepos2.x)+sqr(planepos.z-planepos2.z));

      pixtyp = pixeltoplane(sp,0,239,feldpos,planepos,posrest,raum,1);
      pixtyp = pixeltoplane(sp,0,240,feldpos,planepos2,posrest,raum,1);
      diff2=sqrt(sqr(planepos.x-planepos2.x)+sqr(planepos.z-planepos2.z));

      grs.fdy=(diff2+diff1)/2/2;
      grs.fddy=(diff2-diff1)/480/2;

      /*Bild wird als nicht gezeichnet markiert, damit es mit jedem
       Aufruf von Gui ein Stueck weit neu gezeichnet wird.*/

      grs.komplett=0;

   }
}    /* ZeichneAlles */





/*LadeBild laedt ein Bild, indem es dafuer Speicher anfordert und das Bild
 dann in einer tBild-Variablen ablegt. Das Bild ist run-length-encoded,
 um Speicher zu sparen.*/

void ladebild(tbild& bild,string name)

{
   file<byte> bfile;
   integer nx,ny;
   byte* p;
   byte hb,lb;
   integer len,i,zn,po;
   byte co;



   if (bild.bild==nil) 
      {
         assign(bfile,name);
         reset(bfile);
         bfile >> hb >> lb;
         len=hb*256+lb;
         bfile >> hb >> lb;
         nx=hb*256+lb;
         bfile >> hb >> lb;
         ny=hb*256+lb;
         if (maxavail<len) 
            {
               output << "Nicht genug Speicher!" << NL;
               exit(0);
            }
         getmem(p,len);
         bild.bild=(tzbildarray)(p);

         for( i=0; i <= len-1; i ++)
            bfile >> (*bild.bild)[i];

         zn=0;i=0;po=0;
         bild.zeile[0]=0;

         do {
            co=(*bild.bild)[i];
            if ((co & 1)==1) 
            {
               po += co / 2;
               i += 2;
            }
            else
            {
               po += 1;
               i += 1;
            }
            if (po>=nx) 
            {
               zn += 1;
               bild.zeile[zn]=i;
               po=0;
            }
         } while (!(i>=len));
         bild.xsize=nx;
         bild.ysize=ny;
         bild.name=name;
      }
}    /* LadeBild */





/*Mischt einen Bildpunkt mit dem Hintergrund.
 (Genauer gesagt, der Hintergrund wird auf 1-mix abgesenkt,
 dazu kommen mix-mal Weiss, und das wird dann mit dem Bild
 multipliziert. Schwarze Teile des Bildes bleiben also schwarz.)*/

void bildmix(integer kx,integer ky,byte co,real mix)
{
     if (mix==1) 
               putpixs(kx,ky,co);
     else
        if ((kx & 1)==0) 
           putpixs(kx,ky,trunc(co*((real)(((*hintergrund)[(kx-128) / 2][
                        ky-96] / 16))/15*(1-mix)+mix)));
        else
           putpixs(kx,ky,
                       trunc(co*((real)(((*hintergrund)[(kx-128) / 2][
                        ky-96] & 15))/15*(1-mix)+mix)));
}





/*Laedt ein Vordergrundbild und stellt es direkt dar.*/

void ladevordergrund(string name)

{
   file<byte> bfile;
   integer nx,ny;
   integer x,y;
   byte hb,lb;
   integer len;
   byte anb;
   byte co;

         assign(bfile,name);
         reset(bfile);
         bfile >> hb >> lb;
         len=hb*256+lb;
         bfile >> hb >> lb;
         nx=hb*256+lb;
         bfile >> hb >> lb;
         ny=hb*256+lb;
         anb=0;

         for( y=0; y <= ny-1;  y ++)
            for( x= 0; x <= nx-1; x ++)
            {
               if (anb==0) 
               {
                  bfile >> co;
                  if ((co & 1)==1) 
                  {
                     anb=co / 2;
                     bfile >> co;
                  }
                  else
                     anb=1;
               }
               bildmix(x+128,y+96,co+1,blendfak);
               anb -= 1;
            }

         close(bfile);


}



/*HoleHintergrund speichert den mittleren Teil des Bildes in Hintergrund.*/

void holehintergrund()

{
   integer x,y;



   /*Es werden immer 2 Pixel in einem Byte gespeichert, da sonst das Array
    groesser als 64k waere.*/

   for( y = 0; y <= 287; y ++)
      for( x = 0; x <= 191; x ++)
         (*hintergrund)[x][y]=getpixel(x*2+128,y+96)*16+
                                     getpixel(x*2+129,y+96);


   grs.komplett=64;
}    /* HoleHintergrund */




/*Zeichnet den gespeicherten Hintergrund.*/

void zeichnehintergrund()

{
   integer x,y;


   for( y    = 0; y <= 287; y ++)
      for( x = 0; x <= 191; x ++)
         {
         putpixel(x*2+128,y+96,(*hintergrund)[x][y] / 16);
         putpixel(x*2+129,y+96,(*hintergrund)[x][y] & 15);
         }

}    /* ZeichneHintergrund */





/*ZeichneBild zeichnet die gewuenschten Zeilen eines Bild an gewuenschter
 Stelle ueber den Hintergrund.*/

void zeichnebild(tbild bild,
                      integer xanfang,
                      integer yanfang,
                      integer ybanfang,
                      integer ybsize
                      )

{
   integer x,y;
   integer za;
   byte co,anb;

   anb=0;
   co=0;
   za=bild.zeile[ybanfang];
   for( y=0; y <= ybsize-1; y ++)
   {
      for( x=0; x <= bild.xsize-1; x ++)
      {
         if (anb==0) 
         {
            co=(*bild.bild)[za];
            za += 1;
            if ((co & 1)==1) 
            {
               anb=co / 2;
               co=(*bild.bild)[za];
               za += 1;
            }
            else
               anb=1;
         }
         bildmix(xanfang+x,yanfang+y,co+1,blendfak);
         anb -= 1;
      }
   }

}




/*GuiInit initialisiert alle noetigen Variablen*/

void guiinit(tspiel& sp )

{
   integer t,po,x,y,z,nx,
   nz,pp,i,col,
   gd , gm,  error;
   real va,ami,mi;
   text dfile;
   t0_7 ox,oy;
   byte* p;
   file<byte> bfile;
   byte farbe;
   boolean ok;


/*Zeiger auf Bildspeicher werden auf nil gesetzt.*/
  dotbild.bild=nil;
  ziffernbild.bild=nil;
  ziffern2bild.bild=nil;
  compspielerbild.bild=nil;
  weissschwarzbild.bild=nil;
  steuerbild.bild=nil;
  cpstbild.bild=nil;
  cpsttinybild.bild=nil;



   /*Ordered-Dither-Matrix wird erzeugt, um mehr Farben als vorhanden
    zu koennen. Resultat ist eine 8x8-Matrix, die mit einer Grauwert-Skala
    gefuellt ist, gegen die dann beim Setzen eines Pixels verglichen wird*/

   po=1;
   pp=4;
   va=(real)(1)/64;
   orddith[0][0]=0.0;
   for( t=1; t <= 3; t ++)
   {
      for( x=0; x <= po-1; x ++)
         for( y= 0; y <= po-1; y ++)
         {
            orddith[x*pp*2+pp][y*pp*2+pp]= orddith[x*pp*2][y*pp*2]+va*po*po;
            orddith[x*pp*2+pp][y*pp*2]=    orddith[x*pp*2][y*pp*2]+va*po*po*2;
            orddith[x*pp*2][y*pp*2+pp]=    orddith[x*pp*2][y*pp*2]+va*po*po*3;
         }
      po=po*2;pp=pp / 2;
   }

   /*Die Ordered-Dither-Werte werden mit ihrer Position noch in eine Tabelle
    einsortiert, wird zum Ueberblenden spaeter benutzt*/
   orddtab[0].x=0;
   orddtab[0].y=0;
   ami=0;
   for( i=1; i <= 63; i ++)
   {
      mi=99.9;
      for( oy=0; oy <= 7; oy ++)
         for( ox= 0; ox <= 7; ox ++)
            if ((orddith[ox][oy]>ami) && (orddith[ox][oy]<mi)) 
            {
               mi=orddith[ox][oy];
               orddtab[i].x=ox;
               orddtab[i].y=oy;
            }
      ami=mi;
   }

   /*Den Steinen werden Farbwerte zugeordnet*/

   cst[leer]=0;
   cst[weiss]=steinweiss;
   cst[schwarz]=steinschwarz;
   cst[tip]=steingrau;

   /*Das aktuell markierte Feld wird auf (0,0) gesetzt.*/

   aktfeld.x=0;
   aktfeld.z=0;

   /*Die Anzahl der Screenshots wird auf 0 gesetzt.*/

   screenshotnum=0;
   for( x=0; x <= maxfeldsize +1; x ++)
      for( z=0; z <= maxfeldsize +1; z ++)
      {
         sp.feld[x][z]=leer;
         feldmark[x][z]=farbefeld;
      }

   assign(dfile,"gomoku.pnm");
   reset(dfile);
   dfile >> nx >> NL;
   dfile >> nz >> NL;
   for( z = nz; z >= 1; z --)
      for( x = 1; x <= nx; x ++)
      {
         dfile >> col >> NL;
         if (ioresult!=0)  exit(0);
         if (col<128) 
            if  (Random(2)==0) 
               sp.feld[x][z]=schwarz;
            else
               sp.feld[x][z]=weiss;
      }
   close(dfile);

   /*Die Grafik wird initialisiert*/

   gd=detect;
   gm=0;
   initgraph ( gd , gm, "");
   error = graphresult();
   if ( error != grok ) 
   {
      output <<  "Grafik wird nicht unterstuetzt! "  << NL;
      exit (1);
   }

   /*Die Palette bekommt einen Grauverlauf*/

     for( y = 0; y <= 63; y ++)
         setrgbpalette(y,y*4,y*4,y*4);

     setcolor(15);
     directvideo=false;

   /*Initialisiere Hintergrundbild*/

     for( y=0; y <= 15; y ++)
     setpalette(y,y+16);

   hintergrund = new thintergrund;

   if (~ mousereset) 
   {
      output << "Konnte Maus nicht initialisieren" << NL;
   exit(0);
   }

   sp.feldsizex=19;
   sp.feldsizey=19;
   mausfeldsizex=sp.feldsizex;
   mausfeldsizey=sp.feldsizey;


      /*Defaultansicht von schraeg oben,Abstand haengt von der Feldgroesse ab.*/

   vwi = vwinkel;
   hwi = hwinkel;
   dist= 4.5*max(sp.feldsizex,sp.feldsizey);


   /*Grafikaufbau wird angestossen*/

   zeichnealles(sp);


   sp.zuordnung[weissamzug]   = computer;
   sp.zuordnung[schwarzamzug] = mensch;
   sp.spielstate = intro;
   setzespielstufe(1,ok);

   /*kein zu setzender Stein*/

   sp.neugesetzt.x=0;

   idle=false;

   blendfak = blendfakdef;

   smily = new tsmily;
   assign(bfile,"smily.raw");
   reset(bfile);
   for( y=0; y <= 149; y ++)
      for( x=0; x <= 49; x ++)
      {
         bfile >> farbe;
         (*smily)[x][y]=farbe;
      }
   close(bfile);
   setfeld.x=0;
   setfeld.z=0;
   vismode=false;
   smilymod=0;

}    /* GuiInit */




void zeichnezuordnung(tspiel sp,tspielstate spst)

{
   integer y;


   if (spst== weissamzug) 
      y=183;
   else
      y=214;
   if (sp.zuordnung[spst]==mensch) 
      zeichnebild(compspielerbild,310,y,31,34);
   else
      zeichnebild(compspielerbild,310,y,0,34);
}





/*Die Zuordnung Mensch/Computer zu Schwarz/Weiss wird geaendert.*/

void aenderzuordnung(tspiel& sp,tspielstate spst)

{
   mouseoff;

   if (sp.zuordnung[spst]==mensch) 
   {
      sp.zuordnung[spst]=computer;
   }
   else
      sp.zuordnung[spst]=mensch;
      zeichnezuordnung(sp,spst);
}    /* AenderZuordnung */





/*Ein schwarzer Punkt wird gezeichnet.*/

void zeichnedot(integer x,integer y)
{
   zeichnebild(dotbild,x,y,0,31);
}





/*Die Feldgroesse wird mit einer Zahl angezeigt.*/

void zeichnefgroesse(tspiel& sp)

{
   integer zehner,einer,fs,py;



   mouseoff;
fs=sp.feldsizex;
for( py=0; py <= 1; py ++)
{
   zehner=fs / 10;

   if (zehner>0) 
      zeichnebild(ziffernbild,372,247+py*27,zehner*31,31);
   else
      zeichnebild(ziffernbild,372,247+py*27,310,31);
   einer=fs-zehner*10;
      zeichnebild(ziffernbild,386,247+py*27,einer*31,31);
   zeichnedot(390+fs*3,245+py*27);
fs=sp.feldsizey;
}

}





/*Ein Abbild des Screens wird als TGA-Bild abgespeichert.*/

void screenshot()

{
   text bfile;
   integer x,y;
   byte farbe;
   string s;


   screenshotnum += 1;
   assign(bfile,string("scshot")+intstr(screenshotnum)+".tga");

   rewrite(bfile);
   s="\0\0\3\0\0\0\0\0\0\0\0\0\200\2\340\1\b\b";
   bfile << s;
   for( y=479; y >= 0; y --)
      for( x=0; x <= 639; x ++)
      {
         farbe=getpixel(x,y)*17;
         bfile << chr(farbe);
      }
   close(bfile);

}    /* Screenshot */





/*Zeichnet eine dreistellige Zahl an eine gewÅnschte Stelle.*/

void zeichnenummer(integer zugnummer,integer x,integer y)
{
   integer zifferh,ziffer;


   if (vismode) 
   {
      mouseon;
      blendfak=1;
      zifferh=zugnummer / 100;
      if (zifferh>0) 
         zeichnebild(ziffern2bild,x,y,zifferh*16,16);
      else
         zeichnebild(ziffern2bild,x,y,160,16);
         ziffer=(zugnummer / 10) % 10;
      if ((ziffer>0) || (zifferh>0)) 
         zeichnebild(ziffern2bild,x+8,y,ziffer*16,16);
      else
         zeichnebild(ziffern2bild,x+8,y,160,16);
      ziffer=zugnummer % 10;
      zeichnebild(ziffern2bild,x+16,y,ziffer*16,16);
      blendfak=blendfakdef;
      mouseoff;
   }
}



/*Zeichnet die Anzahl der ZÅge in das Steuerungsfeld.*/

void zeichnezugnummer(integer zugnummer)
{
   zeichnenummer(zugnummer,603,65);
}





/*Lîscht den Tip-Stein, wenn vorhanden.*/

void tipweg(tspiel& sp)

{
   if (sp.feld[setfeld.x][setfeld.z]==tip) 
      sp.feld[setfeld.x][setfeld.z]=leer;
      smilymod=0;
      zeichnefeld(sp,setfeld,steinz);
}





/*Gui ist die Hauptroutine, abhaengig vom Zustand des Spiel zeichnet sie das
 entsprechne und reagiert auf Eingaben und Mausaktionen des Benutzers.*/

tspielstate gui(tspiel& sp)

{
   boolean lrelease,rrelease;
   char char1,char2;
   tspielstate spielstate;
   tpixel mauspos,mausposm;
   tfeldpos feldpos,pos;
   tplanepos planepos,posrest;
   traumpos raum;
   integer x,z,i,farbe,
   ax,ex,ay,ey;
   boolean ok;



   /*Wenn die Szene noch nicht fertig gezeichnet ist, dann wird ein
    weiteres Stueck gezeichnet.*/

   tspielstate gui_result;
   if (grs.komplett<64) 
   {
      zeichneflaeche(sp,minx,miny,maxx,maxy,
                     grs.komplett,grs.komplett,alles);
      grs.komplett += 1;
   }

   spielstate=sp.spielstate;

   /*Wenn der Computer gegen sich selbst spielt, wird immer erst gewartet,
    bis das ganze Bild fertig ist.*/
   if ((grs.komplett<64) && (sp.zuordnung[weissamzug]==computer) && (sp.zuordnung[schwarzamzug]==computer)) 
      idle=true;

   /*Wenn der Computer einen Stein gesetzt hat, wird dieser gezeichnet
   und zum anderen Spieler gewechselt.*/
   if ((sp.neugesetzt.x > 0) && ! idle && ((spielstate==weissamzug) || (spielstate==schwarzamzug))) 
   {
      smilymod=0;
      zeichnefeld(sp,setfeld,steinz);
      setfeld.x=sp.neugesetzt.x;
      setfeld.z=sp.neugesetzt.z;
      sp.feld[setfeld.x][setfeld.z]=stein[sp.spielstate];

      /*Der gesetzte Stein wird mit einem Kreuz gekennzeichnet.*/

      smilymod=2;
      zeichnefeld(sp,setfeld,steinz);
      sp.neugesetzt.x=0;
      spielstate=gegenspieler[spielstate];

/*idle wird auf true gesetzt, damit beim Spiel Computer gegen Computer jeder zweite
 Gui-Aufruf frei bleibt fÅr Benutzer-Aktionen wie Drehen des Spielfeldes, Quitten etc.*/

      idle=true;
      /*Die Zugnummer wird hochgezÑhlt und gezeichnet.*/
      zugnummer += 1;
      zeichnezugnummer(zugnummer);
   }
   else
   {
      idle=false;
      char1='\0';
      char2='\0';
      mauspos.x = mousex;
      mauspos.y = mousey;
      lrelease=buttonleft;
      rrelease=buttonright;

      if (keypressed()) 
      {
         char1=readkey();
         if (char1=='\0') 
            char2=readkey();
      }

      /*Screenshot abspeichern*/

      if (char1=='d') 
         screenshot();

      switch (sp.spielstate) {
        case intro :
             if ((char1!='\0') || (char2!='\0') || lrelease || rrelease) 
             {
                spielstate=startvor;
                hwi=180;vwi=90;
                wspieler=sp.zuordnung[weissamzug];
                sspieler=sp.zuordnung[schwarzamzug];
                gibfeldgroesse(sp.feldsizex,sp.feldsizey);
                mausfeldsizex=sp.feldsizex;
                mausfeldsizey=sp.feldsizey;
                vismode=true;
             }
             break;

        case startvor :
                {
                   mouseoff;
                   vvismode=vismode;
                   if (vismode) 
                   {
                      vismode=false;
                      zeichneflaeche(sp,240,124,320,240,0,grs.komplett-1,alles);
                   }
                   holehintergrund();
                   ladevordergrund("start.rle");
                   ladebild(compspielerbild,"compspie.rle");
                   ladebild(ziffernbild,"ziffern.rle");
                   ladebild(dotbild,"dot.rle");
                   sp.zuordnung[weissamzug]=wspieler;
                   sp.zuordnung[schwarzamzug]=sspieler;
                   zeichnezuordnung(sp,weissamzug);
                   zeichnezuordnung(sp,schwarzamzug);
                   ladebild(cpstbild,"cpst.rle");
                   gibspielstufe(sp.spielstufe);
                   zeichnebild(cpstbild,386,302,sp.spielstufe*30,30);
                   zeichnefgroesse(sp);
                   spielstate=start;
                }
                break;

        case start :
             {
                mouseon;
                if (lrelease) 
                {
                   if (innerhalb(mauspos,440,340,490,370)) 
                      char1='q';
                   if (innerhalb(mauspos,150,340,210,370)) 
                      char1='s';
                   if (innerhalb(mauspos,310,185,420,210)) 
                      char1='w';
                   if (innerhalb(mauspos,310,215,420,240)) 
                      char1='b';
                   for( i=0; i <= 2;  i ++)
                   if (innerhalb(mauspos,386+i*16,302,402+i*16,330)) 
                   char1=chr(i+48);
                }
                if (innerhalb(mauspos,390,247,510,267)) 
                   mausfeldsizex=(mauspos.x-400) / 3;
                if (innerhalb(mauspos,390,274,510,294)) 
                   mausfeldsizey=(mauspos.x-400) / 3;

                mausfeldsizex=max(min(mausfeldsizex,maxfeldsize),minfeldsize);
                if (mausfeldsizex>sp.feldsizex)  char1='+';
                if (mausfeldsizex<sp.feldsizex)  char1='-';

                mausfeldsizey=max(min(mausfeldsizey,maxfeldsize),minfeldsize);
                if (mausfeldsizey>sp.feldsizey)  char1='*';
                if (mausfeldsizey<sp.feldsizey)  char1='_';

                switch (char1) {

                  case 's' :
                     {
                        wspieler=sp.zuordnung[weissamzug];
                        sspieler=sp.zuordnung[schwarzamzug];
                        spielstate = weissamzug;
                        zugnummer = 0;
                        setzefeldgroesse(sp.feldsizex,sp.feldsizey,ok);
                        neuesspiel(ok);
                        vismode=vvismode;
                        dist = maxr(sp.feldsizex*4.5*3/4,sp.feldsizey*4.5);

                        /*Wenn vismode=true, dann wird das Steuerungsfeld gezeichnet.*/

                        if (vismode) 
                        {
                        ladebild(steuerbild,"steuer.rle");
                        ladebild(cpsttinybild,"cpsttiny.rle");
                        ladebild(ziffern2bild,"ziffern2.rle");
                        blendfak=1;
                        zeichnebild(steuerbild,560,0,0,116);
                        zeichnebild(cpsttinybild,600,84,sp.spielstufe*16,16);
                        zeichnezugnummer(zugnummer);
                        blendfak=blendfakdef;
                        }

                        zeichnealles(sp);
                        for( x=1; x <= maxfeldsize; x ++)
                           for( z=1; z <= maxfeldsize; z ++)
                           {
                              sp.feld[x][z]=leer;
                              feldmark[x][z]=farbefeld;
                           }
                     }
                     break;

                  case 'q' : {
                     spielstate =quit;
                  }
                  break;

                  case '+' : if (sp.feldsizex<maxfeldsize) 
                  {
                     if (sp.feldsizex==mausfeldsizex)  mausfeldsizex += 1;
                     sp.feldsizex += 1; /*Spielfeld wird vergroessert*/
                     zeichnefgroesse(sp);
                  }
                  break;

                  case '-' : if (sp.feldsizex>minfeldsize) 
                  {
                     if (sp.feldsizex==mausfeldsizex)  mausfeldsizex -= 1;
                     sp.feldsizex -= 1; /*Spielfeld wird verkleinert*/
                     zeichnefgroesse(sp);
                  }
                  break;

                  case '*' : if (sp.feldsizey<maxfeldsize) 
                  {
                     if (sp.feldsizey==mausfeldsizey)  mausfeldsizey += 1;
                     sp.feldsizey += 1; /*Spielfeld wird vergroessert*/
                     zeichnefgroesse(sp);
                  }
                  break;

                  case '_' : if (sp.feldsizey>minfeldsize) 
                        {
                          if (sp.feldsizey==mausfeldsizey)  mausfeldsizey -= 1;
                            sp.feldsizey -= 1; /*Spielfeld wird verkleinert*/
                          zeichnefgroesse(sp);
                        }
                        break;


                  case 'w':  aenderzuordnung(sp,weissamzug); break;

                  case 'b':  aenderzuordnung(sp,schwarzamzug); break;

                  case '0':case '1':case '2': {
                                 mouseoff;
                                 sp.spielstufe=ord(char1)-48;
                                 setzespielstufe(sp.spielstufe,ok);
                                 zeichnebild(cpstbild,386,302,sp.spielstufe*30,30);
                                 mouseon;
                               }
                               break;

                }    /* case */
             }
             break;

        case weissamzug:case schwarzamzug : {
           mausposm.x=mauspos.x-320;
           mausposm.y=240-mauspos.y;
           if (rrelease) 
              char1='z';

           /*Wenn vismode=true, dann werden Mausklicks auf die Steuerungselemente in Tastendr¸cke umgesetzt.*/

           if (vismode && (mauspos.x>=560) && (mauspos.y<116)) 
           {
           mouseon;

           if (lrelease) 
                {
                   if (innerhalb(mauspos,560,0,640,16)) 
                      char1='c';
                   if (innerhalb(mauspos,560,16,640,32)) 
                      char1='C';
                   if (innerhalb(mauspos,560,32,640,48)) 
                      char1='t';
                   if (innerhalb(mauspos,560,48,607,64)) 
                      char1='z';
                   if (innerhalb(mauspos,607,48,640,64)) 
                      char1='v';
                   if (innerhalb(mauspos,607,96,640,116)) 
                      char1='q';

                   for( i=0; i <= 2;  i ++)
                   if (innerhalb(mauspos,600+i*10,72,616+i*10,96)) 
                   char1=chr(i+48);

                }
           }
           else
           if (pixeltoplane(sp,mausposm.x,mausposm.y,feldpos,
                        planepos,posrest,raum,1)==feld) 
           {
              mouseoff;
              if (lrelease) 
              {
                 if (((sp.feld[feldpos.x][feldpos.z]==leer) ||
                    (sp.feld[feldpos.x][feldpos.z]==tip)) &&
                    ((sp.zuordnung[weissamzug]==mensch) ||
                     (sp.zuordnung[schwarzamzug]==mensch))) 
                 {
                    if (sp.feld[setfeld.x][setfeld.z]==tip) 
                       sp.feld[setfeld.x][setfeld.z]=leer;
                    smilymod=0;
                    zeichnefeld(sp,setfeld,steinz);
                    sp.feld[feldpos.x][feldpos.z]=stein[sp.spielstate];
                    setfeld=feldpos;
                    smilymod=2;
                    zeichnefeld(sp,setfeld,steinz);
                    setzestein(feldpos.x,feldpos.z,ok);
                    spielstate=gegenspieler[sp.spielstate];
                    zugnummer += 1;
                    zeichnezugnummer(zugnummer);
                 }
              }
              if ((aktfeld.x!=feldpos.x) || (aktfeld.z!=feldpos.z)) 
                 bewegefeld(sp,aktfeld,feldpos);
           }
           else
              mouseon;
           switch (char1) {
             case '\0' : {

             /*Mit den Cursortasten kann man sich um das Spielfeld
              herumbewegen, der horizontale und der vertikale Winkel
              des Betrachters zum Spielfeld wird geaendert.*/

             switch (char2) {

               case '\110' : if (vwi>40) 
               {
                  vwi=vwi-5;zeichnealles(sp);
               }
               break;

               case '\120' : if (vwi<90) 
               {
                  vwi=vwi+5;zeichnealles(sp);
               }
               break;

               case '\115' :
             {
                hwi=hwi+10;
                zeichnealles(sp);
             }
             break;
               case '\113' :
             {
                hwi=hwi-10;
                zeichnealles(sp);
             }
             break;
             }    /* case */
          }
          break;

              case ' ':
                      {
                         dist = dist*0.9;
                         zeichnealles(sp);
                      }
                      break;
              case 'b':
                      {
                         dist = dist*1.111;
                         zeichnealles(sp);
                      }
                      break;
              case 'q':
                    {
                      prevkomplett=grs.komplett;
                      prevstate=spielstate;
                      spielstate =quitfrage;
                      mouseoff;
                      holehintergrund();
                      ladevordergrund("quitfrag.rle");
                      mouseon;
                    }
                    break;
           case 'x': {

           /*Vismode wird gewechselt, und es wird entweder das Steuerungsfeld gezeichnet
            oder es wird wieder Åberzeichnet.*/

              vismode= ! vismode;
              if (! vismode) 
              {
                 zeichneflaeche(sp,240,124,320,240,0,63,alles);
              }
              else
              {
                 ladebild(cpsttinybild,"cpsttiny.rle");
                 blendfak=1;
                 zeichnebild(steuerbild,560,0,0,116);
                 zeichnebild(cpsttinybild,600,84,sp.spielstufe*16,16);
                 blendfak=blendfakdef;
              }
           }
           break;

           case 'c': {
           /*Der Computer bekommt den Zug und fÅhrt ihn aus.*/
                  tipweg(sp);
                  sp.zuordnung[spielstate]=computer;
                  sp.zuordnung[gegenspieler[spielstate]]=mensch;
                  computerzug(sp.neugesetzt.x,sp.neugesetzt.z,ok);
                }
                break;

            case 'C': {
            /*Der Computer Åbernimmt beide Spieler und spielt so das Spiel zu Ende.*/
                   tipweg(sp);
                   sp.zuordnung[spielstate]=computer;
                   sp.zuordnung[gegenspieler[spielstate]]=computer;
                   computerzug(sp.neugesetzt.x,sp.neugesetzt.z,ok);
                 }
                 break;

           case '0':case '1':case '2': {
           /*Die Spielstufe wird geÑndert.*/
                          mouseoff;
                          sp.spielstufe=ord(char1)-48;
                          setzespielstufe(sp.spielstufe,ok);
                          if (vismode) 
                          {
                            blendfak=1;
                            zeichnebild(cpsttinybild,600,84,sp.spielstufe*16,16);
                            blendfak=blendfakdef;
                          }
                          mouseon;
                       }
                       break;

           case 'z': {
           /*Der letzte Zug wird zurÅckgenommen.*/
                  mouseoff;
                  tipweg(sp);
                  zugzurueck(setfeld.x,setfeld.z,ok);
                  if (ok) 
                  {
                    sp.feld[setfeld.x][setfeld.z]=leer;
                    zeichnefeld(sp,setfeld,steinz);
                    sp.zuordnung[spielstate]=mensch;
                    if (sp.neugesetzt.x==0) 
                    {
                       spielstate=gegenspieler[sp.spielstate];
                    zugnummer -= 1;
                    zeichnezugnummer(zugnummer);
                    }
                    sp.neugesetzt.x=0;
                    sp.zuordnung[weissamzug]=mensch;
                    sp.zuordnung[schwarzamzug]=mensch;
                  }
                }
                break;

            case 'v': {
            /*Es wird wieder ein zurÅckgenommener Zug gesetzt.*/

                   zugvor(x,z,ok);
                   if (ok) 
                   {
                     zugnummer += 1;
                     zeichnezugnummer(zugnummer);
                     smilymod=0;
                     zeichnefeld(sp,setfeld,steinz);
                     setfeld.x=x;
                     setfeld.z=z;
                     sp.feld[x][z]=stein[sp.spielstate];
                     smilymod=2;
                     zeichnefeld(sp,setfeld,steinz);
                     spielstate=gegenspieler[sp.spielstate];
                   }
                 }
                 break;

             case 't': {

             /*Der Computer gibt einen Tip entsprechend seiner momentanen Spielstufe.*/
                    tipweg(sp);
                    computerzug(x,z,ok);
                    setfeld.x=x;
                    setfeld.z=z;
                    sp.feld[x][z]=tip;
                    zugzurueck(x,z,ok);
                    zugzurueck(x,z,ok);
                    if (ok) 
                      setzestein(x,z,ok);
                    else
                      neuesspiel(ok);
                    smilymod=3;
                    zeichnefeld(sp,setfeld,steinz);
                  }
                  break;

           }    /* case */
        }
        break;



        case quitfrage :
                 {
                    mouseon;
                    if (rrelease)  char1='z';
                    if (lrelease) 
                       {
                       if (innerhalb(mauspos,440,340,490,370)) 
                          char1='q';
                       if (innerhalb(mauspos,150,340,210,370)) 
                          char1='z';
                       if (innerhalb(mauspos,250,340,390,370)) 
                          char1='n';
                       }

                    switch (char1) {
                      case 'q' :
                    {
                       spielstate = quit;
                    }
                    break;
                      case 'n' :
                    {
                       mouseoff;
                       zeichnehintergrund();
                       spielstate = startvor;
                    }
                    break;
                      case 'z' :
                    {
                       mouseoff;
                       zeichnehintergrund();
                       spielstate = prevstate;
                       grs.komplett = prevkomplett;
                    }
                    break;
                    }    /* case */

                 }
                 break;

        case ende  :
           {
              mouseoff;
              if (sp.status==unentschieden1) 
              {
                 holehintergrund();
                 ladevordergrund("remi.rle");
              }
              else
              {

                if (sp.status == eins_gewinnt) 
                  farbe=120;
                else
                  farbe=255;
                /*Markiert die Gewinnreihe mit Smileys.*/
                gibgewinnreihe(ax,ay,ex,ey,ok);
                ok=false;
                smilymod=1;
                while (! ok) 
                {
                  feldmark[ax][ay]=farbe;
                  setfeld.x=ax;
                  setfeld.z=ay;
                  zeichnefeld(sp,setfeld,steinz);
                  ok=(ax==ex) && (ay==ey);
                  ax=ax+sgn(ex-ax);
                  ay=ay+sgn(ey-ay);
                }

                 holehintergrund();
                 ladebild(weissschwarzbild,"weischw.rle");
                 blendfak=0.334;
                 ladevordergrund("sieg.rle");

                 if (sp.status==eins_gewinnt) 
                 {
                    zeichnebild(weissschwarzbild,205,223,0,28);
                 }
                 else
                 {
                    zeichnebild(weissschwarzbild,205,223,31,27);
                 }
               }
               blendfak=blendfakdef;
              spielstate=ende2;
           }
           break;

        case ende2 :
            {
               mouseon;
               if (rrelease)  char1='z';
               if (lrelease) 
               {
                  if (innerhalb(mauspos,560,48,607,64) && vismode) 
                     char1='z';
                  if (innerhalb(mauspos,440,340,540,370)) 
                     char1='q';
                  if (innerhalb(mauspos,150,340,300,370)) 
                     char1='n';
               }
               switch (char1) {
                 case 'n' :
               {
                  mouseoff;
                  zeichnehintergrund();
                  spielstate=startvor;
               }
               break;
                 case 'q' :
               {
                  spielstate=quit;
               }
               break;
                case 'z': {
                mouseoff;
                zeichnehintergrund();
                /*Lîscht die Gewinnreihensmileys*/
                if (sp.status!=unentschieden1) 
                {
                  gibgewinnreihe(ax,ay,ex,ey,ok);
                  ok=false;
                  smilymod=0;
                  while (! ok) 
                  {
                    feldmark[ax][ay]=farbefeld;
                    setfeld.x=ax;
                    setfeld.z=ay;
                    zeichnefeld(sp,setfeld,steinz);
                    ok=(ax==ex) && (ay==ey);
                    ax=ax+sgn(ex-ax);
                    ay=ay+sgn(ey-ay);
                  }
                }
                zugzurueck(x,z,ok);
                zugnummer -= 1;
                setfeld.x=x;
                setfeld.z=z;
                sp.feld[x][z]=leer;
                zeichnefeld(sp,setfeld,steinz);
                sp.zuordnung[spielstate]=mensch;

                sp.zuordnung[spielstate]=mensch;

                gibstatus(sp.status);
                switch (sp.status) {
                case eins_am_zug: spielstate=weissamzug; break;
                case zwei_am_zug: spielstate=schwarzamzug; break;

               }
               sp.zuordnung[spielstate]=mensch;
                }
                break; /*if*/
                }

              }
              break;
      }    /* case */
   }
   gui_result=spielstate;
   return gui_result;
}    /* Gui */


