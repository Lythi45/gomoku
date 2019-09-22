{Sabine Wolf,5999219,q5999219@bonsai.fernuni-hagen.de}

{In dieser Unit werden die Typen deklariert, die von mehreren Units gebraucht
 werden.}

unit gomtypes;

interface

uses q5999219;

const

   MINFELDSIZE = 5;
   MAXFELDSIZE = 30;

type

{Belegungsmoeglichkeiten fuer ein Feld}

   tStein      = (Leer,Weiss,Schwarz,Tip);

{Wie kann ein Spiel enden}

   tErgebnis   = (Sieg,Unentschieden);

{Zustaende in denen sich ein Spiel befinden kann}

   tSpielState = (Intro,StartVor,Start,WeissAmZug,SchwarzAmZug,
		  ZeigSieg,Ende,Ende2,QuitFrage,Quit);

{Die Groesse, die das Feld haben kann.}

   tFeldSize   = MINFELDSIZE..MAXFELDSIZE;

{Eine Koordinaten des Feldes}

   tFeldKoord  = 0..MAXFELDSIZE+1;

{Das Feldarray, das die Belegung mit Steines speichert.}

   tFeld       = array[tFeldKoord,tFeldKoord] of tStein;

{Die beiden Typen von Spielern}

   tSpieler    = (Mensch,Computer);

{Zuordnung von Spielertyp zu den Spielsteinfarben.}

   tZuordnung  = array[tSpielState] of tSpieler;

{Position auf dem Feld}

   tFeldPos = record
		 x : integer; {tFeldKoord;}
		 z : integer; {tFeldKoord;}
	      end;

{Urteil ueber das Spiel, wenn es zu ende ist}

   tBewertung  =  record
		     typ   : tErgebnis;
		     stein : tStein;
		  end;

{Spielstrukturtyp, der den Zustand, in dem sich das Spiel befindet,
 zusammenfasst.}

   tSpiel      =  record
		     Feld	: tFeld;
		     FeldSizeX	: integer; {tFeldSize;}
                     FeldSizeY  : integer; {tFeldSize;}
		     SpielState	: tSpielState;
                     status     : tStatus;
		     Zuordnung	: tZuordnung;
		     Bewertung	: tBewertung;
                     Spielstufe : integer;
                     neuGesetzt : tFeldPos;
		  end;

var

{Zuordnung Spielzustand -> Steinfarbe}
   stein : array[tSpielState] of tStein;
{Der jeweilige Gegenspieler}
   Gegenspieler : array[tSpielState] of tSpielState;

implementation

begin

stein[WeissAmZug]   := Weiss;
stein[SchwarzAmZug] := Schwarz;

Gegenspieler[WeissAmZug]   := SchwarzAmZug;
Gegenspieler[SchwarzAmZug] := WeissAmZug;

end.






