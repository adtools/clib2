#include <string.h>
#include <stdlib.h>
#include <stdio.h>

double x;

void
print_format_int(const char * format_string,int parameter1,int parameter2)
{
	printf("\"");
	printf(format_string,parameter1);
	printf("\"\t");

	printf("Value = % d, Format = \"%s\"\n",parameter1,format_string);

	printf("\"");
	printf(format_string,parameter2);
	printf("\"\t");

	printf("Value = % d, Format = \"%s\"\n",parameter2,format_string);
}

void
print_format_char(const char * format_string,char parameter)
{
	printf("\"");
	printf(format_string,parameter);
	printf("\"\t");

	printf("Value = '%c', Format = \"%s\"\n",parameter,format_string);
}

void
print_format_string(const char * format_string,const char *parameter1,const char *parameter2)
{
	printf("\"");
	printf(format_string,parameter1);
	printf("\"\t");

	printf("Value = \"%s\", Format = \"%s\"\n",parameter1,format_string);

	printf("\"");
	printf(format_string,parameter2);
	printf("\"\t");

	printf("Value = \"%s\", Format = \"%s\"\n",parameter2,format_string);
}

void
print_format_float(const char * format_string,double parameter1,double parameter2)
{
	printf("\"");
	printf(format_string,parameter1);
	printf("\"\t");

	printf("Value = % f, Format = \"%s\"\n",parameter1,format_string);

	printf("\"");
	printf(format_string,parameter2);
	printf("\"\t");

	printf("Value = % f, Format = \"%s\"\n",parameter2,format_string);
}

