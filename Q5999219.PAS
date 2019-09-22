(* Gomoku-Computerspieler von Sabine Wolf, q5999219@bonsai.fernuni-hagen.de, Version vom 7.9.02 *)

unit q5999219;

interface


type
tStatus = ( EINS_AM_ZUG, ZWEI_AM_ZUG,
                 EINS_GEWINNT, ZWEI_GEWINNT, UNENTSCHIEDEN,
                 UNDEFINIERT );



procedure gibStatus ( var Status: tStatus );

(* Der Status des aktuellen Spiels kann abgefragt werden.       *)
(* Am Anfang (ohne neuesSpiel) UNDEFINIERT.                     *)
(* Nach neuesSpiel EINS_AM_ZUG, spaeter nach setzeStein         *)
(* oder ComputerZug entsprechend EINS_AM_ZUG bzw. ZWEI_AM_ZUG.  *)
(* Wenn die Partie durch den letzten Zug beendet wurde, dann    *)
(* EINS_GEWINNT, ZWEI_GEWINNT oder UNENTSCHIEDEN.               *)



procedure neuesSpiel ( var ok: boolean );

(* Ein neues Spiel wird mit den aktuellen Einstellungen begonnen.     *)
(* ok = false, falls das aus irgendeinem Grund unmoeglich sein sollte.*)



procedure setzeStein ( x: integer; y: integer; var ok: boolean );

(* Es wird ein Stein auf das Brett gesetzt ( Koordinaten x,y )     *)
(* setzeStein kann beliebig oft HINTEREINANDER aufgerufen werden,  *)
(* z. B. um eine Stellung auf dem Brett darzustellen.              *)
(* Sollte dieser Zug nicht m=F6glich sein, ist ok = false.           *)



procedure computerZug ( var px: integer; var py: integer; var ok: boolean );

(* Der Computer macht einen Zug fÅr EINS oder ZWEI, wer grad am Zug ist.*)
(* computerZug kann beliebig oft HINTEREINANDER aufgerufen werden, der  *)
(* Computer kann also JEDERZEIT einen Zug fÅr die entsprechende Seite   *)
(* machen. Sollte kein Zug mîglich sein, ist ok = false.                *)



procedure zugZurueck ( var x: integer; var y: integer; var ok: boolean );

(* Die RÅcknahme eines Zuges. ok = false wenn nicht mîglich.       *)
(* zugZurueck kann beliebig oft hintereinander aufgerufen werden.  *)
(* zugZurueck nimmt NUR EINEN Zug zurÅck.                          *)



procedure zugVor ( var x: integer; var y: integer; var ok: boolean );

(* Ein Zug, der mit zugZurueck zur=FCckgenommen wurde, wird wieder=
 gesetzt.*)
(* zugVor macht KEINE NEUEN Z=DCGE. ok = false, falls kein Zug mehr       =
 *)
(* vorhanden ist. zugVor kann beliebig oft aufgerufen werden.            *)
(* zugVor setzt NUR EINEN Zug.                                           *)



procedure setzeSpielstufe ( Stufe: integer; var ok: boolean );

(* Die aktuelle Spielstufe des Computers wird gesetzt,    *)
(* ok = false bei illegaler Spielstufe.                   *)
(* Bei Eingabe einer illegalen Spielstufe wird            *)
(* die aktuelle Spielstufe beibehalten.                   *)



procedure gibSpielstufe ( var Stufe: integer );

(* Die aktuelle Spielstufe wird abgefragt.                *)
(* Vor dem ersten Aufruf von setzeSpielstufe ist Stufe=2. *)



procedure setzeFeldgroesse ( x: integer; y: integer; var ok: boolean );

(* Die Groesse des Spielfeldes wird gesetzt. Das Spielfeld ist rechteckig  *)
(* und besteht aus x mal y Kaestchen, beide zwischen 5 und 30.             *)
(* Bei Erfolg ist der Status = UNDEFINIERT, danach ist neuesSpiel          *)
(* notwendig, ok = false bei illegaler Feldgroesse.                        *)
(* Die aktuelle Feldgroesse wird bei einer                                 *)
(* illegalen Eingabe beibehalten, ebenso der Status.                       *)



procedure gibFeldgroesse ( var x: integer; var y: integer );

(* Die Gr=F6=DFe des Spielfeldes wird abgefragt.               *)
(* Vor dem ersten Aufruf von setzeFeldgroesse gilt x=y=18. *)


procedure gibGewinnreihe ( var x1: integer; var y1: integer;
                           var x2: integer; var y2: integer; var ok:boolean);

(* Der Anfangsstein x1,y1 und der Endstein x2,y2 einer Gewinnreihe wird *)
(* Uebergeben. ok = false falls noch kein Gewinner existiert.          *)



procedure gibNamen ( var name: string );

(* Vor- und Nachname der Programmiererin / des Programmierers wird   *)
(* uebergeben. Die Stringlaenge ist <= 30.                             *)


procedure gibSpitzNamen ( var name: string );
(* Der Spitzname des Programms, unter dem das Turnier gespielt wird. *)
(* Denken Sie sich etwas originelles/witziges aus.                   *)
(* Die Stringlaenge ist <= 30.                                      *)



implementation

