{Sabine Wolf,5999219,q5999219@bonsai.fernuni-hagen.de, Version vom 6.9.0.2}

{Diese Unit erzeugt alle grafischen Objekte und reagiert auf den Benutzer.}

unit gomgui;

interface

uses
   gomtypes,gomutil,q5999219;

procedure GuiInit(var sp :tSpiel );
{initialisiert die Unit-lokalen Variablen}

function Gui(var sp:tSpiel) : tSpielState;
{Die Haupt-Funktion dieser Unit, die auf Benutzereingaben reagiert und
 entsprechende Aktionen ausloest.}

implementation

uses graph,simpmaus,crt;

const
   MINY		 = -240;
   MAXY		 = 240;
   MINX		 = -320;
   MAXX		 = 320;
   MITSX	 = 320;
   MITSY	 = 239;
   VWINKEL	 = 65;
   HWINKEL	 = 190;
   FARBEHIMMEL	 = 255;
   FARBEEBENE	 = 170;
   FARBERAND	 = 53;
   FARBEFELD	 = 204;
   FARBEWMARKFELD= 238;
   FARBESMARKFELD= 170;
   STEINWEISS	 = 127;
   STEINSCHWARZ	 = 40;
   STEINGRAU     = 86;
   BLENDFAKDEF	 = 0.667;

type

   tXScreenKoord= MINX..MAXX;
   tYScreenKoord= MINY..MAXY;

   tFlaeche     = (Reject,Himmel,Ebene,Rand,Feld);

   tZeichModus  = (Feldz,Steinz,Alles);

{Typ fuer die Position auf dem Bildschirm}

   tPixel = record
               x : integer;
               y : integer;
            end;

{Typ fuer die Position auf der Spielfeldebene}

   tPlanePos = record
                     x : Real;
                     z : Real;
                  end;

{Typ fuer die Position eines Punktes im Raum}

   tRaumpos = record
                 x : Real;
                 y : Real;
                 z : Real;
              end;

{Typ fuer die Perspektiv-Parameter}

   tGraphState  = record
                     vx,
                     vy,
                     vz,
                     lx,
                     ly,
                     vxx,
                     vxz,
                     vyx,
                     vyy,
                     vyz,
                     rad     : real;
                     basis   : tPlanePos;
                     stomax,
                     stomix,
                     stomaz,
                     stomiz  : integer;
                     komplett: integer;
                     fdy,
                     fddy    : real;
                  end;

{Struktur der Bildresourcen}

   tBildarray   = array[0..64000] of Byte;

   tzBildarray  = ^tBildarray;

   tZeilenArray = array[0..350] of integer;

   tBild = record
              Name    : String;
              Bild    : tzBildarray;
	      Zeile   : tZeilenArray;
              XSize   : integer;
              YSize   : integer;
              XAnfang : integer;
              YAnfang : integer;
             end;

   tHintergrund = array[0..191,0..287] of Byte;
   tzHintergrund = ^tHintergrund;
   tziffa = array[0..28,0..579] of Byte;
   tzziffa = ^tziffa;
   tsmily = array[0..49,0..149] of Byte;
   tzsmily = ^tsmily;
   tFarbe = real;

{Typen fuer die Ordered-Dither-Matrizen}

   t0_7   = 0..7;
   tr0_7  = record
              x,y : t0_7;
            end;

var
{Der grafische Zustand, in dem alles an Positionen und Parameter gespeichert
 wird, was zur Erzeugung der Grafik wichtig ist}

   GrS                  : tGraphState;

{Die Farben der Steine}

   cst                  : array[tStein] of tFarbe;

{Der Horizontal-,der Vertikalwinkel und der Abstand des Beobachters
 zum Spielfeld}

   vwi,hwi,hz,dist           : real;

{Die Feldgroesse, wie sie mit der Maus eingestellt wurde.}

   MausFeldSizeX,MausFeldSizeY        : integer;

{Die Farbe der Felder, das grad markierte Feld ist heller als die anderen}

   FeldMark :array[tFeldKoord,tFeldKoord] of integer;

{Die benoetigten Bilder}

   ZiffernBild,
   Ziffern2Bild,
   DotBild,
   CompSpielerBild,
   WeissSchwarzBild,
   SteuerBild,
   CpStBild,
   CpStTinyBild        : tBild;
   Hintergrund         : tzHintergrund;

{Vorheriger Spielzustand, um nach einem abgelehnten Quit wieder zum
 richtigen Zustand zurueckzukommen.}

prevState:tSpielState;
prevKomplett:integer;

{Ordered-Dither-Tabellen}

   orddith: array[t0_7,t0_7] of real;
   orddtab: array[0..63] of tr0_7;

{aktuell markiertes Feld}

aktFeld,setFeld:tFeldPos;

{Anzahl der bisher gemachten Screenshots.}

ScreenshotNum:integer;

Blendfak : real;

ziffarray : tzziffa;
smily : tzsmily;
smilymod:integer;
vvismode,vismode: boolean;

Zugnummer : integer;

WSpieler,SSpieler : tSpieler;

idle: boolean;

{PlaneToPixel ermittelt, auf welchem Pixel der Mittelpunkt eines Feldes
 liegt und gibt eine grobe, aber positive Abschaetzung, wieviel Pixel
 das Feld abdeckt.}

function PlaneToPixel(    sp : tSpiel;
                        feld : tFeldPos;
                     var pix : tPixel    ):integer;

var nenn,bx,bz : real;

begin
   bx:=feld.x+GrS.basis.x;
   bz:=feld.z+GrS.basis.z;
   with GrS do
   begin
   nenn:=ly*(bz * vxx * vyy -
                   bx * vxz * vyy +
                   vx * vxz * vyy +
                   vy * vxx * vyz -
                   vy * vyx * vxz -
                   vz * vxx * vyy);
   pix.x:=round(-( bx *  vz * vyy -
                   bx *  vy * vyz -
                   bz *  vx * vyy +
                   bz *  vy * vyx) /nenn);
   pix.y:=round(-( bx * vy * vxz -
                   bz * vy * vxx) /nenn);

      planetopixel := trunc(1500/sqrt(sqr(vy)+sqr(vx-bx)+sqr(vz-bz)));
   end;
end; { planetopixel }





{PixelToPlane ermitteln den Punkt, in dem der Sehstrahl vom Beobachter
 durch das Pixel die Spielebene trifft, und zu welchem Feld dieser Punkt
 gehoert. Die Spielebene erstreckt sich in X- und in Z-Richtung und liegt
 auf Y=-Steinradius, damit sie grade unter den Steinen liegt, deren
 Mittelpunkt auf Y=0 liegt.}

function PixelToPlane(   var  sp  : tSpiel;
                              pixx,pixy : real;
                          var pos : tFeldPos;
                          var posp,
                              posr: tPlanePos;
                          var rpos: tRaumpos;
                              level: integer    ):tFlaeche;

