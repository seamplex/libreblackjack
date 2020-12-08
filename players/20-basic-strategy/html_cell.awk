#function abs(v) {return v < 0 ? -v : v}
function ceil(x, y){y=int(x); return(x>y?y+1:y)}
{
 ev=1e-2*$1
 error=1e-2*$2

 if (ev < -1)
   x=-1
 else if (ev > 1)
   x=1
 else
   x=ev
   
#  r=0.5-0.5*x
#  g=0.5+0.5*x
#  b=1-abs(x)

 pi = atan2(0, -1)
 r=cos((x+1)*pi/4)
 g=cos((x-1)*pi/4)
 b=0.4+0.2*cos(x*pi/2)

 
 if (error < 1e-6) {
   error=1e-4;
 }
 
 precision = (ceil(-log(error)/log(10)))-2;
 
 printf("<div class=\"text-center %s\" style='background-color: rgb(%d,%d,%d)'>", (ev<0)?"text-white":"", 255*r, 255*g, 255*b);
 printf(sprintf("%%+.%df", precision), 100*ev);
 printf("(%.0g)", 10^(precision+2) * error);
 printf("</div>");
 
}