const
   MINFELDSIZE = 5;
   MAXFELDSIZE = 30;
   DEFFELDSIZE = 13;

{Die maximale Anzahl potentieller ZÅge, unter denen ausgewÑhlt wird.}
   MAXAUSWAHL  =100;

type

{Die ZustÑnde die ein Feld haben kann.}

   tStein = (Leer,Weiss,Schwarz,Rand);

{Die Groesse, die das Feld haben kann.}

   tFeldSize   = MINFELDSIZE..MAXFELDSIZE;

{Eine Koordinate des Feldes}

   tFeldKoord  = 0..MAXFELDSIZE+1;

{Das Feldarray, das die Belegung des Spielfeldes mit Steinen speichert.}

   tFeld       = array[tFeldKoord,tFeldKoord] of tStein;

{Ein Zug mit seiner Position und seinem Status.}
   tZug = record
	     x	   : tFeldKoord;
	     y	   : tFeldKoord;
	     State : tStatus
	  end;
{Array zur Speicherung der ZÅge}
   tAZug = array [0..MAXFELDSIZE*MAXFELDSIZE] of tZug;
   tzAZug = ^tAZug;

   Offset	  = -1..1;

   tRichtung = record
		  x : Offset;
		  y : Offset;
	       end;

   tFeldPos = record
		 x : tFeldKoord;
		 y : tFeldKoord;
	      end;
   tBewertung = record
		 x : tFeldKoord;
		 y : tFeldKoord;
                 bewe : longint;
	      end;

{In solchen Arrays werden mîgliche Zugkandidaten gespeichert.}

   tsBewertungA = array[0..MAXAUSWAHL] of tBewertung;
   tBewertungA  = array[0..MAXFELDSIZE*MAXFELDSIZE] of tBewertung;

{Die 4 mîglichen Grundrichtungen}

   tAusrichtung = (waagerecht,senkrecht,vorschraeg,rueckschraeg);

var

{In diesem Array tragen sich Steine in ihre Umgebung ein, damit jedes Feld Åber seine
Umgebung Bescheid wei·.}
   Umgebung            : array[tFeldKoord,tFeldKoord,tAusrichtung,0..1] of integer;

{Zweierpotenzen fÅr die 4 Richtungen.}
   pfak                : array[tAusrichtung] of word;

{In dieses Array tragen sich Steine in ihre nÑhere Nachbarschaft ein, damit ein Feld schnell
wei·, ob es in der Nachbarschaft eines Steines liegt.}
   Nachbarn            : array[tFeldKoord,tFeldKoord] of word;

{Die Nummer des aktuellen Zuges und die Anzahl des gespeicherten ZÅge.
Im normalen Spiel sind beide Werte gleich, nur nach ZugrÅcknahme ist aktZug<endZug.}
   aktZug,endZug       : integer;

{Das Array, das die ZÅge speichert.}
   Zug		       : tzAZug;

   FeldSizeX,FeldSizeY : tFeldSize;
{Das Spielfeldarray, in dem die Steine eingetragen werden.}
   Feld		       : tFeld;

{EINS_AM_ZUG->Weiss,ZWEI_AM_ZUG->Schwarz}
   Steinzuordnung      : array[tStatus] of tStein;

{EINS_AM_ZUG->ZWEI_AM_ZUG,ZWEI_AM_ZUG->EINS_AM_ZUG}
   Flip		       : array [tStatus] of tStatus;

   Spielstufe	       : 0..2;

   rv :array[tAusrichtung] of tRichtung;
   AnfangsPos,EndPos : tFeldPos;

{Setzt einen Stein in das Feldarray und addiert den um die Entfernung mal 2
nach links geshifteten Steinwert(1 fÅr Wei·,2 fÅr Schwarz) auf die entsprechenden
Umgebungswerte der in alle 8 Richtungen liegenden Nachbarn bis zu einer Entfernung
von 5 Feldern. Zur schnellen öberprÅfung, ob ein Stein in der nÑheren Nachbarschaft
eines anderen liegt, wird der Stein noch ohne Unterscheidung der Steinfarbe
auf den Nachbarschaftswert der nÑheren Nachbarn addiert, jede der 8 Richtungen hat
eine entsprechende Bitposition dafÅr, fÅr die unmittelbaren Nachbarn wird das ganze nochmal um 8 Bits nach links geschoben,
so da· 16 Bits fÅr 16 Nachbarn verwendet werden.
}

procedure eintragen(x,y:tFeldKoord;stein:tStein);
var
   xx,yy,t,val : integer;
   ausr: tAusrichtung;