begin
   with GrS do
      begin
   rpos.x := -vx/ly + pixx*vxx + pixy*vyx;
   rpos.y := -vy/ly + pixy*vyy;
   rpos.z := -vz/ly + pixx*vxz + pixy*vyz;

   if rpos.y<0 then
   begin
      posp.x:=vx-rpos.x*(vy+rad)/rpos.y;
      posp.z:=vz-rpos.z*(vy+rad)/rpos.y;
      if  (level<4) and
          (abs(posp.x)<sp.FeldSizeX/2+0.3+fdy+fddy*pixy) and
          (abs(posp.z)<sp.FeldSizeY/2+0.3+fdy+fddy*pixy) and not
         ((abs(posp.x)<sp.FeldSizeX/2+0.3-fdy-fddy*pixy) and
          (abs(posp.z)<sp.FeldSizeY/2+0.3-fdy-fddy*pixy)) then
          PixelToPlane:=Reject
      else
      if  (abs(posp.x)<sp.FeldSizeX/2+0.3) and
          (abs(posp.z)<sp.FeldSizeY/2+0.3) then
          if (level<4) and
          (abs(posp.x)<sp.FeldSizeX/2+fdy+fddy*pixy) and
          (abs(posp.z)<sp.FeldSizeY/2+fdy+fddy*pixy) and not
         ((abs(posp.x)<sp.FeldSizeX/2-fdy-fddy*pixy) and
          (abs(posp.z)<sp.FeldSizeY/2-fdy-fddy*pixy)) then
          PixelToPlane:=Reject
      else
      begin
         pos.x:=round(posp.x-basis.x);
         pos.z:=round(posp.z-basis.z);
         if (pos.x>0) and (pos.x<=sp.FeldSizeX) and
            (pos.z>0) and (pos.z<=sp.FeldSizeY) then
         begin
            posr.x:=(posp.x-basis.x)-pos.x;
            posr.z:=(posp.z-basis.z)-pos.z;

            PixelToPlane:=Feld;
         end
         else
            PixelToPlane:=Rand;
      end
      else
         PixelToPlane:=Ebene;
   end
   else
      PixelToPlane:=Himmel;
      end
end; { pixeltoplane }





{PutPixS reduziert die Farbe eines Pixel auf die darstellbaren Farben
 mittels Ordered-Dither-Verfahren und setzt dieses Pixel}

procedure PutPixS(x,y : integer;va:real);

begin
   putpixel(x,y,trunc(va/17+orddith[(x+800) mod 7,(y+800) mod 7]));
end; { PutPixS }





{PutPix rechnet die zentrierten Koordinaten, wie sie fÅr die Grafik
verwendet werden in Screen-Koordinaten um, und ruft PutPixS auf.}

procedure PutPix(pix : tPixel;va:real);

begin
   PutPixS(pix.x+MITSX,MITSY-pix.y,va);
end;






{BerechnePixel berechnet ein Pixel in der Szene mit dem Spielfeld}

function BerechnePixel(var sp :tSpiel; pixx,pixy:real;modus:tZeichModus;Level:integer):real;

var
   kx,kz            : tFeldKoord;
   bx,bz            : real;
   col              : real;
   a,b,c,d,t,hell   : real;
   kux,kuy,kuz      : real;
   ab               : real;
   pixtyp           : tFlaeche;
   feldpos          : tFeldPos;
   planepos,posrest : tPlanePos;
   raum             : tRaumPos;
   indexx,indexz,
   zif,zx,zz        : integer;
   ps,versatz        : real;


begin

   {Es wird ueberprueft, wo und ob ein Sehstrahl vom Beobachter durch das
    entsprechende Pixel auf dem Bildschirm auf die Spielebene trifft.
    Fuer Himmel und Ebene bekommt das Pixel eine entsprechende Farbe,
    fuer das Spielfeld mit Rand wird genauer geschaut, welchen Farbwert
    das Pixel bekommt.}

   pixtyp := pixeltoplane(sp,pixx,pixy,feldpos,planepos,posrest,raum,level);

   col:=1000;

   if (not (modus=Feldz)) or
   ((feldpos.x=aktFeld.x) and (feldpos.z=aktfeld.z)) then
   begin
            case pixtyp of
              Reject    : col:=998;
              Himmel    : col:=FARBEHIMMEL;
              Ebene     : col:=FARBEEBENE;
              Rand,Feld :
              begin
                 {Fuer jedes Feld- oder Rand-Pixel wird ueberprueft, ob ein
                  auf dem zugehoerigen Feld oder in der Umgebung liegender
                  Stein zu sehen ist, dessen Farbe dann genommen wird}

                 for kx := max(0,feldpos.x+GrS.stomix) to
                           min(sp.FeldSizeX+1,feldpos.x+GrS.stomax) do
                    for kz := max(0,feldpos.z+GrS.stomiz) to
                              min(sp.FeldSizeY+1,feldpos.z+GrS.stomaz)  do
                    begin
                       if  (sp.Feld[kx,kz]<>Leer) then
                       begin

                          {kux,kuy,kuz sind die Koordinaten des
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
                           liegende gebraucht).}

                          kux := kx + GrS.basis.x;
                          kuz := kz + GrS.basis.z;
                          kuy := 0;
                          with GrS do
                             begin
                                a := sqr(raum.x)+sqr(raum.y)+sqr(raum.z);
                                b := 2*((vx-kux)*raum.x+
                                     (vy-kuy)*raum.y+(vz-kuz)*raum.z);
                                c := sqr(vx)+sqr(vy)+sqr(vz)-
                                     2*vx*kux-2*vy*kuy-2*vz*kuz+
                                     sqr(kux)+sqr(kuy)+sqr(kuz)-sqr(rad);
                                d := b*b-4*a*c;
                                ps:=-b/(2*a);
                                if (level<4) and (abs(d)*level<0.0006*ps) then
                                begin
                                   col:=998;
                                end
                                else
                                {Wenn d>0 dann liegt der Stein im Sehstrahl}
                                if d > 0 then
                                begin
                                   if (modus=Feldz) or ((modus=Steinz) and ((kx<>setFeld.x)or(kz<>setFeld.z))) then
                                      col:=999
                                   else
                                   begin
                                   {t gibt die Position des Punktes auf dem
                                    Sehstrahl an, bei t=0 laege er im
                                    Betrachter, bei t=1 im Pixel}

                                   t := (-b-sqrt(d))/(2*a);

                                   {Der Stein ist oben hell,unten dunkel mit
                                    einem cosinusfoermigen Verlauf}
                                   hell := 1-cos((vy+t*raum.y+rad)/rad/2*3.1415);

                                   {Die Pixelfarbe ergibt sich aus der
                                    Grundfarbe mal der Helligkeit.
				    Wenn smilymod>0 und dieses Feld=setFeld,
                                    dann wird ein Smiley bzw. ein Kreuz
                                    auf den Stein gezeichnet.}

                                   if (smilymod>0) and (setFeld.x=kx) and (setFeld.z=kz) then
                                   begin
                                     if level<4 then
                                        col:=998
                                     else
                                       if sp.Feld[kx,kz]=Weiss then
                                         col := maxr(0,hell*(cst[sp.Feld[kx,kz]])-255+
                                         smily^[trunc((vx+t*raum.x-kux)/rad*25+25),
                                         trunc(-(vz+t*raum.z-kuz)/rad*25-25++smilymod*50)])
                                       else
                                         col := minr(255,hell*(cst[sp.Feld[kx,kz]])+170-
                                         smily^[trunc((vx+t*raum.x-kux)/rad*25+25),
                                         trunc(-(vz+t*raum.z-kuz)/rad*25-25+smilymod*50)]*2/3)
                                   end
                                   else
                                   col := hell*cst[sp.Feld[kx,kz]];
                                   end;
                                end;
                             end
                       end
                    end;

                 {Wenn kein Stein im Sehstrahl liegt, dann wird weitergeschaut}

                 if col=1000 then
                 begin
                    if pixtyp=Rand then
                       col:=FARBERAND
                    else
                    begin

                       { Es wird geschaut, ob der Sehstrahl auf eine
                         Fuge trifft, Fuge wird vom Rand hin zur Mitte dunkler}
                       if (level<4) and
                          (((abs(posrest.x)>0.47-GrS.fdy-GrS.fddy*pixy) and
                            (abs(posrest.x)<0.47+GrS.fdy+GrS.fddy*pixy)) or
                            (abs(posrest.x)>0.5-GrS.fdy-GrS.fddy*pixy)) then
                        col:=996
                        else
                        if (level<4) and
                          (((abs(posrest.z)>0.47-GrS.fdy-GrS.fddy*pixy) and
                            (abs(posrest.z)<0.47+GrS.fdy+GrS.fddy*pixy)) or
                            (abs(posrest.z)>0.5-GrS.fdy-GrS.fddy*pixy)) then
                           col:=997
                       else
                       begin
                       if abs(posrest.x)>0.47 then
                          col:=FeldMark[feldpos.x,feldpos.z]+
			       (0.47-abs(posrest.x))*2500;
                       if abs(posrest.z)>0.47 then
                          col:=minr(col,FeldMark[feldpos.x,feldpos.z]+
				        (0.47-abs(posrest.z))*2500);
                       {Wenn keine Fuge, dann bekommt das Pixel die Farbe
                        fuer ein Feld bzw. ein grade markiertes Feld, wobei
                        noch geschaut wird, ob das Pixel im Schatten eines
                        evtl. drauf liegenden Steines liegt. Der Schatten ist
                        kreisrund mit einem cosinusfoermigen Fall-off am Rand.}

                       if col = 1000 then
                       begin
                          col:=FeldMark[feldpos.x,feldpos.z];
                          if  sp.Feld[feldpos.x,feldpos.z] <> Leer then
                          begin

                             {Schattenmittelpunkt etwas versetzt zur Mitte}

                             bx:=posrest.x-0.02;
                             bz:=posrest.z+0.02;

                             {Abstand zur Mitte}

                             ab:=sqrt(bx*bx+bz*bz);

                             {Fall-off zwischen Radius+0.1 und Radius-0.1}

                             if (ab<GrS.rad+0.1) then
                                if (ab<GrS.rad-0.1) then
                                   col:=col/3
                                else
                                   col:=col/3+col*2/6*
                                        (1-cos((ab-GrS.rad+0.1)/0.2*3.1415))
                             end;
                          end
                       end
                    end;
                 end;
              end
            end
   end;

   case  trunc(col) of
   998: begin
          versatz:=0.25/level;
          col:=(BerechnePixel(sp,pixx-versatz,pixy-versatz,Alles,level*2)+
                BerechnePixel(sp,pixx-versatz,pixy+versatz,Alles,level*2)+
                BerechnePixel(sp,pixx+versatz,pixy-versatz,Alles,level*2)+
                BerechnePixel(sp,pixx+versatz,pixy+versatz,Alles,level*2))/4;
        end;
   996: begin
          versatz:=0.25/level;
          col:=(BerechnePixel(sp,pixx-versatz,pixy,Alles,level*2)+
                BerechnePixel(sp,pixx+versatz,pixy,Alles,level*2))/2;
        end;
   997: begin
          versatz:=0.25/level;
          col:=(BerechnePixel(sp,pixx,pixy-versatz,Alles,level*2)+
                BerechnePixel(sp,pixx,pixy+versatz,Alles,level*2))/2;
        end;
   end;
   BerechnePixel:=col;

