/*
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * $Id: main.c,v 1.5 2003/09/11 14:03:46 bporter Exp $
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer. This is a demonstration
 *	main program to initialize and configure the web server.
 */

/********************************* Includes ***********************************/

#include	"../inc/uemf.h"
#include	"../inc/wsIntrn.h"
#include	<signal.h>
#include	<unistd.h> 
#include	<sys/types.h>
#include	<sys/wait.h>

#ifdef WEBS_SSL_SUPPORT
#include	"../inc/websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
#include	"../inc/um.h"
void	formDefineUserMgmt(void);
#endif

void RegisterWebPageDealFun();
/*********************************** Locals ***********************************/
/*
 *	Change configuration here
 */

static char_t		*rootWeb = T("");			/* Root web directory */
static char_t		*password = T("");				/* Security password */
static int			port = 80;						/* Server port */
static int			retries = 5;					/* Server port retries */
static int			finished;						/* Finished flag */



/****************************** Forward Declarations **************************/

static int 	initWebs();
static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
static void formTest(webs_t wp, char_t *path, char_t *query);
static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
				int arg, char_t *url, char_t *path, char_t *query);
extern void defaultErrorHandler(int etype, char_t *msg);
extern void defaultTraceHandler(int level, char_t *buf);
#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif
extern int web_app();
extern int ps2WebsSecurityHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, char_t *url, char_t *path, char_t *query);
/*********************************** Code *************************************/
/*
 *	Main -- entry point from LINUX
 */
int bspstrreplace(char *str,const char *str_src,const char *str_dest)
{
    char *ptr = NULL;
	char buff[1024];
	char buff2[1024];
	int i=0;
	if (str !=NULL)
	{
		strcpy(buff2,str);
	}
	else
	{
	    printf("error!\n");
		return -1;
	}
	memset(buff,0x0,sizeof(buff));
	while((ptr = strstr(buff2,str_src))!=0)
	{
        if (ptr - buff2 !=0) 
			memcpy(&buff[i],buff2,ptr-buff2);
		memcpy(&buff[i+ptr-buff2],str_dest,strlen(str_dest));
		i += ptr - buff2 +strlen(str_dest);
		strcpy(buff2,ptr+strlen(str_src));
	}
	strcat(buff,buff2);
	strcpy(str,buff);
	return 0;
}

int web_app()
{
/*
 *	Initialize the memory allocator. Allow use of malloc and start 
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	bopen(NULL, (60 * 1024), B_USE_MALLOC);
	signal(SIGPIPE, SIG_IGN);

/*
 *	Initialize the web server
 */
	if (initWebs() < 0) 
	{
	    printf("initwebs error!\n");
		  return -1;
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLOpen();
#endif

/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block until an event occurs. SocketProcess
 *	will actually do the servicing.
 */
	while (!finished) {
		if (socketReady(-1) || socketSelect(-1, 1000)) {
			socketProcess(-1);
		}
		websCgiCleanup();
		emfSchedProcess();
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	socketClose();
#ifdef B_STATS
	memLeaks();
#endif
	bclose();
	return 0;
}

/******************************************************************************/
/*
 *	Initialize the web server.
 */

//extern ULONG BspGetCtrlNetIp(UCHAR * pucAddr);
static int initWebs()
{
	struct hostent	*hp;
	struct in_addr	intaddr;
	char			host[128], dir[128], webdir[128];
	char			*cp;
	char_t			wbuf[128];

/*
 *	Initialize the socket subsystem
 */
	socketOpen();

#ifdef USER_MANAGEMENT_SUPPORT

/*
 *	Initialize the User Management database
 */
	umOpen();
	umRestore(T("umconfig.txt"));
#endif

/*
 *	Define the local Ip address, host name, default home page and the 
 *	root web directory.
 */
	if (gethostname(host, sizeof(host)) < 0) 
	{
	  
		error(E_L, E_LOG, T("Can't get hostname"));
		return -1;
	}
	BspGetCtrlNetIp(host);
	printf("web server's ip addr ->%s\n",host);
	if ((hp = gethostbyname(host)) == NULL) 
	{
	 
		error(E_L, E_LOG, T("Can't get host address"));
		return -1;
	}
	memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
		(size_t) hp->h_length);



/*
 *	Set ../web as the root web. Modify this to suit your needs
 */
	getcwd(dir, sizeof(dir)); 
	if ((cp = strrchr(dir, '/'))) {
		*cp = '\0';
	}
	sprintf(webdir, "%s/%s", dir, rootWeb);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultDir(webdir);
	cp = inet_ntoa(intaddr);
	ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	websSetIpaddr(wbuf);
	ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	websSetHost(wbuf);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultPage(T("index.asp"));
	websSetPassword(password);

/* 
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	websOpenServer(port, retries);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security 
 *	handler, forms handler and the default web page handler.
 */
 #if 1
	websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler, 
		WEBS_HANDLER_FIRST);
#else	 	
	 	websUrlHandlerDefine(T(""), NULL, 0 , ps2WebsSecurityHandler,
 		WEBS_HANDLER_FIRST);	
#endif
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler, 
		WEBS_HANDLER_LAST); 

