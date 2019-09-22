 {Sabine Wolf,5999219,q5999219@bonsai.fernuni-hagen.de}

{Hauptprogram, ruft Gui in zwei Schleifen auf und ruft den
Computer auf, solange bis das Spiel zu Ende ist oder gequitet wurde.}

program gomoku;

uses
   gomgui,gomtypes,q5999219;


var
   Spiel      : tSpiel;
   SpielState : tSpielState;
   status: tStatus;
   ok : boolean;
begin

   GuiInit(Spiel);
   setzeSpielstufe(1,ok);
   repeat

{Es wird solange die Benutzeroberflaeche aufgerufen, bis sich der
 Spielzustand durch eine Benutzeraktion aendert.}

      repeat
         SpielState := Gui(Spiel);
      until  SpielState<>Spiel.SpielState;

      Spiel.SpielState:=SpielState;

      gibStatus(status);

      case SpielState of

        WeissAmZug,SchwarzAmZug :
           begin
              if (status=EINS_GEWINNT) or (status=ZWEI_GEWINNT) or (status=UNENTSCHIEDEN) then
                 SpielState := Ende
           else
             if Spiel.Zuordnung[SpielState]=Computer then
                computerZug(Spiel.neuGesetzt.x,Spiel.neuGesetzt.z,ok);

           end;
      end; { case }

      Spiel.SpielState:=SpielState;
      Spiel.status:=status;
      until SpielState=Quit

end.