end; { ZeichnePixel }



{ZeichnePixel zeichnet ein Pixel in der Szene mit dem Spielfeld}

procedure ZeichnePixel(var sp :tSpiel; pix :tPixel;modus:tZeichModus);
var
   col:real;
begin
   col:=BerechnePixel(sp,pix.x,pix.y,modus,1);
   if col<256 then
      PutPix(pix,col);
end;



{ZeichneFlaeche zeichnet einen Ausschnitt der Szene bzw. die ganze Szene.
 Die Flaeche wird  aufgeteilt in 64 Teile,so da es auch m"glich ist, nur
 einen Teil auf einmal zu zeichnen. Jeder Teil deckt den ganzen Bereich ab,
 hat aber in X- und in Y-Richtung nur jedes 8. Pixel fuer sich. Jeder Teil hat
 einen anderen Versatz, der in ordtab steht. Zeichne ich ein neues Bild
 ergibt sich durch das Zeichen der 64 Teile eine Art Ueberblendeffekt.}

procedure ZeichneFlaeche(var sp          : tSpiel;
                             ax,ay,ex,ey : integer;
                             von,bis     : integer;
                             modus       : tZeichModus );

var
   i,randx      : integer;
   pix          : tPixel;

begin
   MouseOff;
   for i:=von to bis do
   begin
      pix.y := ay-((ay+320) mod 8)+orddtab[i].y;
      while pix.y<ey do
      begin

	 {Wenn vismode=true, dann wird die rechte, obere Screen-Ecke
          freigehalten f¸r das Steuerungsfeld.}

	 if (pix.y<124) or (not vismode) then
           randx:=320
         else
           randx:=240;

	 pix.x := ax-((ax+320) mod 8)+orddtab[i].x;

         while pix.x<min(ex,randx) do
         begin
            ZeichnePixel(sp,pix,modus);
            inc(pix.x,8);
         end;
         inc(pix.y,8);
      end;
   end;
   MouseOn;
end; { ZeichneFlaeche }





{ZeichneFeld zeichnet den Bereich, den ein Feld abdeckt.}

procedure ZeichneFeld(var sp  : tSpiel;
                         pos  : tFeldPos;
                         modus: tZeichModus);

var
   pix :tPixel;
   gr  : integer;

begin

   {Es wird eine Flaeche neugezeichnet, die so gross ist, dass das Feld
    auf jeden Fall ganz drinnliegt, es sei denn GrS.komplett ist null,
    weil dann das ganze Bild eh neugezeichnet wird.}

   if GrS.komplett>0 then
   begin
      gr := PlaneToPixel(sp,pos,pix);
      ZeichneFlaeche(sp,pix.x-gr, pix.y-gr, pix.x+gr,pix.y+gr,0,
      GrS.komplett-1,modus);
   end;

end; { ZeichneFeld }





{BewegeFeld setzt das markierte Feld neu und zeichnet den Bereich,
 der sich geaendert hat neu. (Genauer das Rechteck, das beide
 Feldbereiche einschliesst.)}

procedure BewegeFeld (var sp   : tSpiel;
                      var von  : tFeldPos;
                      var nach : tFeldPos);

var

   pixvon,pixnach   : tPixel;
   grvon,grnach     : integer;

begin
   {Die Markierung wechselt vom von-Feld zum nach-Feld}

   grnach := PlaneToPixel(sp,nach,pixnach);
   if von.x>0 then
   begin
      FeldMark[von.x,von.z]:=FARBEFELD;
      ZeichneFeld(sp,von,Feldz);
   end;
   von:=nach;
   if sp.SpielState = WeissAmZug then
      FeldMark[nach.x,nach.z]:=FARBEWMARKFELD
   else
      FeldMark[nach.x,nach.z]:=FARBESMARKFELD;
   ZeichneFeld(sp,nach,Feldz);

end; { BewegeFeld }




{Testet, ob ein Pixel innerhalb eines Bereiches liegt.}

function innerhalb(pos :tPixel;
                   ax,ay,ex,ey : integer):boolean;
begin

   innerhalb:=(pos.x>=ax) and (pos.x<ex) and
              (pos.y>=ay) and (pos.y<ey);

end; { innerhalb }




{Zeichnet die ganze Szene neu.}

procedure ZeichneAlles(var sp      : tSpiel);
var
   pixtyp           : tFlaeche;
   feldpos          : tFeldPos;
   planepos,planepos2,
   posrest          : tPlanePos;
   raum             : tRaumPos;
   diff1,diff2      : real;