begin
   val:=ord(stein);
   Feld[x,y]:=stein;
   if stein<Rand then
      for ausr:=waagerecht to rueckschraeg do
      begin
	 inc(nachbarn[x-rv[ausr].x,y-rv[ausr].y],pfak[ausr]*256);
	 if (x-rv[ausr].x*2>0) and (y-rv[ausr].y*2>0) and
            (x-rv[ausr].x*2<=FeldSizeX) then
	    inc(nachbarn[x-rv[ausr].x*2,y-rv[ausr].y*2],pfak[ausr]);
	 inc(nachbarn[x+rv[ausr].x,y+rv[ausr].y],pfak[ausr]*512);
         if (x+rv[ausr].x*2>0) and
            (x+rv[ausr].x*2<=FeldSizeX) and (y+rv[ausr].y*2<=FELDSIZEY) then
	    inc(nachbarn[x+rv[ausr].x*2,y+rv[ausr].y*2],pfak[ausr]*2);
      end;
   for t:=1 to 5 do
   begin
      for ausr:=waagerecht to rueckschraeg do
      begin
	 xx:=x+rv[ausr].x*t;
	 yy:=y+rv[ausr].y*t;
	 if (xx>0) and (yy>0) and (xx<=FeldSizeX) and (yy<=FeldSizeY) then
	    inc(umgebung[xx,yy,ausr,0],val);
	 xx:=x-rv[ausr].x*t;
	 yy:=y-rv[ausr].y*t;
	 if (xx>0) and (yy>0) and (xx<=FeldSizeX) and (yy<=FeldSizeY)then
	    inc(umgebung[xx,yy,ausr,1],val);
      end;
{Der Steinwert wird um 2 Bits nach links geschoben.}
      val:=val*4;
   end;
end;



{Diese Prozedur trÑgt einen Stein wieder aus und macht alle von eintragen
vorgenommenen énderungen wieder rÅckgÑngig.}

procedure austragen(x,y:tFeldKoord;stein:tStein);
var
   xx,yy,t,val : integer;
   ausr: tAusrichtung;

begin
   val:=ord(stein);
   Feld[x,y]:=Leer;
   if stein<Rand then
      for ausr:=waagerecht to rueckschraeg do
      begin
	 dec(nachbarn[x-rv[ausr].x,y-rv[ausr].y],pfak[ausr]*256);
	 if (x-rv[ausr].x*2>0) and (y-rv[ausr].y*2>0) and
            (x-rv[ausr].x*2<=FeldSizeX) then
	    dec(nachbarn[x-rv[ausr].x*2,y-rv[ausr].y*2],pfak[ausr]);
	 dec(nachbarn[x+rv[ausr].x,y+rv[ausr].y],pfak[ausr]*512);
         if (x+rv[ausr].x*2>0) and
            (x+rv[ausr].x*2<=FeldSizeX) and (y+rv[ausr].y*2<=FELDSIZEY) then
	    dec(nachbarn[x+rv[ausr].x*2,y+rv[ausr].y*2],pfak[ausr]*2);
      end;

   for t:=1 to 5 do
   begin
      for ausr:=waagerecht to rueckschraeg do
      begin
	 xx:=x+rv[ausr].x*t;
	 yy:=y+rv[ausr].y*t;
	 if  (xx>0) and (yy>0)and (xx<=FeldSizeX) and (yy<=FeldSizeY) then
	    dec(umgebung[xx,yy,ausr,0],val);
	 xx:=x-rv[ausr].x*t;
	 yy:=y-rv[ausr].y*t;
	 if (xx>0) and (yy>0) and  (xx<=FeldSizeX) and (yy<=FeldSizeY)then
	    dec(umgebung[xx,yy,ausr,1],val);
      end;
      val:=val*4;
   end;
end;




procedure gibStatus ( var Status: tStatus );

begin;
   Status := Zug^[aktZug].State;
end;



{Initialisiert fÅr ein neues Spiel das Feld-, das Nachbarschaft- und
das Umgebungsarray, trÑgt den Rand in das Umgebungsarray ein,
setzt das Zugarray zurÅck und den Spielstatus auf EINS_AM_ZUG.}

procedure neuesSpiel(var ok:boolean);

var
   x,y	   : tFeldKoord;
   ausr: tAusrichtung;
begin;
   for x:= 0 to FeldSizeX+1 do
      for y := 0 to FeldSizeY+1 do
      begin
	 Feld[x,y]:=Leer;
	 nachbarn[x,y]:=0;
	 for ausr:=waagerecht to rueckschraeg do
	 begin
	    umgebung[x,y,ausr,0]:=0;
	    umgebung[x,y,ausr,1]:=0;
	 end;
      end;
   for x:= 0 to FeldSizeX+1 do
      for y := 0 to FeldSizeY+1 do
      begin
	 if (x=0) or (x=FeldSizeX+1) or (y=0) or (y=FeldSizeY+1) then
	    eintragen(x,y,Rand);
      end;
   aktzug:=0;
   endzug:=0;
   Zug^[aktZug].State := EINS_AM_ZUG;
   ok:=true;
end;



{Setzt einen Stein und schaut, ob sich durch das Setzen ein Sieg oder ein
Unentschieden ergeben hat.}

procedure setzeStein ( x: integer; y: integer; var ok: boolean );
var
   ausr	 : tAusrichtung;
   shift : integer;

begin;

if ((Zug^[aktZug].State=EINS_AM_ZUG) or (Zug^[aktZug].State=ZWEI_AM_ZUG)) AND
   (x>0) and (x<=FeldSizeX) and
   (y>0) and (y<=FeldSizeY) then
begin
   if Feld[x,y]=Leer then
   begin
   Zug^[aktzug].x := x;
   Zug^[aktZug].y := y;
   eintragen(x,y,Steinzuordnung[Zug^[aktzug].State]);
   Zug^[aktZug+1].State := Flip[Zug^[aktZug].State];
   inc(aktzug);
   endzug := aktzug;

