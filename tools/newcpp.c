#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <string.h>

void usage(void)
{
	printf("\nusage:\nnewcpp <filename> [module name] [class 2] [class 3] ...\n");
}

main(int argc, char** argv)
{
	FILE* newfile;
	FILE* datefile;
	char* modname;
	char* fname;
	int classno;
    char tmpbuf[128];
	char filename[128];
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
	strcat(filename,".cpp");
    fscanf(datefile,"%d/%d/%d",&mo,&day,&yr);
	fclose(datefile);
	system("rm date.tmp");

	newfile = fopen(filename,"w");

	fname = argv[1];
	if(argc > 2)
		modname = argv[2];
	else
		modname = fname;

	fprintf(newfile,"\
//-----------------------------------------------------------------------------|\
\n// Copyright (C) 2020, All rights reserved.\
\n// embeddedKeith\
\n// -*^*-\
\n//-----------------------------------------------------------------------------|\
\n//\
\n//	Module:");

	fprintf(newfile,"\t%s",(argc > 2)?argv[2]:argv[1]);
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
	fprintf(newfile,"#include <sys/types.h>\n");
	fprintf(newfile,"#include <sys/socket.h>\n");
	fprintf(newfile,"#include <unistd.h>\n");
	fprintf(newfile,"#include <stdlib.h>\n");
	fprintf(newfile,"#include <stdio.h>\n");
	fprintf(newfile,"#include <errno.h>\n");
	fprintf(newfile,"#include <strings.h>\n");
	fprintf(newfile,"#include \"../include/%s.h\"\n",argv[1]);

	fprintf(newfile,"\n\
\n//-----------------------------------------------------------------------------|\
\n// %s member functions\
\n//\n",argv[1]);

	fprintf(newfile,"\n%s::%s()\n{\n\n};\n",argv[1],argv[1]);
	fprintf(newfile,"\n%s::~%s()\n{\n\n};\n",argv[1],argv[1]);
	
	for(classno = 3; classno < argc; classno++)
	{
		fprintf(newfile,"\n\n\
\n//-----------------------------------------------------------------------------|\
\n// %s member functions\
\n//\n",argv[classno]);

		fprintf(newfile,"%s::%s()\n{\n\n};\n",argv[classno],argv[classno]);
		fprintf(newfile,"\n%s::~%s()\n{\n\n};\n",argv[classno],argv[classno]);
	}
	
	fclose(newfile);

	return(0);

}



