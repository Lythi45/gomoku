#include "ptoc.h"

 /*Sabine Wolf,5999219,q5999219@bonsai.fernuni-hagen.de*/

/*Hauptprogram, ruft Gui in zwei Schleifen auf und ruft den
Computer auf, solange bis das Spiel zu Ende ist oder gequitet wurde.*/



#include "gomgui.h"
#include "gomtypes.h"
#include "q5999219.h"



tspiel spiel;
tspielstate spielstate;
tstatus status;
boolean ok;
int main(int argc, const char* argv[])
{

   pio_initialize(argc, argv);
   guiinit(spiel);
   setzespielstufe(1,ok);
   do {

/*Es wird solange die Benutzeroberflaeche aufgerufen, bis sich der
 Spielzustand durch eine Benutzeraktion aendert.*/

      do {
         spielstate = gui(spiel);
      } while  (!(spielstate!=spiel.spielstate));

      spiel.spielstate=spielstate;

      gibstatus(status);

      switch (spielstate) {

        case weissamzug:case schwarzamzug :
           {
              if ((status==eins_gewinnt) || (status==zwei_gewinnt) || (status==unentschieden1)) 
                 spielstate = ende;
           else
             if (spiel.zuordnung[spielstate]==computer) 
                computerzug(spiel.neugesetzt.x,spiel.neugesetzt.z,ok);

           }
           break;
      }    /* case */

      spiel.spielstate=spielstate;
      spiel.status=status;
      } while (!(spielstate==quit));

   return EXIT_SUCCESS;
}
