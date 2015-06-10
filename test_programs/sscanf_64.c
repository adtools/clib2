#include <stdio.h>

/*extern int __debug_level;*/

int
main (int argc, char **argv)
{
   long long int l;
   char * arg;

   /*__debug_level = 2;*/

   if(argc > 1)
     arg = argv[1];
   else
     arg = "123456789012";

   sscanf (arg, "%lld", &l);
   printf ("number = %lld = 0x%llX\n", l, l);

   return(0);
}
