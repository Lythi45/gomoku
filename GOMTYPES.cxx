#include "ptoc.h"

/*Sabine Wolf,5999219,q5999219@bonsai.fernuni-hagen.de*/

/*In dieser Unit werden die Typen deklariert, die von mehreren Units gebraucht
 werden.*/

#define __gomtypes_implementation__


#include "GOMTYPES.h"


class unit_gomtypes_initialize {
  public: unit_gomtypes_initialize();
};
static unit_gomtypes_initialize gomtypes_constructor;

unit_gomtypes_initialize::unit_gomtypes_initialize() {

stein[weissamzug]   = weiss;
stein[schwarzamzug] = schwarz;

gegenspieler[weissamzug]   = schwarzamzug;
gegenspieler[schwarzamzug] = weissamzug;

}






