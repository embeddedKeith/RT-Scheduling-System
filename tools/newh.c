#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>

void usage(void)
{
	printf("\nusage:\nnewc <filename>\n");
}

main(int argc, char** argv)
{
	FILE* newfile;
	FILE* datefile;
    char tmpbuf[128];
	char filename[32];
	char capsname[32];
	char* fname;
	char* modname;
	int classno;
	int index;
	int argindex;
	int mo,day,yr;

	system("date \"+%m/%d/20%y\" > date.tmp");
	datefile = fopen("date.tmp","r");

	if(argc < 2)
	{
		usage();
		return(1);
	}

	filename[0]=(char)0;
	strcat(filename,argv[1]);
	strcat(filename,".h");
    fscanf(datefile,"%d/%d/%d",&mo,&day,&yr);
	fclose(datefile);
	system("rm date.tmp");

	newfile = fopen(filename,"w");

	index = 0;
	argindex = 0;
	capsname[index++]='_';
	while((argv[1][argindex]!=(char)0)&&(index<32))
	{
		if((argv[1][argindex]>='a')&&(argv[1][argindex]<='z'))
			capsname[index++]=argv[1][argindex++]-(char)('a'-'A');
		else
			capsname[index++]=argv[1][argindex++];
	}
	capsname[index++]='_';
	capsname[index++]='H';
	capsname[index++]='_';
	capsname[index++]='\0';

	fprintf(newfile,"#ifndef %s",capsname);
	fprintf(newfile,"\n#define %s",capsname);

	fname = argv[1];
	if(argc > 2)
		modname = argv[2];
	else
		modname = fname;

	fprintf(newfile,"\
\n//-----------------------------------------------------------------------------|\
\n// Copyright (C) 2020, All rights reserved.\
\n// embeddedKeith\
\n// -*^*-\
\n//-----------------------------------------------------------------------------|\
\n//\
\n//	Module:");

	fprintf(newfile,"\t%s",argv[1]);
	fprintf(newfile,"\
\n//\
\n//	Description:	\
\n//\
\n//	FileName:");
	fprintf(newfile,"\t%s",filename);
	fprintf(newfile,"\
\n//\
\n//	Originator:	Keith DeWald\
\n//\
\n//  Initials  Date           Description\
\n//  --------  ---------      -------------------------------------------------\
\n//  KRD       %02d/%02d/%d",mo,day,yr);
	fprintf(newfile,"     Created file and began implementation");
	fprintf(newfile,"\
\n//\
\n//\
\n//-----------------------------------------------------------------------------|\n");

	fprintf(newfile,"\n\n\
\n//-----------------------------------------------------------------------------|\
\n// %s class definition\
\n//\n",argv[1]);
	fprintf(newfile,"class %s\
\n{\n\tpublic:\n\t\t%s();\n\t\t~%s();\n\tprotected:\n\tprivate:\n};",argv[1],
					argv[1],argv[1]);

	for(classno = 3; classno < argc; classno++)
	{
		fprintf(newfile,"\n\n\
\n//-----------------------------------------------------------------------------|\
\n// %s class definition\
\n//\n",argv[classno]);
		fprintf(newfile,"class %s\
\n{\n\tpublic:\n\t\t%s();\n\t\t~%s();\n\tprotected:\n\tprivate:\n};",
			argv[classno],argv[classno],argv[classno]);
		fprintf(newfile,"\n#endif\t// %s\n",capsname);
	}

	fclose(newfile);
	return(0);
}