/*
 *	Now define two test procedures. Replace these with your application
 *	relevant ASP script procedures and form functions.
 */
	websAspDefine(T("aspTest"), aspTest);
	websFormDefine(T("formTest"), formTest);

/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0); 
	RegisterWebPageDealFun();
	return 0;
}

/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to 
 *	"localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith")); 
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave.")); 

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}

/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t *url, char_t *path, char_t *query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		websRedirect(wp, T("index.asp"));
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Default error handler.  The developer should insert code to handle
 *	error messages in the desired manner.
 */

void defaultErrorHandler(int etype, char_t *msg)
{
#if 0
	write(1, msg, gstrlen(msg));
#endif
}

/******************************************************************************/
/*
 *	Trace log. Customize this function to log trace output
 */

void defaultTraceHandler(int level, char_t *buf)
{
/*
 *	The following code would write all trace regardless of level
 *	to stdout.
 */
#if 0
	if (buf) {
		write(1, buf, gstrlen(buf));
	}
#endif
}

/******************************************************************************/
/*
 *	Returns a pointer to an allocated qualified unique temporary file name.
 *	This filename must eventually be deleted with bfree();
 */

char_t *websGetCgiCommName()
{
	char_t	*pname1, *pname2;

	pname1 = (char_t *)mkstemp(T("cgi"));
	pname2 = bstrdup(B_L, pname1);
	free(pname1);
	return pname2;
}

/******************************************************************************/
/*
 *	Launch the CGI process and return a handle to it.
 */

int websLaunchCgiProc(char_t *cgiPath, char_t **argp, char_t **envp,
					  char_t *stdIn, char_t *stdOut)
{
	int	pid, fdin, fdout, hstdin, hstdout, rc;

	fdin = fdout = hstdin = hstdout = rc = -1; 
	if ((fdin = open(stdIn, O_RDWR | O_CREAT, 0666)) < 0 ||
		(fdout = open(stdOut, O_RDWR | O_CREAT, 0666)) < 0 ||
		(hstdin = dup(0)) == -1 ||
		(hstdout = dup(1)) == -1 ||
		dup2(fdin, 0) == -1 ||
		dup2(fdout, 1) == -1) {
		goto DONE;
	}
		
 	rc = pid = fork();
 	if (pid == 0) {
/*
 *		if pid == 0, then we are in the child process
 */
		if (execve(cgiPath, argp, envp) == -1) {
			printf("content-type: text/html\n\n"
				"Execution of cgi process failed\n");
		}
		exit (0);
	} 

DONE:
	if (hstdout >= 0) {
		dup2(hstdout, 1);
      close(hstdout);
	}
	if (hstdin >= 0) {
		dup2(hstdin, 0);
      close(hstdin);
	}
	if (fdout >= 0) {
		close(fdout);
	}
	if (fdin >= 0) {
		close(fdin);
	}
	return rc;
}

/******************************************************************************/
/*
 *	Check the CGI process.  Return 0 if it does not exist; non 0 if it does.
 */

int websCheckCgiProc(int handle)
{
/*
 *	Check to see if the CGI child process has terminated or not yet.  
 */
	if (waitpid(handle, NULL, WNOHANG) == handle) {
		return 0;
	} else {
		return 1;
	}
}