int
main(void)
{
	/*
	unsigned long foo[2] = { 0x41f00000, 0 };

	memcpy(&x,foo,sizeof(x));

	printf("%.20g\n",x);
	*/

	print_format_int("%12d",45,-45);
	print_format_int("%012d",45,-45);
	print_format_int("% 012d",45,-45);
	print_format_int("%+12d",45,-45);
	print_format_int("%+012d",45,-45);
	print_format_int("%-12d",45,-45);
	print_format_int("%- 12d",45,-45);
	print_format_int("%-+12d",45,-45);
	print_format_int("%12.4d",45,-45);
	print_format_int("%-12.4d",45,-45);
	print_format_int("%12.0d",45,-45);

	printf("\n");

	print_format_int("%14u",45,-45);
	print_format_int("%014u",45,-45);
	print_format_int("%#14u",45,-45);
	print_format_int("%#014u",45,-45);
	print_format_int("%-14u",45,-45);
	print_format_int("%-#14u",45,-45);
	print_format_int("%14.4u",45,-45);
	print_format_int("%-14.4u",45,-45);
	print_format_int("%14.0u",45,-45);

	printf("\n");

	print_format_int("%14o",45,-45);
	print_format_int("%014o",45,-45);
	print_format_int("%#14o",45,-45);
	print_format_int("%#014o",45,-45);
	print_format_int("%-14o",45,-45);
	print_format_int("%-#14o",45,-45);
	print_format_int("%14.4o",45,-45);
	print_format_int("%-14.4o",45,-45);
	print_format_int("%14.0o",45,-45);

	printf("\n");

	print_format_int("%12x",45,-45);
	print_format_int("%012x",45,-45);
	print_format_int("%#12X",45,-45);
	print_format_int("%#012X",45,-45);
	print_format_int("%-12x",45,-45);
	print_format_int("%-#12x",45,-45);
	print_format_int("%12.4x",45,-45);
	print_format_int("%-12.4x",45,-45);
	print_format_int("%12.0x",45,-45);

	printf("\n");

	print_format_char("%12c",'*');
	print_format_char("%012c",'*');
	print_format_char("%-12c",'*');
	print_format_char("%12.0c",'*');

	printf("\n");

	print_format_string("%12s","zap","longish");
	print_format_string("%12.5s","zap","longish");
	print_format_string("%012s","zap","longish");
	print_format_string("%-12s","zap","longish");
	print_format_string("%12.0s","zap","longish");

	printf("\n");

	print_format_float("%10.2f",12.678,-12.678);
	print_format_float("%010.2f",12.678,-12.678);
	print_format_float("% 010.2f",12.678,-12.678);
	print_format_float("%+10.2f",12.678,-12.678);
	print_format_float("%+010.2f",12.678,-12.678);
	print_format_float("%-10.2f",12.678,-12.678);
	print_format_float("%- 10.2f",12.678,-12.678);
	print_format_float("%-+10.4f",12.678,-12.678);
	print_format_float("%f",12.678,-12.678);
	print_format_float("%10f",12.678,-12.678);
	print_format_float("%10.0f",12.678,-12.678);

	printf("\n");

	print_format_float("%10.2e",12.678,-12.678);
	print_format_float("%010.2e",12.678,-12.678);
	print_format_float("% 010.2e",12.678,-12.678);
	print_format_float("%+10.2E",12.678,-12.678);
	print_format_float("%+010.2E",12.678,-12.678);
	print_format_float("%-10.2e",12.678,-12.678);
	print_format_float("%- 10.2e",12.678,-12.678);
	print_format_float("%-+10.2e",12.678,-12.678);
	print_format_float("%e",12.678,-12.678);
	print_format_float("%10e",12.678,-12.678);
	print_format_float("%10.0e",12.678,-12.678);

	printf("\n");

	print_format_float("%10.2a",12.678,-12.678);
	print_format_float("%010.2a",12.678,-12.678);
	print_format_float("% 010.2a",12.678,-12.678);
	print_format_float("%+10.2A",12.678,-12.678);
	print_format_float("%+010.2A",12.678,-12.678);
	print_format_float("%-10.2a",12.678,-12.678);
	print_format_float("%- 10.2a",12.678,-12.678);
	print_format_float("%-+10.2a",12.678,-12.678);
	print_format_float("%a",12.678,-12.678);
	print_format_float("%10a",12.678,-12.678);
	print_format_float("%10.0a",12.678,-12.678);

	printf("\n");

	print_format_float("%10.2g",12.678,-12.678);
	print_format_float("%010.2g",12.678,-12.678);
	print_format_float("% 010.2g",12.678,-12.678);
	print_format_float("%+10.2G",12.678,-12.678);
	print_format_float("%+010.2G",12.678,-12.678);
	print_format_float("%-10.2g",12.678,-12.678);
	print_format_float("%- 10.2g",12.678,-12.678);
	print_format_float("%-+10.2g",12.678,-12.678);
	print_format_float("%g",12.678,-12.678);
	print_format_float("%10g",12.678,-12.678);
	print_format_float("%10.0g",12.678,-12.678);

	printf("\n");

	print_format_float("%10.2g",0.678,-0.678);
	print_format_float("%010.2g",0.678,-0.678);
	print_format_float("% 010.2g",0.678,-0.678);
	print_format_float("%+10.2G",0.678,-0.678);
	print_format_float("%+010.2G",0.678,-0.678);
	print_format_float("%-10.2g",0.678,-0.678);
	print_format_float("%- 10.2g",0.678,-0.678);
	print_format_float("%-+10.2g",0.678,-0.678);
	print_format_float("%g",0.678,-0.678);
	print_format_float("%10g",0.678,-0.678);
	print_format_float("%10.0g",0.678,-0.678);

	printf("\n");

	print_format_float("%10.2g",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("%010.2g",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("% 010.2g",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("%+10.2G",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("%+010.2G",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("%-10.2g",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("%- 10.2g",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("%-+10.2g",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("%g",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("%10g",strtod("infinity",NULL),strtod("-infinity",NULL));
	print_format_float("%10.0g",strtod("infinity",NULL),strtod("-infinity",NULL));

	printf("\n");

	print_format_float("%10.2g",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("%010.2g",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("% 010.2g",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("%+10.2G",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("%+010.2G",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("%-10.2g",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("%- 10.2g",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("%-+10.2g",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("%g",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("%10g",strtod("nan",NULL),strtod("-nan",NULL));
	print_format_float("%10.0g",strtod("nan",NULL),strtod("-nan",NULL));

	return(0);
}
