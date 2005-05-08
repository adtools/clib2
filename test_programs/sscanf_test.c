#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int
main(void)
{
	int first, second, third, num;
	int n,a,b,c;
	char str[4];
	double f;

	a = 0;
	n = sscanf("060206","%x",&a);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d\n",n,a);

	num = sscanf("day-month-year", "%d-%d-%d", &first, &second, &third);

	printf("%s:%d:",__FILE__,__LINE__);printf("%d %d %d %d\n", num, first, second, third);

	num = sscanf("8-11-2004", "%d-%d-%d", &first, &second, &third);

	printf("%s:%d:",__FILE__,__LINE__);printf("%d %d %d %d\n", num, first, second, third);

	num = sscanf("", "%d-%d-%d", &first, &second, &third);

	printf("%s:%d:",__FILE__,__LINE__);printf("%d %d %d %d\n", num, first, second, third);

	num = sscanf("6", "%d %d", &first, &second);

	printf("%s:%d:",__FILE__,__LINE__);printf("%d %d\n", num, first);

	a = b = c = 0;
	n = sscanf("","%*d,%d,%d",&a,&b,&c);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d, b = %d, c = %d\n",n,a,b,c);

	a = b = c = 0;
	n = sscanf("1,2,3","%*d,%d,%d",&a,&b,&c);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d, b = %d, c = %d\n",n,a,b,c);

	a = b = c = 0;
	n = sscanf("1,2","%*d,%d,%d",&a,&b,&c);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d, b = %d, c = %d\n",n,a,b,c);

	a = b = c = 0;
	n = sscanf("asdf","*d,d,d",&a,&b,&c);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d, b = %d, c = %d\n",n,a,b,c);

	memset(str,0,sizeof(str));
	n = sscanf("asdf","%[abc]",str);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, str = '%s'\n",n,str);

	memset(str,0,sizeof(str));
	n = sscanf("asdbbfc","%[abc]",str);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, str = '%s'\n",n,str);

	memset(str,0,sizeof(str));
	n = sscanf("","%[abc]",str);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, str = '%s'\n",n,str);

	memset(str,0,sizeof(str));
	n = sscanf("abcdef","%[abc]",str);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, str = '%s'\n",n,str);

	a = b = c = 0;
	n = sscanf("-","%d",&a);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d\n",n,a);

	a = b = c = 0;
	n = sscanf("-4,-","%d,%d",&a,&b);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d, b = %d\n",n,a,b);

	memset(str,0,sizeof(str));
	n = sscanf("1 abc","%d %4c",&a,str);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d, str = '%s'\n",n,a,str);

	memset(str,0,sizeof(str));
	n = sscanf("abc","%4c",&a,str);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, str = '%s'\n",n,str);
	
	a = 0;
	n = sscanf("17","%i",&a);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d\n",n,a);

	a = 0;
	n = sscanf("017","%i",&a);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d\n",n,a);

	a = 0;
	n = sscanf("0x17","%i",&a);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d\n",n,a);

	a = 0;
	n = sscanf("0x017","%i",&a);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d\n",n,a);

	a = 0;
	n = sscanf("0x017","%x",&a);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d\n",n,a);

	a = 0;
	n = sscanf("0017","%x",&a);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %d\n",n,a);

	a = 0;
	n = sscanf("0x80000000","%i",&a);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, a = %u\n",n,a);

	memset(str,0,sizeof(str));
	n = sscanf("1,e","%*d,%[abc]",str);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, str = '%s'\n",n,str);

	n = sscanf("1","%lf",&f);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, f = '%f' (%f)\n",n,f,atof("1"));

	n = sscanf("1234.567","%lf",&f);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, f = '%f' (%f)\n",n,f,atof("1234.567"));

	n = sscanf("inf","%lf",&f);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, f = '%f' (%f)\n",n,f,atof("inf"));

	n = sscanf("infinity","%lf",&f);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, f = '%f' (%f)\n",n,f,atof("infinity"));

	n = sscanf("nan","%lf",&f);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, f = '%f' (%f)\n",n,f,atof("nan"));

	n = sscanf("nan(23)","%lf",&f);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, f = '%f' (%f)\n",n,f,atof("nan(23)"));

	n = sscanf("0xabc.defp+101","%lf",&f);
	printf("%s:%d:",__FILE__,__LINE__);printf("n = %d, f = '%f' (%f)\n",n,f,atof("0xabc.defp+101"));

	return(0);
}