{Wenn das Spielfeld voll ist, gibt es ein Unentschieden, es sei denn, es wird
noch ein Sieg festgestellt.}

   if aktzug=FeldSizeX*FeldSizeY then
      Zug^[aktZug].State:=UNENTSCHIEDEN;

{Es wird geschaut, ob sich durch das Setzen des Steins eine FÅnfer-Reihe ergeben hat.
Die der gesetzte Stein am Anfang, dazwischen oder am Ende einer solchen Reihe stehen kann,
werden alle 5 Positionen abgefragt.}

   for ausr := waagerecht to rueckschraeg do
   begin
      shift:=0;
      if ((umgebung[x,y,ausr,1] and 255)=85*ord(Feld[x,y])) then
	 shift:=1;
      if   (((umgebung[x,y,ausr,1] and 63)=21*ord(Feld[x,y])) and
	    ((umgebung[x,y,ausr,0] and 3)=ord(Feld[x,y])))    then
	 shift:=2;
      if   (((umgebung[x,y,ausr,1] and 15)=5*ord(Feld[x,y])) and
	    ((umgebung[x,y,ausr,0] and 15)=5*ord(Feld[x,y])))     then
	 shift:=3 ;
      if   (((umgebung[x,y,ausr,1] and 3)=ord(Feld[x,y])) and
	       ((umgebung[x,y,ausr,0] and 63)=21*ord(Feld[x,y])))   then
	    shift:=4  ;
      if  ((umgebung[x,y,ausr,0] and 255)=85*ord(Feld[x,y])) then
	       shift:=5;
      if shift>0 then
      begin
	 if Feld[x,y]=Weiss then
	    Zug^[aktZug].State:= EINS_GEWINNT
	 else
	    Zug^[aktZug].State:= ZWEI_GEWINNT;

{Je nach Positon des aktuellen Steines in der Siegreihe wird der Anfangs- und
der Endstein gesetzt.}

	 AnfangsPos.x:=x-rv[ausr].x*(shift-1);
	 AnfangsPos.y:=y-rv[ausr].y*(shift-1);
	 Endpos.x:=x+rv[ausr].x*(5-shift);
	 EndPos.y:=y+rv[ausr].y*(5-shift);
      end ;
   end;
   ok:=true;
   end
   else
   ok:=false
   end
   else
   ok:=false;
end;



{Der Computer ermittelt den fÅr in gÅnstigsten Stein und setzt diesen. Dabei geht er
je nach Spielstufe mehr oder weniger intelligent vor.}

procedure computerZug ( var px: integer; var py: integer; var ok: boolean);

var
   Computerstein,Spielerstein : tStein;
   auswahl,ost		      : integer;
   State		      : tStatus;
   qual			      : longint;
   nzuege        	      : tBewertungA;

{BesterZug ermittelt den besten Zug, bekommt dafÅr mitgeteilt,
bis zu welcher Tiefe ZÅge kalkuliert werden dÅrfen,,welche Farbe
der Computer und welche der Gegner hat,wie hoch die
Bewertungsschwelle ist, ab der gute Felder als solche erkannt und
wie stark zu verlÑngernde gegenÅber zu blockierenden Ketten gewichtet
werden sollen.}

function besterZug (var px,py	  : integer;
			Zugtiefe  : integer;
			cpst,spst : tStein;
			schwelle  : integer;
                        gewicht   : integer ) : longint;

var sumbewe,bewe,faktor,ri : integer;
   st			   : tStein;
   ausr			   : tAusrichtung;
   x,y,siegx,siegy         : tFeldKoord;
   xx,yy,xxx,yyy,t,
   nAusw,nNach,sieg,ssieg  : integer;
   zuege        	   : tsBewertungA;
   kett3,block3,block4     : boolean;
   skett1,sblock1,
   skett2,sblock2,
   skett3,skett3b,
   sblock3,sblock3b,
   sblock4: integer;
   maxqual                 : longint;

begin
   nNach:=0;
   sieg:=0;
   siegx:=1;
   siegy:=1;
   sblock3:=0;
   skett3:=0;
   sblock4:=0;
   for y:=1 to FeldSizeY do
      for x:=1 to FeldSizeX do
	 if (Feld[x,y]=Leer) and (nachbarn[x,y]>0) and (sieg<4000) then