/******************************************************************************/

#ifdef B_STATS
static void memLeaks() 
{
	int		fd;

	if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY, 0666)) >= 0) {
		bstats(fd, printMemStats);
		close(fd);
	}
}

/******************************************************************************/
/*
 *	Print memory usage / leaks
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
	va_list		args;
	char_t		buf[256];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	write(handle, buf, strlen(buf));
}
#endif
 #define		NONE_OPTION		T("<NONE>")
#define		MSG_START		T("<body><h2>")
#define		MSG_END			T("</h2></body>")

/******************************************************************************
* 函数名: websMsgStart
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void	websMsgStart(webs_t wp)
{
	websWrite(wp, MSG_START);
}

/******************************************************************************
* 函数名: websMsgEnd
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void	websMsgEnd(webs_t wp)
{
	websWrite(wp, MSG_END);
}
/******************************************************************************
* 函数名: uservalid
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
int uservalid(int eid, webs_t wp, int argc, char_t **argv)
{
    a_assert(wp);
    //if (0 == strcmp(argv[1],"admin") && 0 == strcmp(argv[3],"admin"))
	  printf("enter success!\n");
    return 1;
}

/******************************************************************************
* 函数名: isAccessPagePub
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
static unsigned char isAccessPagePub(unsigned char *url)
{
	if (NULL == gstrstr((const char *)url, T("function.asp")) && (0 != gstrcmp((const char *)url, T("/goform/loginSet"))))
	{
		return 0;	
	}		
	return 1;
}

#define NO_PARAM				(-10)
#define NO_NEXT					(-20)	

#define USR_NAME_LEN   32+1
#define USR_PWD_LEN    32+1
#define USR_LEVEL_LOW     1
#define USR_LEVEL_NORMAL  2
#define USR_LEVEL_HIGH    3 
#define MAX_USR_NUM       10

#define  NOALARM 		0
#define  MINORALARM 	1
#define  MAJORALARM 	2

#define KNUMACTUALUSERS         10
#define	kMaxSnmpInstance 				128
#define MAX_WRITE_BUF_SIZE 5*1024
#define WAITBUFF 50


#define MIB_SCALAR_VALUE_TYPE 1

 typedef struct usrLoginInfo{
	char		name[USR_NAME_LEN]; /**用户名*/
	char		pwd[USR_PWD_LEN];   /**用户密码*/
	int			level;              /**用户级别*/
	int 		disable;            /**是否可用*/
	int 		language;           /**语言类别*/
}USRLOGININFO;

#define QUERY_NAEM_LEN_MAX 64
#define QUERY_VAR_LEN_MAX 256
 
typedef struct
{
	int valueIndex;
	char NameStr[QUERY_NAEM_LEN_MAX];
	char VarStr[QUERY_VAR_LEN_MAX];
	char *pVarStr;
}T_QueryStruct;

 #define auth_normal 
#define ALARMMAX	256
#define VALUEMAX 160
 
#define UNDEFINE_TBLID 67
#define OK							0
#define ERROR						(-1)
char_t g_webWriteBuf[MAX_WRITE_BUF_SIZE];
/*************************************页面登陆******************************************/
/******************************************************************************
* 函数名: loginSet
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void loginSet(webs_t wp, char_t *path, char_t *query)
{
    char    host[128];
    char    deststr[128]; 
    BspGetCtrlNetIp(host);
    sprintf(deststr, "%s%s%s","http://",host,"/success.asp");
    websRedirect(wp,deststr);
}
/******************************************************************************
* 函数名: UsrSet
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void UsrSet(webs_t wp, char_t *path, char_t *query)
{
    //USRLOGININFO pUserInfo;
    //T_QueryStruct	QueryTab[VALUEMAX] = {0};
    system("reboot");
}

/******************************************************************************
* 函数名: RegisterWebPageDealFun
* 功  能: 
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
void RegisterWebPageDealFun() 
{
    websAspDefine(T("uservalid"), uservalid);
    websFormDefine(T("loginSet"), loginSet);
    websFormDefine(T("UsrSet"), UsrSet);
}

/******************************************************************************/

