/*
 * $Id: uname.c,v 1.2 2005-03-03 15:12:18 obarthel Exp $
 *
 * :ts=4
 */

/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

int main(int cnt,char *arg[])
{
struct utsname uinfo;
int Ret=EXIT_SUCCESS;

if(cnt>1 && (!strcmp(arg[1],"-h") || !strcmp(arg[1],"--help"))) {
	printf(
		"  -a\tPrint all information.\n"
		"  -m\tPrint the machine type.\n"
		"  -n\tPrint the node (host) name.\n"
		"  -r\tPrint the OS release.\n"
		"  -s\tPrint the OS name.\n"
		"  -v\tPrint the OS version.\n\n"
		"  -h or --help displays this message.\n\n"
	);
} else {
	if(!uname(&uinfo)) {
		if(cnt<2 || !strcmp(arg[1],"-s")) {
			printf("%s\n",uinfo.sysname);
		} else if(!strcmp(arg[1],"-a")) {
			printf("%s %s %s %s %s\n",uinfo.sysname,uinfo.version,uinfo.release,uinfo.nodename,uinfo.machine);
		} else if(!strcmp(arg[1],"-m")) {
			printf("%s\n",uinfo.machine);
		} else if(!strcmp(arg[1],"-n")) {
			printf("%s\n",uinfo.nodename);
		} else if(!strcmp(arg[1],"-r")) {
			printf("%s\n",uinfo.release);
		} else if(!strcmp(arg[1],"-v")) {
			printf("%s\n",uinfo.version);
		} else {
			printf("Unknown option \"%s\"!\nTry -h or --help.\n",arg[1]);
			Ret=EXIT_FAILURE;
		}
	} else {
		printf("Unknown error!\n");
	}
}

return(Ret);
}

/* vi:set ts=3: */