{Wenn ein Feld leer ist und in der Umgebung eines anderen Steines liegt
(direkt drann oder mit einem Feld Abstand), wird es nÑher untersucht, ob
sich eigene Ketten bilden oder gegnerische Ketten abblocken lassen. Eine
gegebene Kette zu verlÑngern zÑhlt das dreifache wie das Abblocken einer
solchen Kette. LÑngere Ketten geben mehr Punkte, vîllig freie Ketten mehr
als die, die mit einem Stein schon geblockt werden kînnen.}

         begin
	    sumbewe:=0;
            block3:=false;
            kett3:=false;
            block4:=false;
            skett3b:=0;
            sblock3b:=0;
            skett1:=0;
            sblock1:=0;
            skett2:=0;
            sblock2:=0;
	    for st:=Weiss to Schwarz do
	    begin
	       ost:=ord(st);
	       if st=cpst then
                  faktor:=gewicht
               else
                  faktor:=1;
	       for ausr := waagerecht to rueckschraeg do
		  for ri:=0 to 1 do
		  begin
		     bewe:=0;

        {oo?..}      if ((umgebung[x,y,ausr,ri] and 15) = 5*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 15) = 0) then
			bewe:=1;
        {oo.?.}      if ((umgebung[x,y,ausr,ri] and 63) = 20*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 3) = 0) then
			bewe:=1;
        {oo..?}      if ((umgebung[x,y,ausr,ri] and 255) = 80*ost) then
			bewe:=1;

	{...o?.}     if ((umgebung[x,y,ausr,ri] and 255) = ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 3) = 0) then
			bewe:=1;
	{..o?..}     if ((umgebung[x,y,ausr,ri] and 63) = ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 15) = 0) then
			bewe:=1;
	{.o?...}     if ((umgebung[x,y,ausr,ri] and 15) = ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 63) = 0) then
			bewe:=1;
	{.o.?..}     if ((umgebung[x,y,ausr,ri] and 63) = 4*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 15) = 0) then
			bewe:=1;
        {.o..?.}     if ((umgebung[x,y,ausr,ri] and 255) = 16*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 3) = 0) then
			bewe:=1;

	{.oo?..}     if ((umgebung[x,y,ausr,ri] and 63) = 5*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 15) = 0) then
			bewe:=9;
	{..oo?.}     if ((umgebung[x,y,ausr,ri] and 255) = 5*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 3) = 0) then
			bewe:=9;
        {.oo.?.}     if ((umgebung[x,y,ausr,ri] and 255) = 20*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 3) = 0) then
			bewe:=9;
	{.o.o?.}     if ((umgebung[x,y,ausr,ri] and 255) = 17*ost) and
                        ((umgebung[x,y,ausr,1-ri] and 3) = 0)then
			bewe:=9;
	{..o?o.}     if ((umgebung[x,y,ausr,ri] and 63) = ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 15) = ost)then
			bewe:=9;
        {..o?o..}    if ((umgebung[x,y,ausr,ri] and 63) = ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 63) = ost)then
                     begin {wegen Symmetrie nur in einer Richtung werten.}
		       if ri=0 then bewe:=9 else bewe:=0;
                     end;
	{.o.?o.}     if ((umgebung[x,y,ausr,ri] and 63) = 4*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 15) = ost)then
			bewe:=9;
        {o.o?o}	     if ((umgebung[x,y,ausr,ri] and 63) = 17*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 3) = ost) then
			bewe:=9*5;
        {.o.o?o}     if ((umgebung[x,y,ausr,ri] and 255) = 17*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 3) = ost) then
			bewe:=0;
        {.o?o.o}     if ((umgebung[x,y,ausr,ri] and 15) = ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 63) = 17*ost) then
			bewe:=9*5;
	{.ooo?}	     if ((umgebung[x,y,ausr,ri] and 255) = 21*ost)  then
			bewe:=9*5;
	{ooo?.}	     if ((umgebung[x,y,ausr,ri]  and 63) = 21*ost) and
			((umgebung[x,y,ausr,1-ri] and 3) = 0) then
			bewe:=9*5;
        {ooo.?}	     if ((umgebung[x,y,ausr,ri]  and 255) = 84*ost) then
			bewe:=9*5;
	{oo?o.}	     if ((umgebung[x,y,ausr,ri] and 15) = 5*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 15) = ost) then
			bewe:=9*5;
	{oo?.o}	     if ((umgebung[x,y,ausr,ri] and 15) = 5*ost) and
                        ((umgebung[x,y,ausr,1-ri] and 15) = 4*ost)then
			bewe:=9*5;
        {oo.?o}	     if ((umgebung[x,y,ausr,ri] and 63) = 20*ost)  and
                        ((umgebung[x,y,ausr,1-ri] and 3) = ost)then
			bewe:=9*5;
        {.ooo.?}     if ((umgebung[x,y,ausr,ri]  and 1023) = 84*ost) then
			bewe:=0;

	{o.o?o.o}    if ((umgebung[x,y,ausr,ri] and 63) = 17*ost) and
                        ((umgebung[x,y,ausr,1-ri] and 63) = 17*ost) then
			bewe:=81;
	{o.oo?.o}    if ((umgebung[x,y,ausr,ri] and 255) = 69*ost) and
                        ((umgebung[x,y,ausr,1-ri] and 15) = 4*ost) then
			bewe:=81;
	{.ooo?.}     if ((umgebung[x,y,ausr,ri] and 255) = 21*ost) and
                        ((umgebung[x,y,ausr,1-ri] and 3) = 0) then
			bewe:=81;
	{.oo?o.}     if ((umgebung[x,y,ausr,ri] and 63) = 5*ost) and
                        ((umgebung[x,y,ausr,1-ri] and 15) = ost) then
			bewe:=82;
        {folgende 2 Positionen sind nur zum Blocken.}
        {.oo.o?}     if ((umgebung[x,y,ausr,ri] and 1023) = 81*ost) and
                        (st<>cpst) then
			bewe:=81;
        {.o.oo?}     if ((umgebung[x,y,ausr,ri] and 1023) = 69*ost) and
                        (st<>cpst) then
			bewe:=81;
	{oooo?}	     if (umgebung[x,y,ausr,ri] and 255) = 85*ost then
			bewe:=1000;
	{ooo?o}	     if ((umgebung[x,y,ausr,ri] and 15) = 5*ost) and
                        ((umgebung[x,y,ausr,1-ri] and 15) = 5*ost) then
			bewe:=1000;
	{oo?oo}	     if ((umgebung[x,y,ausr,ri] and 63) = 21*ost) and
                        ((umgebung[x,y,ausr,1-ri] and 3) = ost) then
			bewe:=1000;

	{.oooo?}     if ((umgebung[x,y,ausr,ri] and 1023) = 85*ost) and (st<>cpst) and (sieg<>3000) then
                     begin
                       sieg:=-4000;
                       siegx:=x;
                       siegy:=y;
                     end;

        {Wenn eine 4er-Kette verlÑngert werden kann,
         bedeutet das den sofortigen Sieg, Blocken ist zwingend.}
                     if (bewe=1000) then
                     begin
                       nNach:=0;
                       if (st=cpst) then
                       begin
                         sieg:=4000;
                         siegx:=x;
                         siegy:=y;
                       end
                       else
                         block4:=true;
                     end;
         {Eine freie Dreierreihe mu· geblockt oder verlÑngert werden.}
                     if bewe>=81 then
                     begin
                       if st=cpst then
                         kett3:=true
                       else
                         block3:=true
                     end;

         {Halboffene 3er-Ketten werden gezÑhlt.}
                     if (bewe=9*5) then
                     begin
                       if st=cpst then
		         inc(skett3b)
                       else
                         inc(sblock3b);
                     end;

         {Offene 2er-Ketten werden gezÑhlt.}
                     if (bewe=9) then
                     begin
                       if st=cpst then
		         inc(skett2)
                       else
                         inc(sblock2);
                     end;
         {Offene einzelne Steine werden gezÑhlt.}
                     if (bewe=1) then
                     begin
                       if st=cpst then
		         inc(skett1)
                       else
                         inc(sblock1);
                     end;
                     inc(sumbewe,bewe*faktor);
	         end;
	      end;

              if sieg<4000 then
                begin

                ssieg:=0;
                if (sblock3b>1) or ((sblock3b>0) and (sblock2>0)) then
                begin
                   block3:=true;
                   inc(sblock3);
                end;

       {ZÑhle zu verlÑngernde 3er-Kettenenden.}
              if kett3 then
              begin
                 inc(skett3);

       {Wenn die besten ZÅge eine freie 3er-Kette verlÑngern, bedeutet das auch den Sieg.}
                 if (sieg>=-3000) and (skett3>0) then
                    ssieg:=3000;
              end;
              if (skett1>1) and (sumbewe<9*faktor)then
                 inc(sumbewe,8*faktor);
              if (sblock1>1) and (sumbewe<9) then
                 inc(sumbewe,8);

       {ZÑhle zu blockierende 3er-Kettenenden.}
              if block3 then
              begin
                 inc(sblock3);
       {Wenn es zwei zu blockende 3er-Ketten gibt, es also mehr als 3 Enden gibt,
        die zu blockieren sind, bedeutet das die Niederlage.}
                 if (sieg=0) and (sblock3>3) then
                    ssieg:=-3000;

              end;

       {ZÑhle zu blockierende 4er-Kettenenden.}
              if block4 then
              begin
                 inc(sblock4);
       {Wenn es mehr als ein zu blockierendes 4er-Ketten-Ende gibt,
        so bedeutet da· eine Niederlage.}
                 if (sblock4>1) then
                    ssieg:=-4000;
              end;

       {Wenn ein Stein 2 offene Zweierketten oder halboffene 3er-Ketten verlÑngert, so
       bedeutet das den Sieg, wenn es nichts mit hîherer PrioritÑt gibt.}
              if (sieg=0) and (skett2+skett3b>1) and ((Spielstufe=2) or (skett3b>0)) then
                 ssieg:=2000;

       {Wenn ein Stein 2 halboffene 3er-Ketten verlÑngert, bedeutet auch das den Sieg.}
              if (sieg>=-3000) and (skett3b>1) then
                 ssieg:=3000;

              if (ssieg<>0) and (sblock4=0) then
              begin
                 sieg:=ssieg;
                 siegx:=x;
                 siegy:=y;
              end;
            end;

            if (sblock2>1) and (sumbewe<81) and (Spielstufe=2) then
               inc(sumbewe,9*4);

            if (block3) and (sumbewe<81) and (Spielstufe=1) then
               inc(sumbewe,9*4);

            if ((sblock3>0) and (sieg=2000)) or ((sblock4>0) and (sieg=3000)) then
              sieg:=0;

       {Wenn die Bewertung eines Feldes >= der momentanen Schwelle ist, wird des Feld in
        die Liste der mîglichen ZÅge eingetragen.}
	    if sumbewe>=schwelle then
	    begin
        {Wenn die Rekursionstiefe erreicht ist, werden nur die besten ZÅge eingetragen.}
	       if (Zugtiefe=0) then
	       begin
		  if (sumbewe>schwelle) then
		  begin
		     nNach:=0;
		     schwelle:=sumbewe;
		  end
	       end
         {Ansonsten werden ZÅge einer bestimmten Gruppe eingetragen. Wenn es z.B. einen
          Feld mit einer Bewertung von mind. 81 gibt, hei·t das, das dieses Feld unbedingt
          geblockt werden mu·, Felder mit einer niedrigeren Bewertung sind damit irrelevant.}
	       else
	       begin
		  while (sumbewe>=schwelle*9) do
		  begin
		     nNach:=0;
		     schwelle:=schwelle*9;
		  end
	       end;

               if nNach<MAXAUSWAHL then
               begin
	          zuege[nNach].x:=x;
	          zuege[nNach].y:=y;
	          zuege[nNach].bewe:=sumbewe;
               end
               else
               begin
               {Reicht der Platz im Array nicht aus, wird ein Feld Åberschrieben mit
                einer gewissen Wahrscheinlichkeit.}
                 auswahl:=random(nNach);
                 if auswahl<MAXAUSWAHL then
                 begin
                   zuege[auswahl].x:=x;
	           zuege[auswahl].y:=y;
	           zuege[auswahl].bewe:=sumbewe;
                 end;
               end;
               inc(nNach);
	    end;

	 end;
{Wurden keine guten Felder gefunden, so wird zufÑllig ein Feld ausgewÑhlt,
das in der Nachbarschaft eines Steines liegt.}

   if nNach=0  then
   begin
      for y:=1 to FeldSizeY do
	 for x:=1 to FeldSizeX do
	    if (Feld[x,y]=Leer) and (nachbarn[x,y]>255) then
		     begin
			nzuege[nNach].x:=x;
			nzuege[nNach].y:=y;
			inc(nNach);
		     end;
      auswahl:=random(nNach);
      px:=nzuege[auswahl].x;
      py:=nzuege[auswahl].y;
      besterZug:=0;
   end
   else
   begin
{Wenn die Stellung Sieg oder Niederlage bedeutet, wird ein sehr hoher Wert
 fÅr Sieg und ein sehr niedriger Wert fÅr Niederlage zurÅckgegeben.}
      if sieg<>0 then
      begin
	 px:=siegx;
	 py:=siegy;
	 besterZug:=sieg;
      end
      else
{Wenn die Zugtiefe erreicht ist, wird zufÑllig ein Feld aus der Liste
ausgewÑhlt, da alle diese Felder gleich gut bewertet wurden.}
	 if Zugtiefe=0 then
	 begin
	    auswahl:=random(nNach);
	    px:=zuege[auswahl].x;
	    py:=zuege[auswahl].y;
	    besterZug:=zuege[auswahl].bewe;
	 end
	 else
{Ansonsten wird jedes Feld aus der Liste erstmal gesetzt und nach dem besten Zug
gesucht, mit dem die andere Steinfarbe auf diesen Zug reagieren kînnte.
Die Felder, die in der Summe aus der zweifachen eigenen Bewertung minus der Bewertung
aus der besten mîglichen gegnerischen Reaktion darauf, am besten abschneiden werden genommen.
Der Faktor zwei ergab sich empirisch beim Spielen der Stufe 2 gegen Stufe 0.
}
	 begin
	    maxqual:=-5000;
	    nAusw:=0;

	    for t:=0 to nNach-1 do
	    begin
	       xx:=zuege[t].x;
	       yy:=zuege[t].y;
               eintragen(xx,yy,cpst);
               qual:=zuege[t].bewe*2*Zugtiefe-besterZug(xxx,yyy,Zugtiefe-1,spst,cpst,1,3);
	       if qual>=maxqual then
	       begin
		  if qual>maxqual then
		  begin
		     maxqual:=qual;
		     nAusw:=0;
		  end;
		  zuege[nAusw].x:=xx;
		  zuege[nAusw].y:=yy;
		  inc(nAusw);
	       end;

