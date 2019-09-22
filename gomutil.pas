{Sabine Wolf,5999219,q5999219@bonsai.fernuni-hagen.de}

{In dieser Unit stecken nuetzliche kleine Funktionen, die an vielen
 Stellen gebraucht werden.}

unit gomutil;

interface

{gibt das Vorzeichen einer Zahl als -1,0 oder 1 zurueck}

function sgn(x:integer):integer;

{Gibt ein integer als string zurueck.}

function intstr(i: integer ):   string;


{Gibt den kleineren wert zurueck.}

function min(a,b : integer):integer;


{Gibt den kleineren Real-Wert zurueck.}

function minr(a,b : real):real;


{Gibt den groesseren wert zurueck.}

function max(a,b : integer):integer;
function maxr(a,b : real):real;

implementation

function sgn(x:integer):integer;
begin
if x<0 then
sgn:=-1
else if x>0 then
sgn:=1
else
sgn:=0;
end;

function intstr(i: integer ):   string;
var s : string;
begin
   str(i,s);
   intstr:=s;
end; { intstr }

function min(a,b : integer):integer;
begin
   if a>b then min:=b else min:=a;
end; { min }





function minr(a,b : real):real;
begin
   if a>b then minr:=b else minr:=a;
end; { minr }





function max(a,b : integer):integer;
begin
   if a<b then max:=b else max:=a;
end; { max }

function maxr(a,b : real):real;
begin
   if a<b then maxr:=b else maxr:=a;
end; { maxr }




begin
end.
