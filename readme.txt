Sabine Wolf,5999219,q5999219@bonsai.fernuni-hagen.de

Das Programm beseteht im wesentlichen aus drei Teilen.

Das sehr knappe Hauptprogramm besteht aus einer Schleife, 
die dann terminiert, wenn ein Spiel zu Ende ist und keine 
Fortsetzung gewuenscht wurde oder es abgebrochen wurde. 
Innerhalb dieser Schleife wird solange die Gui-Routine 
aufgerufen (s.u.), bis sich der Zustand des Spieles 
geändert hat, weil der Benutzer eine Aktion ausgeführt hat,
oder ein Zustand, der nur zur Initialisierung diente, wieder 
verlassen wurde (StartVor und Ende sind solche Zustaende.) 
Wenn der Computer selber an der Reihe ist, so uebernimmt es 
das Hauptprogramm, den Computerzug aufzurufen, es sei denn,
der Benutzer der Benutzer gibt seinen Zug an den Computer
weiter, dann ruft gomgui den Computerzug auf. Ganz am Anfang 
wird noch GuiInit aufgerufen, um die lokalen Variablen der 
Gomgui-Unit zu initialisieren und das Intro anzuzeigen.
Nach jedem Zug wird die Q-Unit abgefragt, ob das Spiel
durch ein Sieg oder ein Unentschieden beendet wurde. 

Eine Auflistung des Spielzustaende:

Intro: Es wird bloß auf eine beliebige Aktion gewartet, 
       auf einen Mausklick oder ein Tastendruck, um dann 
       in den StartVor-Zustand zu wechseln.

StartVor: Der Startbildschirm wird aufgebaut und es wird zu
          Start gewechselt.

Start:    In diesen Zustand kann der Spieler Einstellungen 
          vornehmen zur Feldgroesse und dazu, wer Weiß 
          spielt und ob zwei Spieler gegeneinander oder 
          einer gegen den Computer spielen soll. Das Spiel 
          kann gestartet werden, dann wechselt der Zustand 
          zu WeissAmZug oder abgebrochen werden, dann 
          wechselt der Zustand zu Quit, wodurch das Spiel 
          terminiert. 

WeissAmZug,
SchwarzAmZug: In den beiden Spielzustaenden kann der oder 
              die Spieler, wenn sie denn drann sind, ein 
              freies Feld auswaehlen und auf dieses Feld 
              einen eigenen Stein setzen, woraufhin der 
              Zustand in den jeweils anderen wechselt, es 
              sei denn das Spiel ist zu Ende durch einen 
              Sieg oder ein Unentschieden (-> Ende). Das 
              Spiel kann auch abgebrochen werden, dann 
              erscheint eine Schirm mit einer Rueckfrage 
              und  der Zustand wechselt zu Quitfrage. 
              Ausserdem kann die Perspektive, mit der auf 
              das Spielfeld geschaut wird, geaendert werden,
              man kann sich um das Spielfeld herumbewegen 
              und naeher ran oder weiter weg gehen. 

QuitFrage: Der Spieler wird nochmal gefragt, ob er wirklich
           quitten will und kann dies bestaetigen -> Quit, 
           dies verneinen -> WeissAmZug bzw. SchwarzAmZug 
           oder ein neues Spiel anfangen -> StartVor.

Ende: Je nachdem, ob ein Sieg oder ein unentschieden 
      vorliegt, wird ein entsprechender Schreen gezeigt,
      im Falle eines Sieges wird ausserdem angezeigt, wer
      gewonnen hat. Wechselt sofort zu Ende.

Ende2: Es wird darauf gewartet, ob der Spieler ein neues 
       Spiel starten will oder das Spiel abbrechen will.
       Bricht er ab, so terminiert das Spiel mit dem Quit-
       Zustand, sonst geht es mit StartVor wieder auf den
       Startscreen. 

Ausser der Gui-Routine enthält die gomgui-Unit noch Routinen
die zum Aufbau der Grafik dienen, zum perspektivischen 
Darstellen des Spielfeldes und zum Einlesen und Darstellen 
von Screens und einzelnen Bildelementen, wie Ziffern oder 
der Anzeige von Dingen wie Weiss/Schwarz oder 
Mensch/Computer. Die Screens werden halbtransparent ueber
dem Spielfeld als Hintergrund dargestellt. Zu jeder Zeit im 
Spiel kann ein Screenshot von dem grad angezeigten Bild 
gemacht werden, diese Screenshot werden pro Sitzung laufend
nummeriert (fängt bei einem Neustart also wieder bei 1 an). 
Der Screenshot wird als TGA abgespeichert.

Die Unit gomtypes enthält Deklarationen von in mehreren Units 
gebrauchten Typen, insbesondere von  tSpiel.

Die Unit gomutil enthält einige Hilfsroutinen, um aus einer
Zahl einen String zu machen und noch einige Routinen für
Minimum- und Maximum-Werte. 
      

  
 