{ Der spekulativ gesetze Stein wird wieder zurÅckgenommen.}
	       austragen(xx,yy,cpst);

	    end;

{Aus den besten Feldern wird einer ausgewÑhlt.}
	    auswahl:=random(nAusw);
	    px:=zuege[auswahl].x;
	    py:=zuege[auswahl].y;
	    besterZug:=maxqual;
	 end;
     end;

end;

begin;

   State :=  Zug^[aktZug].State;

{Nur wenn das Spiel lÑuft, kann der Computer einen Zug machen.}

   if (State<>EINS_AM_ZUG) and (State<>ZWEI_AM_ZUG) then
      ok := false
   else
   begin
      ok:=true;

      Computerstein := Steinzuordnung[State];
      Spielerstein  := Steinzuordnung[Flip[State]];

{Beim ersten Zug wird fÅr die Spielstufen 0 und 1 ein zufÑlliges Feld
ausgewÑhlt, fÅr die Stufe zwei eines in der Mitte.}

      if aktZug=0 then
      begin
	 if Spielstufe<2 then
	 begin
	    px := random(FeldSizeX)+1;
	    py := random(FeldSizeY)+1;
	 end
	 else
	 begin
	    px := (FeldSizeX - random(2)) div 2 + 1;
	    py := (FeldSizeY - random(2)) div 2 + 1;
	 end
      end
      else
      begin
	 case Spielstufe of

