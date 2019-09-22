#ifndef __Q5999219_h__
#define __Q5999219_h__



enum tstatus { eins_am_zug, zwei_am_zug,
                 eins_gewinnt, zwei_gewinnt, unentschieden,
                 undefiniert , last_tstatus};



void gibstatus ( tstatus& status );

/* Der Status des aktuellen Spiels kann abgefragt werden.       */
/* Am Anfang (ohne neuesSpiel) UNDEFINIERT.                     */
/* Nach neuesSpiel EINS_AM_ZUG, spaeter nach setzeStein         */
/* oder ComputerZug entsprechend EINS_AM_ZUG bzw. ZWEI_AM_ZUG.  */
/* Wenn die Partie durch den letzten Zug beendet wurde, dann    */
/* EINS_GEWINNT, ZWEI_GEWINNT oder UNENTSCHIEDEN.               */



void neuesspiel ( boolean& ok );

/* Ein neues Spiel wird mit den aktuellen Einstellungen begonnen.     */
/* ok = false, falls das aus irgendeinem Grund unmoeglich sein sollte.*/



void setzestein ( integer x, integer y, boolean& ok );

/* Es wird ein Stein auf das Brett gesetzt ( Koordinaten x,y )     */
/* setzeStein kann beliebig oft HINTEREINANDER aufgerufen werden,  */
/* z. B. um eine Stellung auf dem Brett darzustellen.              */
/* Sollte dieser Zug nicht m=F6glich sein, ist ok = false.           */



void computerzug ( integer& px, integer& py, boolean& ok );

/* Der Computer macht einen Zug fÅr EINS oder ZWEI, wer grad am Zug ist.*/
/* computerZug kann beliebig oft HINTEREINANDER aufgerufen werden, der  */
/* Computer kann also JEDERZEIT einen Zug fÅr die entsprechende Seite   */
/* machen. Sollte kein Zug mîglich sein, ist ok = false.                */



void zugzurueck ( integer& x, integer& y, boolean& ok );

/* Die RÅcknahme eines Zuges. ok = false wenn nicht mîglich.       */
/* zugZurueck kann beliebig oft hintereinander aufgerufen werden.  */
/* zugZurueck nimmt NUR EINEN Zug zurÅck.                          */



void zugvor ( integer& x, integer& y, boolean& ok );

/* Ein Zug, der mit zugZurueck zur=FCckgenommen wurde, wird wieder=
 gesetzt.*/
/* zugVor macht KEINE NEUEN Z=DCGE. ok = false, falls kein Zug mehr       =
 */
/* vorhanden ist. zugVor kann beliebig oft aufgerufen werden.            */
/* zugVor setzt NUR EINEN Zug.                                           */



void setzespielstufe ( integer stufe, boolean& ok );

/* Die aktuelle Spielstufe des Computers wird gesetzt,    */
/* ok = false bei illegaler Spielstufe.                   */
/* Bei Eingabe einer illegalen Spielstufe wird            */
/* die aktuelle Spielstufe beibehalten.                   */



void gibspielstufe ( integer& stufe );

/* Die aktuelle Spielstufe wird abgefragt.                */
/* Vor dem ersten Aufruf von setzeSpielstufe ist Stufe=2. */



void setzefeldgroesse ( integer x, integer y, boolean& ok );

/* Die Groesse des Spielfeldes wird gesetzt. Das Spielfeld ist rechteckig  */
/* und besteht aus x mal y Kaestchen, beide zwischen 5 und 30.             */
/* Bei Erfolg ist der Status = UNDEFINIERT, danach ist neuesSpiel          */
/* notwendig, ok = false bei illegaler Feldgroesse.                        */
/* Die aktuelle Feldgroesse wird bei einer                                 */
/* illegalen Eingabe beibehalten, ebenso der Status.                       */



void gibfeldgroesse ( integer& x, integer& y );

/* Die Gr=F6=DFe des Spielfeldes wird abgefragt.               */
/* Vor dem ersten Aufruf von setzeFeldgroesse gilt x=y=18. */


void gibgewinnreihe ( integer& x1, integer& y1,
                               integer& x2, integer& y2, boolean& ok);

/* Der Anfangsstein x1,y1 und der Endstein x2,y2 einer Gewinnreihe wird */
/* Uebergeben. ok = false falls noch kein Gewinner existiert.          */



void gibnamen ( string& name );

/* Vor- und Nachname der Programmiererin / des Programmierers wird   */
/* uebergeben. Die Stringlaenge ist <= 30.                             */


void gibspitznamen ( string& name );
/* Der Spitzname des Programms, unter dem das Turnier gespielt wird. */
/* Denken Sie sich etwas originelles/witziges aus.                   */
/* Die Stringlaenge ist <= 30.                                      */



#endif