begin
   {Aus den Werten fuer den Horizontal- und Vertikalwinkel und dem
    Abstand des Beobachters zum Spielfeldmittelpunkt werden die
    Position des Beobachters und Parameter fuer die Lage der Pixelflaeche
    (eine Art Fenster, durch das der Beobachter auf das Spielfeld schaut)
    berechnet.}

   MouseOff;
   with GrS do
   begin
       vy := sin(vwi/180*3.1415)*dist;
       hz := cos(vwi/180*3.1415)*dist;
       vx := hz*sin(hwi/180*3.1415);
       vz := hz*cos(hwi/180*3.1415);
       lx := sqrt(sqr(vx)+sqr(vz));
       ly := sqrt(sqr(vx)+sqr(vy)+sqr(vz));
      vxx :=-vz/lx*0.0005;
      vxz := vx/lx*0.0005;
      vyx :=-vy/ly*vx/lx*0.0005;
      vyy := lx/ly*0.001666;
      vyz :=-vy/ly*vz/lx*0.0005;
      rad := 0.4;
      basis.x:=-sp.FeldSizeX/2-0.5;
      basis.z:=-sp.FeldSizeY/2-0.5;

      {Schaut man schraeg auf das Feld, kann es sein, dass im
       Bereich eines Feldes mehrere Steine zu sehen sind, deshalb
       muss ich die Nachbarschaft dann noch mit einbeziehen.}

      stomax:=0;
      stomix:=0;
      stomiz:=0;
      stomaz:=0;

      if vx/dist> 0.2 then stomax:= 1;
      if vx/dist<-0.2 then stomix:=-1;
      if vz/dist> 0.2 then stomaz:= 1;
      if vz/dist<-0.2 then stomiz:=-1;

      {Es wird ermittelt, um wieviel sich etwa die Feldkoordinaten pro Pixel Ñndern.}
      pixtyp := pixeltoplane(sp,0,-240,feldpos,planepos,posrest,raum,1);
      pixtyp := pixeltoplane(sp,0,-239,feldpos,planepos2,posrest,raum,1);

      diff1:=sqrt(sqr(planepos.x-planepos2.x)+sqr(planepos.z-planepos2.z));

      pixtyp := pixeltoplane(sp,0,239,feldpos,planepos,posrest,raum,1);
      pixtyp := pixeltoplane(sp,0,240,feldpos,planepos2,posrest,raum,1);
      diff2:=sqrt(sqr(planepos.x-planepos2.x)+sqr(planepos.z-planepos2.z));

      fdy:=(diff2+diff1)/2/2;
      fddy:=(diff2-diff1)/480/2;

      {Bild wird als nicht gezeichnet markiert, damit es mit jedem
       Aufruf von Gui ein Stueck weit neu gezeichnet wird.}

      komplett:=0;

   end;
end; { ZeichneAlles }





{LadeBild laedt ein Bild, indem es dafuer Speicher anfordert und das Bild
 dann in einer tBild-Variablen ablegt. Das Bild ist run-length-encoded,
 um Speicher zu sparen.}

procedure LadeBild(var bild     : tBild;name:String);

var
   bfile : file of byte;
   nx,ny : integer;
   p	 : ^Byte;
   hb,lb : byte;
   len,i,zn,po	 : integer;
   co	 : byte;

begin

   if bild.Bild=nil then
      begin
         assign(bfile,name);
         reset(bfile);
	 read(bfile,hb,lb);
	 len:=hb*256+lb;
	 read(bfile,hb,lb);
	 nx:=hb*256+lb;
	 read(bfile,hb,lb);
	 ny:=hb*256+lb;
         if MaxAvail<len then
            begin
               writeln('Nicht genug Speicher!');
               halt;
            end;
         GetMem(p,len);
         bild.Bild:=tzBildArray(p);

	 for i:=0 to len-1 do
	    read(bfile,bild.Bild^[i]);

	 zn:=0;i:=0;po:=0;
	 bild.Zeile[0]:=0;

	 repeat
	    co:=bild.Bild^[i];
	    if (co and 1)=1 then
	    begin
               inc(po,co div 2);
               inc(i,2);
            end
	    else
	    begin
               inc(po);
               inc(i);
            end;
	    if po>=nx then
	    begin
	       inc(zn);
	       bild.Zeile[zn]:=i;
       	       po:=0;
	    end
	 until i>=len;
	 bild.XSize:=nx;
         bild.YSize:=ny;
         bild.Name:=name;
      end;
end; { LadeBild }





{Mischt einen Bildpunkt mit dem Hintergrund.
 (Genauer gesagt, der Hintergrund wird auf 1-mix abgesenkt,
 dazu kommen mix-mal Weiss, und das wird dann mit dem Bild
 multipliziert. Schwarze Teile des Bildes bleiben also schwarz.)}

procedure BildMix(kx,ky : integer;co:Byte;mix:real);
begin
     if mix=1 then
	       PutPixS(kx,ky,co)
     else
	if (kx and 1)=0 then
           PutPixS(kx,ky,trunc(co*((Hintergrund^[(kx-128) div 2,
                        ky-96] div 16)/15*(1-mix)+mix)))
	else
	   PutPixS(kx,ky,
		       trunc(co*((Hintergrund^[(kx-128) div 2,
		        ky-96] and 15)/15*(1-mix)+mix)));
end;





{Laedt ein Vordergrundbild und stellt es direkt dar.}

procedure LadeVordergrund(name:String);

var
   bfile : file of byte;
   nx,ny : integer;
   x,y	 : integer;
   hb,lb : byte;
   len	 : integer;
   anb	 : byte;
   co	 : byte;
begin
         assign(bfile,name);
         reset(bfile);
	 read(bfile,hb,lb);
	 len:=hb*256+lb;
	 read(bfile,hb,lb);
	 nx:=hb*256+lb;
	 read(bfile,hb,lb);
	 ny:=hb*256+lb;
	 anb:=0;

         for y:=0 to ny-1  do
            for x:= 0 to nx-1 do
	    begin
	       if anb=0 then
	       begin
		  read(bfile,co);
		  if (co and 1)=1 then
		  begin
		     anb:=co div 2;
		     read(bfile,co)
		  end
		  else
		     anb:=1;
	       end;
	       BildMix(x+128,y+96,co+1,BLENDFAK);
	       dec(anb);
            end;

         close(bfile);


end;



{HoleHintergrund speichert den mittleren Teil des Bildes in Hintergrund.}

procedure HoleHintergrund;

var
   x,y      : integer;

begin

   {Es werden immer 2 Pixel in einem Byte gespeichert, da sonst das Array
    groesser als 64k waere.}

   for y := 0 to 287 do
      for x := 0 to 191 do
         Hintergrund^[x,y]:=getpixel(x*2+128,y+96)*16+
	                             getpixel(x*2+129,y+96);


   GrS.komplett:=64;
end; { HoleHintergrund }




{Zeichnet den gespeicherten Hintergrund.}

procedure ZeichneHintergrund;

var
   x,y      : integer;

begin
   for y    := 0 to 287 do
      for x := 0 to 191 do
	 begin
         PutPixel(x*2+128,y+96,Hintergrund^[x,y] div 16);
	 PutPixel(x*2+129,y+96,Hintergrund^[x,y] and 15);
	 end;

end; { ZeichneHintergrund }





{ZeichneBild zeichnet die gewuenschten Zeilen eines Bild an gewuenschter
 Stelle ueber den Hintergrund.}

procedure ZeichneBild(bild     : tBild;
                      XAnfang,
                      YAnfang  : integer;
                      YBAnfang : integer;
                      YBSize   : integer
                      );

var
   x,y	 : integer;
   za	 : integer;
   co,anb	 : Byte;
begin
   anb:=0;
   co:=0;
   za:=bild.Zeile[YBAnfang];
   for y:=0 to YBSize-1 do
   begin
      for x:=0 to bild.XSize-1 do
      begin
	 if anb=0 then
	 begin
	    co:=bild.Bild^[za];
	    inc(za);
	    if (co and 1)=1 then
	    begin
	       anb:=co div 2;
	       co:=bild.Bild^[za];
	       inc(za);
	    end
	    else
	       anb:=1;
	 end;
	 BildMix(XAnfang+x,YAnfang+y,co+1,BLENDFAK);
	 dec(anb);
      end;
   end;

