#include <stdio.h>
#include <string.h>

#define LINEBUFLENGTH 180

#define STRIP_LF(str) (str[strlen(str)-1]=0)
void invert_str(char * in)
{
char t;
while(t=*in)
	{
	*in++=~t;
	}
}

int main(int i, char *c[])
{
char dest_fname[80], in_linebuffer[LINEBUFLENGTH];
FILE * fileout, * filein;

if(i>1)
	{
	sprintf(dest_fname, "%s.c", c[1]);

	fileout=fopen(dest_fname, "w");
	filein =fopen(c[1], "r");

	if(fileout && filein)
		{
		fgets(in_linebuffer, LINEBUFLENGTH, filein);
		STRIP_LF(in_linebuffer);
		invert_str(in_linebuffer);
		fputs("char *s_leading=\"", fileout);
		fputs(in_linebuffer, fileout);
		fputs("\";\n", fileout);

		fputs("char *s_messages[]={\n", fileout);

		while(fgets(in_linebuffer, LINEBUFLENGTH, filein))
			{
			STRIP_LF(in_linebuffer);
			invert_str(in_linebuffer);

			fputs("\"", fileout);
			fputs(in_linebuffer, fileout);
			fputs("\",\n", fileout);
			}
		fputs("};\n", fileout);
		fputs("unsigned s_mess_num = sizeof(s_messages)/sizeof(char *);\n", fileout);
		fclose(filein);
		fclose(fileout);
		}
	}
return 0;
}

/*
What's this stuff for ? I use it in SP_Engine to hide the usual bunch of "secret
messages". As you can see, the strings are simply not'ed .
This source shows both flaws: fgets() and the missing buffer flush. You can
change the while() statement with

while(!feof(filein))
	{
	fgets(in_linebuffer, LINEBUFLENGTH, filein);
....

This way you'll workaround the first problem.
The second issue manifests itself this way: the last two fputs() followed by the
fclose() don't do anything: no "};\n" and no "unsigned....." lines are output to
'fileout'. With SAS, it works perfectly. Converting the source to dos.library
calls also works perfectly. I wonder if there's some kind of strange interaction
with the dos/shell updates. 
*/