{Nur Viererketten werden erkannt.}
	   0 : qual:=besterZug(px,py,0,Computerstein,Spielerstein,1000,2);

{Dreier und Viererketten werden erkannt.}
	   1 : qual:=besterZug(px,py,2,Computerstein,Spielerstein,81,2);

{auch kuerzere Ketten werden bewertet.}
	   2 : qual:=besterZug(px,py,2,Computerstein,Spielerstein,1,3);

	 end;
      end;

{Der ausgewÑhlte Stein wird gesetzt.}
      setzeStein(px,py,ok);

   end;
end;



{zieht einen Zug zurÅck, indem der vorherige Zug zum aktuellen gemacht wird
und dessen Stein zurÅckgenommen wird. Ist der aktuelle Zug allerdings der erste,
wird ok wird ok auf false gesetzt.x und y wird auf die Position des zurÅckgenommenen
Steines gesetzt.}

procedure zugZurueck ( var x: integer; var y: integer; var ok: boolean );

begin;
   if aktZug = 0 then
      ok := false
   else
   begin
      ok := true;
      dec(aktZug);
      x := Zug^[aktZug].x;
      y := Zug^[aktZug].y;
      austragen(x,y,Feld[x,y]);

   end;
end;




{zieht einen Zug vor, indem der aktuelle Stein wieder auf das Feld gesetzt
wird und der nÑchste Zug zum aktuellen gemacht wird. Ist der aktuelle Zug
allerdings schon der letzte, wird ok auf false gesetzt. x und y werden auf
die Position des gesetzten Steines gesetzt.}

