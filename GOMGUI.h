#ifndef __GOMGUI_h__
#define __GOMGUI_h__


#include "gomtypes.h"
#include "gomutil.h"
#include "q5999219.h"


void guiinit(tspiel& sp );
/*initialisiert die Unit-lokalen Variablen*/

tspielstate gui(tspiel& sp);
/*Die Haupt-Funktion dieser Unit, die auf Benutzereingaben reagiert und
 entsprechende Aktionen ausloest.*/

#endif