end;




{GuiInit initialisiert alle noetigen Variablen}

procedure GuiInit(var sp :tSpiel );

var
   t,po,x,y,z,nx,
   nz,pp,i,col,
   gd , gm,  error : integer;
   va,ami,mi           : real;
   dfile           : text;
   ox,oy           : t0_7;
   p               :^Byte;
   bfile:  file of byte;
   farbe : byte;
   ok: boolean;

begin
{Zeiger auf Bildspeicher werden auf nil gesetzt.}
  DotBild.bild:=nil;
  ZiffernBild.bild:=nil;
  Ziffern2Bild.bild:=nil;
  CompSpielerBild.bild:=nil;
  WeissSchwarzBild.bild:=nil;
  SteuerBild.bild:=nil;
  CpStBild.bild:=nil;
  CpStTinyBild.bild:=nil;



   {Ordered-Dither-Matrix wird erzeugt, um mehr Farben als vorhanden
    zu koennen. Resultat ist eine 8x8-Matrix, die mit einer Grauwert-Skala
    gefuellt ist, gegen die dann beim Setzen eines Pixels verglichen wird}

   po:=1;
   pp:=4;
   va:=1/64;
   orddith[0,0]:=0.0;
   for t:=1 to 3 do
   begin
      for x:=0 to po-1 do
         for y:= 0 to po-1 do
         begin
            orddith[x*pp*2+pp,y*pp*2+pp]:= orddith[x*pp*2,y*pp*2]+va*po*po;
            orddith[x*pp*2+pp,y*pp*2]:=    orddith[x*pp*2,y*pp*2]+va*po*po*2;
            orddith[x*pp*2,y*pp*2+pp]:=    orddith[x*pp*2,y*pp*2]+va*po*po*3;
         end;
      po:=po*2;pp:=pp div 2;
   end;

   {Die Ordered-Dither-Werte werden mit ihrer Position noch in eine Tabelle
    einsortiert, wird zum Ueberblenden spaeter benutzt}
   orddtab[0].x:=0;
   orddtab[0].y:=0;
   ami:=0;
   for i:=1 to 63 do
   begin
      mi:=99.9;
      for oy:=0 to 7 do
         for ox:= 0 to 7 do
            if (orddith[ox,oy]>ami) and (orddith[ox,oy]<mi) then
            begin
               mi:=orddith[ox,oy];
               orddtab[i].x:=ox;
               orddtab[i].y:=oy;
            end;
      ami:=mi;
   end;

   {Den Steinen werden Farbwerte zugeordnet}

   cst[Leer]:=0;
   cst[Weiss]:=STEINWEISS;
   cst[Schwarz]:=STEINSCHWARZ;
   cst[Tip]:=STEINGRAU;

   {Das aktuell markierte Feld wird auf (0,0) gesetzt.}

   aktFeld.x:=0;
   aktFeld.z:=0;

   {Die Anzahl der Screenshots wird auf 0 gesetzt.}

   ScreenshotNum:=0;
   for x:=0 to MAXFELDSIZE +1 do
      for z:=0 to MAXFELDSIZE +1 do
      begin
         sp.Feld[x,z]:=Leer;
         FeldMark[x,z]:=FARBEFELD;
      end;

   assign(dfile,'gomoku.pnm');
   reset(dfile);
   readln(dfile,nx);
   readln(dfile,nz);
   for z := nz downto 1 do
      for x := 1 to nx do
      begin
         readln(dfile,col);
         if IOResult<>0 then halt;
         if col<128 then
            if  random(2)=0 then
               sp.Feld[x,z]:=Schwarz
            else
               sp.Feld[x,z]:=Weiss;
      end;
   close(dfile);

   {Die Grafik wird initialisiert}

   gd:=Detect;
   gm:=0;
   initgraph ( gd , gm, '') ;
   error := graphResult ;
   if ( error <> grOk ) Then
   begin
      writeln ( 'Grafik wird nicht unterstuetzt! ' ) ;
      halt (1)
   end;

   {Die Palette bekommt einen Grauverlauf}

     for y := 0 to 63 do
         SetRGBPalette(y,y*4,y*4,y*4);

     SetColor(15);
     DirectVideo:=False;

   {Initialisiere Hintergrundbild}

     for y:=0 to 15 do
     SetPalette(y,y+16);

   new(Hintergrund);

   if not MouseReset then
   begin
      writeln('Konnte Maus nicht initialisieren');
   halt;
   end;

   sp.FeldSizeX:=19;
   sp.FeldSizeY:=19;
   MausFeldSizeX:=sp.FeldSizeX;
   MausFeldSizeY:=sp.FeldSizeY;


      {Defaultansicht von schraeg oben,Abstand haengt von der Feldgroesse ab.}

   vwi := VWinkel;
   hwi := HWinkel;
   dist:= 4.5*max(sp.FeldSizeX,sp.FeldSizeY);


   {Grafikaufbau wird angestossen}

   ZeichneAlles(sp);


   sp.Zuordnung[WeissAmZug]   := Computer;
   sp.Zuordnung[SchwarzAmZug] := Mensch;
   sp.SpielState := Intro;
   setzeSpielStufe(1,ok);

   {kein zu setzender Stein}

   sp.neugesetzt.x:=0;

   idle:=false;

   Blendfak := BLENDFAKDEF;

   new(smily);
   assign(bfile,'smily.raw');
   reset(bfile);
   for y:=0 to 149 do
      for x:=0 to 49 do
      begin
         read(bfile,farbe);
         smily^[x,y]:=farbe;
      end;
   close(bfile);
   setfeld.x:=0;
   setfeld.z:=0;
   vismode:=false;
   smilymod:=0;

end; { GuiInit }




procedure ZeichneZuordnung(sp:tSpiel;spst : tSpielState);

var
   y:integer;

begin
   if spst= WeissAmZug then
      y:=183
   else
      y:=214;
   if sp.Zuordnung[spst]=Mensch then
      ZeichneBild(CompSpielerBild,310,y,31,34)
   else
      ZeichneBild(CompSpielerBild,310,y,0,34);
end;





{Die Zuordnung Mensch/Computer zu Schwarz/Weiss wird geaendert.}

procedure AenderZuordnung(var  sp : tSpiel;spst : tSpielState);

begin
   MouseOff;

   if sp.Zuordnung[spst]=Mensch then
   begin
      sp.Zuordnung[spst]:=Computer;
   end
   else
      sp.Zuordnung[spst]:=Mensch;
      ZeichneZuordnung(sp,spst);
end; { AenderZuordnung }





{Ein schwarzer Punkt wird gezeichnet.}

procedure ZeichneDot(x,y:integer);
begin
   ZeichneBild(DotBild,x,y,0,31);
end;





{Die Feldgroesse wird mit einer Zahl angezeigt.}

procedure ZeichneFGroesse(var sp :  tSpiel);

var
   zehner,einer,FS,py :  integer;

begin

   MouseOff;
FS:=sp.FeldSizeX;
for py:=0 to 1 do
begin
   zehner:=FS div 10;

   if zehner>0 then
      ZeichneBild(ZiffernBild,372,247+py*27,zehner*31,31)
   else
      ZeichneBild(ZiffernBild,372,247+py*27,310,31);
   einer:=FS-zehner*10;
      ZeichneBild(ZiffernBild,386,247+py*27,einer*31,31);
   ZeichneDot(390+FS*3,245+py*27);
FS:=sp.FeldSizeY;
end;

end;





{Ein Abbild des Screens wird als TGA-Bild abgespeichert.}

procedure Screenshot;

var
   bfile : text;
   x,y   : integer;
   farbe : byte;
   s : string;