procedure zugVor ( var x: integer; var y: integer; var ok: boolean );

begin;
   if aktZug = endZug then
      ok := false
   else
   begin
      ok := true;
      x := Zug^[aktZug].x;
      y := Zug^[aktZug].y;
      eintragen(x,y,Steinzuordnung[Zug^[aktZug].State]);
      inc(aktZug);

   end;
end;

{Setzt die Spielstufe.}

procedure setzeSpielstufe ( Stufe: integer; var ok: boolean );

begin;
   if (Stufe<0) or (Stufe>2) then
      ok := false
   else
   begin
      ok := true;
      Spielstufe:=Stufe;
   end;
end;



{Gibt die Spielstufe zurÅck.}

procedure gibSpielstufe ( var Stufe: integer );

begin;
   Stufe:=Spielstufe;
end;



{Setzt die Feldgrî·e.}

procedure setzeFeldgroesse ( x: integer; y: integer; var ok: boolean );

begin;
   if (x<MINFELDSIZE) or (x>MAXFELDSIZE) or (y<MINFELDSIZE) or (y>MAXFELDSIZE) then
      ok := false
   else
   begin
      ok := true;
      FeldSizeX:=x;
      FeldSizeY:=y;
      aktzug:=0;
      endzug:=0;
      Zug^[aktZug].State := UNDEFINIERT;

   end;
end;



{Gibt die Feldgrî·e zurÅck.}

procedure gibFeldgroesse ( var x: integer; var y: integer );

begin;
   x:=FeldSizeX;
   y:=FeldSizeY;
end;



{Gibt die Gewinnreihe zurÅck.}

procedure gibGewinnreihe (var x1 : integer; var y1: integer;
			  var x2 : integer; var y2: integer; var ok: boolean);

begin;
if (Zug^[aktZug].State=EINS_GEWINNT) or (Zug^[aktZug].State=ZWEI_GEWINNT) then
   begin
   x1:=AnfangsPos.x;
   y1:=AnfangsPos.y;
   x2:=EndPos.x;
   y2:=EndPos.y;
   ok := true;
   end
else
   ok := false;
end;



{Gibt meinen Namen zurÅck.}

procedure gibNamen ( var name: string );

begin;
   name := 'Sabine Wolf';
end;



{Gibt den Spitznamen des Computerspielers zurÅck.}

procedure gibSpitzNamen ( var name: string );

begin;
   name:='Neck-Breaker';
end;

{Variablen werden initialisiert und auf Defaultwerte gesetzt.}

begin

   FeldSizeX := DEFFELDSIZE;
   FeldSizeY := DEFFELDSIZE;
   Spielstufe := 2;


   aktZug := 0;
   endzug := 0;
   new(Zug);
   Zug^[aktZug].State := UNDEFINIERT;

   Steinzuordnung[EINS_AM_ZUG] := Weiss;
   Steinzuordnung[ZWEI_AM_ZUG] := Schwarz;

   Flip[EINS_AM_ZUG] := ZWEI_AM_ZUG;
   Flip[ZWEI_AM_ZUG] := EINS_AM_ZUG;
{Zufall wird initialisiert}
   randomize;

{die Vektoren der vier Richtungen}

   rv[waagerecht].x := 1;
   rv[waagerecht].y := 0;
   rv[senkrecht].x := 0;
   rv[senkrecht].y := 1;
   rv[vorschraeg].x := 1;
   rv[vorschraeg].y := 1;
   rv[rueckschraeg].x := -1;
   rv[rueckschraeg].y := 1;

{die den Richtungen zugeordneten Zweierpotenzen}

   pfak[waagerecht]:=1;
   pfak[senkrecht]:=4;
   pfak[vorschraeg]:=16;
   pfak[rueckschraeg]:=64;

end.
