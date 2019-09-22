#include "ptoc.h"

/*Sabine Wolf,5999219,q5999219@bonsai.fernuni-hagen.de*/

/*In dieser Unit stecken nuetzliche kleine Funktionen, die an vielen
 Stellen gebraucht werden.*/

#define __gomutil_implementation__


#include "GOMUTIL.h"


integer sgn(integer x)
{
integer sgn_result;
if (x<0) 
sgn_result=-1;
else if (x>0) 
sgn_result=1;
else
sgn_result=0;
return sgn_result;
}

string intstr(integer i )
{
    string s;

   string intstr_result;
   str(i,s);
   intstr_result=s;
   return intstr_result;
}    /* intstr */

integer min(integer a,integer b)
{
   integer min_result;
   if (a>b)  min_result=b; else min_result=a;
   return min_result;
}    /* min */





real minr(real a,real b)
{
   real minr_result;
   if (a>b)  minr_result=b; else minr_result=a;
   return minr_result;
}    /* minr */





integer max(integer a,integer b)
{
   integer max_result;
   if (a<b)  max_result=b; else max_result=a;
   return max_result;
}    /* max */

real maxr(real a,real b)
{
   real maxr_result;
   if (a<b)  maxr_result=b; else maxr_result=a;
   return maxr_result;
}    /* maxr */




class unit_gomutil_initialize {
  public: unit_gomutil_initialize();
};
static unit_gomutil_initialize gomutil_constructor;

unit_gomutil_initialize::unit_gomutil_initialize() {;
}