begin

   inc(ScreenshotNum);
   assign(bfile,'scshot'+intstr(ScreenshotNum)+'.tga');

   rewrite(bfile);
   s:=#0#0#3#0#0#0#0#0#0#0#0#0#128#2#224#1#8#8;
   write(bfile,s);
   for y:=479 downto 0 do
      for x:=0 to 639 do
      begin
         farbe:=GetPixel(x,y)*17;
         write(bfile,chr(farbe));
      end;
   close(bfile);

end; { Screenshot }





{Zeichnet eine dreistellige Zahl an eine gewÅnschte Stelle.}

procedure ZeichneNummer(Zugnummer:integer;x,y:integer);
var
   zifferh,ziffer : integer;

begin
   if vismode then
   begin
      MouseOn;
      BlendFak:=1;
      zifferh:=Zugnummer div 100;
      if zifferh>0 then
	 ZeichneBild(Ziffern2Bild,x,y,zifferh*16,16)
      else
	 ZeichneBild(Ziffern2Bild,x,y,160,16) ;
	 ziffer:=(Zugnummer div 10) mod 10;
      if (ziffer>0) or (zifferh>0) then
	 ZeichneBild(Ziffern2Bild,x+8,y,ziffer*16,16)
      else
	 ZeichneBild(Ziffern2Bild,x+8,y,160,16);
      ziffer:=Zugnummer mod 10;
      ZeichneBild(Ziffern2Bild,x+16,y,ziffer*16,16);
      BlendFak:=BLENDFAKDEF;
      MouseOff;
   end;
end;



{Zeichnet die Anzahl der ZÅge in das Steuerungsfeld.}

procedure ZeichneZugnummer(Zugnummer:integer);
begin
   ZeichneNummer(Zugnummer,603,65);
end;





{Lîscht den Tip-Stein, wenn vorhanden.}

procedure TipWeg(var sp:tSpiel);

begin
   if sp.Feld[setFeld.x,setFeld.z]=Tip then
      sp.Feld[setFeld.x,setFeld.z]:=Leer;
      smilymod:=0;
      ZeichneFeld(sp,setFeld,Steinz);
end;





{Gui ist die Hauptroutine, abhaengig vom Zustand des Spiel zeichnet sie das
 entsprechne und reagiert auf Eingaben und Mausaktionen des Benutzers.}

function Gui(var sp:tSpiel): tSpielState;

var
   LRelease,RRelease: boolean;
   char1,char2      : char;
   SpielState       : tSpielState;
   mauspos,mausposm : tPixel;
   feldpos,pos      : tFeldPos;
   planepos,posrest : tPlanePos;
   raum             : tRaumPos;
   x,z,i,farbe,
   ax,ex,ay,ey      : integer;
   ok               : boolean;

begin

   {Wenn die Szene noch nicht fertig gezeichnet ist, dann wird ein
    weiteres Stueck gezeichnet.}

   if GrS.komplett<64 then
   begin
      ZeichneFlaeche(sp,MINX,MINY,MAXX,MAXY,
                     GrS.komplett,GrS.komplett,Alles);
      inc(GrS.komplett);
   end;

   SpielState:=sp.SpielState;

   {Wenn der Computer gegen sich selbst spielt, wird immer erst gewartet,
    bis das ganze Bild fertig ist.}
   if (GrS.komplett<64) and (sp.Zuordnung[WeissAmZug]=Computer) and (sp.Zuordnung[SchwarzAmZug]=Computer) then
      idle:=true;

   {Wenn der Computer einen Stein gesetzt hat, wird dieser gezeichnet
   und zum anderen Spieler gewechselt.}
   if (sp.neuGesetzt.x > 0) and not idle and ((SpielState=WeissAmZug) or (SpielState=SchwarzAmZug)) then
   begin
      smilymod:=0;
      ZeichneFeld(sp,setFeld,Steinz);
      setFeld.x:=sp.neuGesetzt.x;
      setFeld.z:=sp.neuGesetzt.z;
      sp.Feld[setFeld.x,setFeld.z]:=Stein[sp.Spielstate];

      {Der gesetzte Stein wird mit einem Kreuz gekennzeichnet.}

      smilymod:=2;
      ZeichneFeld(sp,setFeld,Steinz);
      sp.neuGesetzt.x:=0;
      SpielState:=Gegenspieler[SpielState];

