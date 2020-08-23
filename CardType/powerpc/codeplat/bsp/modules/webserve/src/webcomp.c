/*
 * webcomp -- Compile web pages into C source
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * $Id: webcomp.c,v 1.3 2002/10/24 14:44:50 bporter Exp $
 */

/******************************** Description *********************************/

/*
 *	Usage: webcomp prefix filelist >webrom.c
 *
 *	filelist is a file containing the pathnames of all web pages
 *	prefix is a path prefix to remove from all the web page pathnames
 *	webrom.c is the resulting C source file to compile and link.
 */

/********************************* Includes ***********************************/

#include	"../inc/wsIntrn.h"

/**************************** Forward Declarations ****************************/

static int 	compile(char_t *fileList, char_t *prefix);
static void usage();
/********************************************************************************
* 函数名称: test_main							
* 功    能:                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/

/*********************************** Code *************************************/
/*
 *	Main program for webpack test harness
 */

int test_main(int argc, char_t* argv[])
{
	char_t		*fileList, *prefix;

	fileList = NULL;

	if (argc != 3) {
		usage();
	}

	prefix = argv[1];
	fileList = argv[2];

	if (compile(fileList, prefix) < 0) {
		return -1;
	}
	return 0;
}
/********************************************************************************
* 函数名称: table_find_fq							
* 功    能:                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/

/******************************************************************************/
/*
 *	Output usage message
 */

static void usage()
{
	fprintf(stderr, "usage: webcomp prefix filelist >output.c\n");
	exit(2);
}
/********************************************************************************
* 函数名称: compile							
* 功    能:                                   
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述	
* 说   明: 
*********************************************************************************/

/******************************************************************************/
/*
 *	Compile the web pages
 */

static int compile(char_t *fileList, char_t *prefix)
{
	gstat_t			sbuf;
	FILE			*lp;
	time_t			now;
	char_t			file[FNAMESIZE];
	char_t			*cp, *sl;
	char			buf[512];
	unsigned char	*p;
	int				j, i, len, fd, nFile;

/*
 *	Open list of files
 */
	if ((lp = fopen(fileList, "r")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", fileList);
		return -1;
	}

	time(&now);
	fprintf(stdout, "/*\n * webrom.c -- Compiled Web Pages\n *\n");
	fprintf(stdout, " * Compiled by GoAhead WebCompile: %s */\n\n", 
		gctime(&now));
	fprintf(stdout, "#include \"wsIntrn.h\"\n\n");
	fprintf(stdout, "#ifndef WEBS_PAGE_ROM\n");
	fprintf(stdout, "websRomPageIndexType websRomPageIndex[] = {\n");
	fprintf(stdout, "    { 0, 0, 0 },\n};\n");
	fprintf(stdout, "#else\n");

/*
 *	Open each input file and compile each web page
 */
	nFile = 0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file,'\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
		if (gstat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		} 
		if ((fd = gopen(file, O_RDONLY | O_BINARY)) < 0) {
			fprintf(stderr, "Can't open file %s\n", file);
			return -1;
		}
		fprintf(stdout, "static unsigned char page_%d[] = {\n", nFile);

		while ((len = read(fd, buf, sizeof(buf))) > 0) {
			p = (unsigned char *)buf;
			for (i = 0; i < len; ) {
				fprintf(stdout, "    ");
				for (j = 0; p < &buf[len] && j < 16; j++, p++) {
					fprintf(stdout, "%3d,", *p);
				}
				i += j;
				fprintf(stdout, "\n");
			}
		}
		fprintf(stdout, "    0 };\n\n");

		close(fd);
		nFile++;
	}
	fclose(lp);

/*
 *	Now output the page index
 */
	fprintf(stdout, "websRomPageIndexType websRomPageIndex[] = {\n");

	if ((lp = fopen(fileList, "r")) == NULL) {
		fprintf(stderr, "Can't open file list %s\n", fileList);
		return -1;
	}
	nFile = 0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
/*
 *		Remove the prefix and add a leading "/" when we print the path
 */
		if (strncmp(file, prefix, gstrlen(prefix)) == 0) {
			cp = &file[gstrlen(prefix)];
		} else {
			cp = file;
		}
		while((sl = strchr(file, '\\')) != NULL) {
			*sl = '/';
		}
		if (*cp == '/') {
			cp++;
		}

		if (gstat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			fprintf(stdout, "    { T(\"/%s\"), 0, 0 },\n", cp);
			continue;
		}
		fprintf(stdout, "    { T(\"/%s\"), page_%d, %d },\n", cp, nFile, 
			sbuf.st_size);
		nFile++;
	}
	fclose(lp); 
	
	fprintf(stdout, "    { 0, 0, 0 },\n");
	fprintf(stdout, "};\n");
	fprintf(stdout, "#endif /* WEBS_PAGE_ROM */\n");

	fclose(lp);
	fflush(stdout);
	return 0;
}

/******************************************************************************/