{idle wird auf true gesetzt, damit beim Spiel Computer gegen Computer jeder zweite
 Gui-Aufruf frei bleibt fÅr Benutzer-Aktionen wie Drehen des Spielfeldes, Quitten etc.}

      idle:=true;
      {Die Zugnummer wird hochgezÑhlt und gezeichnet.}
      inc(Zugnummer);
      ZeichneZugnummer(Zugnummer);
   end
   else
   begin
      idle:=false;
      char1:=#0;
      char2:=#0;
      mauspos.x := MouseX;
      mauspos.y := MouseY;
      LRelease:=ButtonLeft;
      RRelease:=ButtonRight;

      if KeyPressed then
      begin
         char1:=ReadKey;
         if char1=#0 then
            char2:=ReadKey;
      end;

      {Screenshot abspeichern}

      if char1='d' then
         Screenshot;

      case sp.SpielState of
        Intro :
             if (char1<>#0) or (char2<>#0) or LRelease or RRelease then
             begin
                SpielState:=StartVor;
                hwi:=180;vwi:=90;
                WSpieler:=sp.Zuordnung[WeissAmZug];
                SSpieler:=sp.Zuordnung[SchwarzAmZug];
                gibFeldgroesse(sp.FeldSizeX,sp.FeldSizeY);
                MausFeldSizeX:=sp.FeldSizeX;
                MausFeldSizeY:=sp.FeldSizeY;
                vismode:=true;
             end;

        StartVor :
                begin
                   MouseOff;
                   vvismode:=vismode;
                   if vismode then
                   begin
                      vismode:=false;
                      ZeichneFlaeche(sp,240,124,320,240,0,GrS.komplett-1,Alles);
                   end;
                   HoleHintergrund;
                   LadeVordergrund('start.rle');
                   LadeBild(CompSpielerBild,'compspie.rle');
                   LadeBild(ZiffernBild,'ziffern.rle');
                   LadeBild(DotBild,'dot.rle');
                   sp.Zuordnung[WeissAmZug]:=WSpieler;
                   sp.Zuordnung[SchwarzAmZug]:=SSpieler;
                   ZeichneZuordnung(sp,WeissAmZug);
                   ZeichneZuordnung(sp,SchwarzAmZug);
                   LadeBild(CpStBild,'cpst.rle');
                   gibSpielstufe(sp.Spielstufe);
                   ZeichneBild(CpStBild,386,302,sp.Spielstufe*30,30);
                   ZeichneFGroesse(sp);
                   SpielState:=Start;
                end;

        Start :
             begin
                MouseOn;
                if LRelease then
                begin
                   if innerhalb(mauspos,440,340,490,370) then
                      char1:='q';
                   if innerhalb(mauspos,150,340,210,370) then
                      char1:='s';
                   if innerhalb(mauspos,310,185,420,210) then
                      char1:='w';
                   if innerhalb(mauspos,310,215,420,240) then
                      char1:='b';
                   for i:=0 to 2  do
                   if innerhalb(mauspos,386+i*16,302,402+i*16,330) then
                   char1:=chr(i+48);
                end;
                if innerhalb(mauspos,390,247,510,267) then
                   MausFeldSizeX:=(mauspos.x-400) div 3;
                if innerhalb(mauspos,390,274,510,294) then
                   MausFeldSizeY:=(mauspos.x-400) div 3;

                MausFeldSizeX:=max(min(MausFeldSizeX,MAXFELDSIZE),MINFELDSIZE);
                if (MausFeldSizeX>sp.FeldSizeX) then char1:='+';
                if (MausFeldSizeX<sp.FeldSizeX) then char1:='-';

                MausFeldSizeY:=max(min(MausFeldSizeY,MAXFELDSIZE),MINFELDSIZE);
                if (MausFeldSizeY>sp.FeldSizeY) then char1:='*';
                if (MausFeldSizeY<sp.FeldSizeY) then char1:='_';

                case char1 of

                  's' :
                     begin
                        WSpieler:=sp.Zuordnung[WeissAmZug];
                        SSpieler:=sp.Zuordnung[SchwarzAmZug];
                        SpielState := WeissAmZug;
                        Zugnummer := 0;
                        setzeFeldGroesse(sp.FeldSizeX,sp.FeldSizeY,ok);
                        neuesSpiel(ok);
                        vismode:=vvismode;
                        dist := maxr(sp.FeldSizeX*4.5*3/4,sp.FeldSizeY*4.5);

                        {Wenn vismode=true, dann wird das Steuerungsfeld gezeichnet.}

                        if vismode then
                        begin
                        LadeBild(SteuerBild,'steuer.rle');
                        LadeBild(CpStTinyBild,'cpsttiny.rle');
                        LadeBild(Ziffern2Bild,'ziffern2.rle');
                        BlendFak:=1;
                        ZeichneBild(SteuerBild,560,0,0,116);
                        ZeichneBild(CpStTinyBild,600,84,sp.Spielstufe*16,16);
                        ZeichneZugnummer(Zugnummer);
                        BlendFak:=BlendFakDef;
                        end;

                        ZeichneAlles(sp);
                        for x:=1 to MAXFELDSIZE do
                           for z:=1 to MAXFELDSIZE do
                           begin
                              sp.Feld[x,z]:=Leer;
                              FeldMark[x,z]:=FARBEFELD;
                           end;
                     end;

                  'q' : begin
                     SpielState :=Quit;
                  end;

                  '+' : if sp.FeldSizeX<MAXFELDSIZE then
                  begin
                     if sp.FeldSizeX=MausFeldSizeX then inc(MausFeldSizeX);
                     inc(sp.FeldSizeX); {Spielfeld wird vergroessert}
                     ZeichneFGroesse(sp);
                  end;

                  '-' : if sp.FeldSizeX>MINFELDSIZE then
                  begin
                     if sp.FeldSizeX=MausFeldSizeX then dec(MausFeldSizeX);
                     dec(sp.FeldSizeX); {Spielfeld wird verkleinert}
                     ZeichneFGroesse(sp);
                  end;

                  '*' : if sp.FeldSizeY<MAXFELDSIZE then
                  begin
                     if sp.FeldSizeY=MausFeldSizeY then inc(MausFeldSizeY);
                     inc(sp.FeldSizeY); {Spielfeld wird vergroessert}
                     ZeichneFGroesse(sp);
                  end;

                  '_' : if sp.FeldSizeY>MINFELDSIZE then
                        begin
                          if sp.FeldSizeY=MausFeldSizeY then dec(MausFeldSizeY);
                            dec(sp.FeldSizeY); {Spielfeld wird verkleinert}
                          ZeichneFGroesse(sp);
                        end;


                  'w':  AenderZuordnung(sp,WeissAmZug) ;

                  'b':  AenderZuordnung(sp,SchwarzAmZug) ;

                  '0','1','2': begin
                                 MouseOff;
                                 sp.Spielstufe:=ord(char1)-48;
                                 setzeSpielstufe(sp.Spielstufe,ok);
                                 ZeichneBild(CpStBild,386,302,sp.Spielstufe*30,30);
                                 MouseOn;
                               end;

                end; { case }
             end;

        WeissAmZug,SchwarzAmZug : begin
           mausposm.x:=mauspos.x-320;
           mausposm.y:=240-mauspos.y;
           if RRelease then
              char1:='z';

           {Wenn vismode=true, dann werden Mausklicks auf die Steuerungselemente in Tastendr¸cke umgesetzt.}

           if vismode and (mauspos.x>=560) and (mauspos.y<116) then
           begin
           MouseOn;

           if LRelease then
                begin
                   if innerhalb(mauspos,560,0,640,16) then
                      char1:='c';
                   if innerhalb(mauspos,560,16,640,32) then
                      char1:='C';
                   if innerhalb(mauspos,560,32,640,48) then
                      char1:='t';
                   if innerhalb(mauspos,560,48,607,64) then
                      char1:='z';
                   if innerhalb(mauspos,607,48,640,64) then
                      char1:='v';
                   if innerhalb(mauspos,607,96,640,116) then
                      char1:='q';

                   for i:=0 to 2  do
                   if innerhalb(mauspos,600+i*10,72,616+i*10,96) then
                   char1:=chr(i+48);

                end;
           end
           else
           if pixeltoplane(sp,mausposm.x,mausposm.y,feldpos,
                        planepos,posrest,raum,1)=Feld then
           begin
              MouseOff;
              if LRelease then
              begin
                 if ((sp.Feld[FeldPos.x,FeldPos.z]=Leer) or
                    (sp.Feld[FeldPos.x,FeldPos.z]=Tip)) and
                    ((sp.Zuordnung[WeissAmZug]=Mensch) or
                     (sp.Zuordnung[SchwarzAmZUg]=Mensch)) then
                 begin
                    if sp.Feld[setFeld.x,setFeld.z]=Tip then
                       sp.Feld[setFeld.x,setFeld.z]:=Leer;
                    smilymod:=0;
                    ZeichneFeld(sp,setFeld,Steinz);
                    sp.Feld[FeldPos.x,FeldPos.z]:=stein[sp.SpielState];
                    setFeld:=FeldPos;
                    smilymod:=2;
                    ZeichneFeld(sp,setFeld,Steinz);
                    setzeStein(FeldPos.x,FeldPos.z,ok);
                    SpielState:=Gegenspieler[sp.SpielState];
                    inc(Zugnummer);
                    ZeichneZugnummer(Zugnummer);
                 end;
              end;
              if (aktFeld.x<>feldpos.x) or (aktFeld.z<>feldpos.z) then
                 BewegeFeld(sp,aktFeld,Feldpos);
           end
           else
              MouseOn;
           case char1 of
             #0 : begin

             {Mit den Cursortasten kann man sich um das Spielfeld
              herumbewegen, der horizontale und der vertikale Winkel
              des Betrachters zum Spielfeld wird geaendert.}

             case char2 of

               #72 : if vwi>40 then
               begin
                  vwi:=vwi-5;ZeichneAlles(sp)
               end;

               #80 : if vwi<90 then
               begin
                  vwi:=vwi+5;ZeichneAlles(sp)
               end;

               #77 :
             begin
                hwi:=hwi+10;
                ZeichneAlles(sp)
             end;
               #75 :
             begin
                hwi:=hwi-10;
                ZeichneAlles(sp)
             end;
             end; { case }
          end;

              ' ':
                      begin
                         dist := dist*0.9;
                         ZeichneAlles(sp)
                      end;
              'b':
                      begin
                         dist := dist*1.111;
                         ZeichneAlles(sp)
                      end;
              'q':
                    begin
                      prevKomplett:=GrS.komplett;
                      prevState:=SpielState;
                      SpielState :=QuitFrage;
                      MouseOff;
                      HoleHintergrund;
		      LadeVordergrund('quitfrag.rle');
                      MouseOn;
                    end;
           'x': begin

           {Vismode wird gewechselt, und es wird entweder das Steuerungsfeld gezeichnet
            oder es wird wieder Åberzeichnet.}

  	      vismode:= not vismode;
	      if not vismode then
	      begin
		 ZeichneFlaeche(sp,240,124,320,240,0,63,Alles);
	      end
	      else
	      begin
		 LadeBild(CpStTinyBild,'cpsttiny.rle');
		 BlendFak:=1;
		 ZeichneBild(SteuerBild,560,0,0,116);
		 ZeichneBild(CpStTinyBild,600,84,sp.Spielstufe*16,16);
		 BlendFak:=BlendFakDef;
	      end;
	   end;

           'c': begin
           {Der Computer bekommt den Zug und fÅhrt ihn aus.}
                  TipWeg(sp);
  	          sp.Zuordnung[SpielState]:=Computer;
                  sp.Zuordnung[Gegenspieler[SpielState]]:=Mensch;
                  computerZug(sp.neuGesetzt.x,sp.neuGesetzt.z,ok);
                end;

            'C': begin
            {Der Computer Åbernimmt beide Spieler und spielt so das Spiel zu Ende.}
                   TipWeg(sp);
                   sp.Zuordnung[SpielState]:=Computer;
                   sp.Zuordnung[Gegenspieler[SpielState]]:=Computer;
                   computerZug(sp.neuGesetzt.x,sp.neuGesetzt.z,ok);
                 end;

           '0','1','2': begin
           {Die Spielstufe wird geÑndert.}
                          MouseOff;
                          sp.Spielstufe:=ord(char1)-48;
                          setzeSpielstufe(sp.Spielstufe,ok);
                          if vismode then
                          begin
                            BlendFak:=1;
                            ZeichneBild(CpStTinyBild,600,84,sp.Spielstufe*16,16);
                            BlendFak:=BlendFakDef;
                          end;
                          MouseOn;
                       end;

           'z': begin
           {Der letzte Zug wird zurÅckgenommen.}
                  MouseOff;
                  TipWeg(sp);
                  zugZurueck(setFeld.x,setFeld.z,ok);
                  if ok then
                  begin
                    sp.Feld[setFeld.x,setFeld.z]:=Leer;
                    ZeichneFeld(sp,setFeld,Steinz);
                    sp.Zuordnung[SpielState]:=Mensch;
                    if sp.neugesetzt.x=0 then
                    begin
                       SpielState:=Gegenspieler[sp.SpielState];
                    dec(Zugnummer);
                    ZeichneZugnummer(Zugnummer);
                    end;
                    sp.neugesetzt.x:=0;
                    sp.Zuordnung[WeissAmZug]:=Mensch;
                    sp.Zuordnung[SchwarzAmZug]:=Mensch;
                  end;
                end;

            'v': begin
            {Es wird wieder ein zurÅckgenommener Zug gesetzt.}

                   zugVor(x,z,ok);
                   if ok then
                   begin
                     inc(Zugnummer);
                     ZeichneZugnummer(Zugnummer);
                     smilymod:=0;
                     ZeichneFeld(sp,setFeld,Steinz);
                     setFeld.x:=x;
                     setFeld.z:=z;
                     sp.Feld[x,z]:=stein[sp.SpielState];
                     smilymod:=2;
                     ZeichneFeld(sp,setFeld,Steinz);
                     SpielState:=Gegenspieler[sp.SpielState];
                   end;
                 end;

             't': begin

             {Der Computer gibt einen Tip entsprechend seiner momentanen Spielstufe.}
                    TipWeg(sp);
                    computerzug(x,z,ok);
                    setFeld.x:=x;
                    setFeld.z:=z;
                    sp.Feld[x,z]:=Tip;
                    zugZurueck(x,z,ok);
                    zugZurueck(x,z,ok);
                    if ok then
                      SetzeStein(x,z,ok)
                    else
                      NeuesSpiel(ok);
                    smilymod:=3;
                    ZeichneFeld(sp,setFeld,Steinz);
                  end;

           end; { case }
        end;



        QuitFrage :
                 begin
                    MouseOn;
                    if RRelease then char1:='z';
                    if LRelease then
                       begin
                       if innerhalb(mauspos,440,340,490,370) then
                          char1:='q';
                       if innerhalb(mauspos,150,340,210,370) then
                          char1:='z';
                       if innerhalb(mauspos,250,340,390,370) then
                          char1:='n';
                       end;

                    case char1 of
                      'q' :
                    begin
                       SpielState := Quit
                    end;
                      'n' :
		    begin
                       MouseOff;
		       ZeichneHintergrund;
                       SpielState := StartVor
                    end;
                      'z' :
                    begin
                       MouseOff;
                       ZeichneHintergrund;
                       SpielState := prevState;
                       GrS.komplett := prevKomplett;
                    end;
                    end; { case }

                 end;

        Ende  :
           begin
              MouseOff;
              if sp.status=UNENTSCHIEDEN then
              begin
                 HoleHintergrund;
                 LadeVordergrund('remi.rle');
              end
              else
	      begin

                if sp.status = EINS_GEWINNT then
                  farbe:=120
                else
                  farbe:=255;
                {Markiert die Gewinnreihe mit Smileys.}
                gibGewinnreihe(ax,ay,ex,ey,ok);
                ok:=false;
                smilymod:=1;
                while not ok do
                begin
                  FeldMark[ax,ay]:=farbe;
                  setFeld.x:=ax;
                  setFeld.z:=ay;
                  ZeichneFeld(sp,setFeld,Steinz);
                  ok:=(ax=ex) and (ay=ey);
                  ax:=ax+sgn(ex-ax);
                  ay:=ay+sgn(ey-ay);
                end;

                 HoleHintergrund;
		 LadeBild(WeissSchwarzBild,'weischw.rle');
                 blendfak:=0.334;
                 LadeVordergrund('sieg.rle');

                 if sp.status=EINS_GEWINNT then
                 begin
                    ZeichneBild(WeissSchwarzBild,205,223,0,28)
                 end
                 else
                 begin
                    ZeichneBild(WeissSchwarzBild,205,223,31,27)
                 end
               end;
               blendfak:=BLENDFAKDEF;
              SpielState:=Ende2;
           end;

        Ende2 :
            begin
               MouseOn;
               if RRelease then char1:='z';
               if LRelease then
               begin
                  if innerhalb(mauspos,560,48,607,64) and vismode then
                     char1:='z';
                  if innerhalb(mauspos,440,340,540,370) then
                     char1:='q';
                  if innerhalb(mauspos,150,340,300,370) then
                     char1:='n';
               end;
               case char1 of
                 'n' :
               begin
                  MouseOff;
                  ZeichneHintergrund;
                  SpielState:=StartVor;
               end;
                 'q' :
               begin
                  SpielState:=Quit;
               end;
                'z': begin
                MouseOff;
                ZeichneHintergrund;
                {Lîscht die Gewinnreihensmileys}
                if sp.status<>UNENTSCHIEDEN then
                begin
                  gibGewinnreihe(ax,ay,ex,ey,ok);
                  ok:=false;
                  smilymod:=0;
                  while not ok do
                  begin
                    FeldMark[ax,ay]:=FARBEFELD;
                    setFeld.x:=ax;
                    setFeld.z:=ay;
                    ZeichneFeld(sp,setFeld,Steinz);
                    ok:=(ax=ex) and (ay=ey);
                    ax:=ax+sgn(ex-ax);
                    ay:=ay+sgn(ey-ay);
                  end;
                end;
                zugZurueck(x,z,ok);
                dec(Zugnummer);
                setFeld.x:=x;
                setFeld.z:=z;
                sp.Feld[x,z]:=Leer;
                ZeichneFeld(sp,setFeld,Steinz);
                sp.Zuordnung[SpielState]:=Mensch;

                sp.Zuordnung[SpielState]:=Mensch;

                gibStatus(sp.status);
                case sp.status of
                EINS_AM_ZUG: SpielState:=WeissAmZug;
                ZWEI_AM_ZUG: SpielState:=SchwarzAmZug;

               end;
               sp.Zuordnung[SpielState]:=Mensch;
                end; {if}
                end;

              end;
      end; { case }
   end;
   Gui:=SpielState;
end; { Gui }

end.
