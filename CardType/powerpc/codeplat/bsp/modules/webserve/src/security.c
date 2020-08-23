/*
 * security.c -- Security handler
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * $Id: security.c,v 1.9 2003/09/19 17:04:44 bporter Exp $
 */

/******************************** Description *********************************/

/*
 *	This module provides a basic security policy.
 */

/********************************* Includes ***********************************/

#include	"../inc/wsIntrn.h"
#include	"../inc/um.h"
#ifdef DIGEST_ACCESS_SUPPORT
#include	"../inc/websda.h"
#endif

/********************************** Defines ***********************************/
/*
 *	The following #defines change the behaviour of security in the absence 
 *	of User Management.
 *	Note that use of User management functions require prior calling of
 *	umInit() to behave correctly
 */

#ifndef USER_MANAGEMENT_SUPPORT
#define umGetAccessMethodForURL(url) AM_FULL
#define umUserExists(userid) 0
#define umUserCanAccessURL(userid, url) 1
#define umGetUserPassword(userid) websGetPassword()
#define umGetAccessLimitSecure(accessLimit) 0
#define umGetAccessLimit(url) NULL
#endif

#define WEB_ACCESS_TYPE_POST 1
#define WEB_ACCESS_TYPE_GET 0

#define LOGON_WEB_NAME "logon.asp"
#define INDEX_WEB_NAME "index.asp"
#define NO_RIGHT_WEB_NAME "nolevel.asp"

/******************************** Local Data **********************************/

static char_t	websPassword[WEBS_MAX_PASS];	/* Access password (decoded) */
#ifdef _DEBUG
static int		debugSecurity = 1;
#else
static int		debugSecurity = 0;
#endif

typedef struct
{
	char pageUrl[32];
	int  pageGetLevel;	
	int  pageSetLevel;
}T_pageAccessLevel;


typedef struct
{
	char		name[33];
	char		pwd[33];
	int			level;
	int 		disable;
}userInfo_t;
#define	kHwMaxSimultaneousClients	30
#define KNUMACTUALUSERS         10
#define	kMaxSnmpInstance 				128
#define MAX_WRITE_BUF_SIZE 5*1024
#define WAITBUFF 50
//static userInfo_t UserDatabase[KNUMACTUALUSERS];	/** �����û���Ϣ */
int UserNumBuff[kHwMaxSimultaneousClients]; 								/*����ÿһ��clientindex��Ӧ���û����*/
/*********************************** Code *************************************/
/*
 *	Determine if this request should be honored
 */
/******************************************************************************
* ������: websSecurityHandler
* ��  ��: 
* �����洢����:
* ����:
* ������        ����        ����/���       ����
* ����ֵ: 0 ��������������
* ˵��:
******************************************************************************/
int websSecurityHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
						char_t *url, char_t *path, char_t *query)
{
	char_t			*type, *userid, *password, *accessLimit;
	int				flags, nRet;
	accessMeth_t	am;

	a_assert(websValid(wp));
	a_assert(url && *url);
	a_assert(path && *path);
/*
 *	Get the critical request details
 */
	type = websGetRequestType(wp);
	password = websGetRequestPassword(wp);
	userid = websGetRequestUserName(wp);
	flags = websGetRequestFlags(wp);
/*
 *	Get the access limit for the URL.  Exit if none found.
 */
	accessLimit = umGetAccessLimit(path);
	if (accessLimit == NULL) {
		return 0;
	}
		 
/*
 *	Check to see if URL must be encrypted
 */
#ifdef WEBS_SSL_SUPPORT
	nRet = umGetAccessLimitSecure(accessLimit);
	if (nRet && ((flags & WEBS_SECURE) == 0)) {
		websStats.access++;
		websError(wp, 405, T("Access Denied\nSecure access is required."));
		trace(3, T("SEC: Non-secure access attempted on <%s>\n"), path);
      /* bugfix 5/24/02 -- we were leaking the memory pointed to by
       * 'accessLimit'. Thanks to Simon Byholm.
       */
      bfree(B_L, accessLimit);
		return 1;
	}
#endif

/*
 *	Get the access limit for the URL
 */
	am = umGetAccessMethodForURL(accessLimit);

	nRet = 0;
	if ((flags & WEBS_LOCAL_REQUEST) && (debugSecurity == 0)) {
/*
 *		Local access is always allowed (defeat when debugging)
 */
	} else if (am == AM_NONE) {
/*
 *		URL is supposed to be hidden!  Make like it wasn't found.
 */
		websStats.access++;
		websError(wp, 404, T("Page Not Found"));
		nRet = 1;
	} else 	if (userid && *userid) {
		if (!umUserExists(userid)) {
			websStats.access++;
			websError(wp, 401, T("Access Denied\nUnknown User"));
			trace(3, T("SEC: Unknown user <%s> attempted to access <%s>\n"), 
				userid, path);
			nRet = 1;
		} else if (!umUserCanAccessURL(userid, accessLimit)) {
			websStats.access++;
			websError(wp, 403, T("Access Denied\nProhibited User"));
			nRet = 1;
		} else if (password && * password) {
			char_t * userpass = umGetUserPassword(userid);
			if (userpass) {
				if (gstrcmp(password, userpass) != 0) {
					websStats.access++;
					websError(wp, 401, T("Access Denied\nWrong Password"));
					trace(3, T("SEC: Password fail for user <%s>")
								T("attempt to access <%s>\n"), userid, path);
					nRet = 1;
				} else {
/*
 *					User and password check out.
 */
				}

				bfree (B_L, userpass);
			}
#ifdef DIGEST_ACCESS_SUPPORT
		} else if (flags & WEBS_AUTH_DIGEST) {

			char_t *digestCalc;

/*
 *			Check digest for equivalence
 */
			wp->password = umGetUserPassword(userid);

			a_assert(wp->digest);
			a_assert(wp->nonce);
			a_assert(wp->password);
							 
			digestCalc = websCalcDigest(wp);
			a_assert(digestCalc);

			if (gstrcmp(wp->digest, digestCalc) != 0) {
				websStats.access++;
            /* 16 Jun 03 -- error code changed from 405 to 401 -- thanks to
             * Jay Chalfant.
             */
				websError(wp, 401, T("Access Denied\nWrong Password"));
				nRet = 1;
			}

			bfree (B_L, digestCalc);
#endif
		} else {
/*
 *			No password has been specified
 */
#ifdef DIGEST_ACCESS_SUPPORT
			if (am == AM_DIGEST) {
				wp->flags |= WEBS_AUTH_DIGEST;
			}
#endif
			websStats.errors++;
			websError(wp, 401, 
				T("Access to this document requires a password"));
			nRet = 1;
		}
	} else if (am != AM_FULL) {
/*
 *		This will cause the browser to display a password / username
 *		dialog
 */
#ifdef DIGEST_ACCESS_SUPPORT
		if (am == AM_DIGEST) {
			wp->flags |= WEBS_AUTH_DIGEST;
		}
#endif
		websStats.errors++;
		websError(wp, 401, T("Access to this document requires a User ID"));
		nRet = 1;
	}

	bfree(B_L, accessLimit);

	return nRet;
}
/******************************************************************************
* ������: websSecurityDelete
* ��  ��: 
* �����洢����:
* ����:
* ������        ����        ����/���       ����
* ����ֵ: 0 ��������������
* ˵��:
******************************************************************************/
/******************************************************************************/
/*
 *	Delete the default security handler
 */

void websSecurityDelete()
{
	websUrlHandlerDelete(websSecurityHandler);
}
/******************************************************************************
* ������: websSetPassword
* ��  ��: 
* �����洢����:
* ����:
* ������        ����        ����/���       ����
* ����ֵ: 0 ��������������
* ˵��:
******************************************************************************/
/******************************************************************************/
/*
 *	Store the new password, expect a decoded password. Store in websPassword in 
 *	the decoded form.
 */

void websSetPassword(char_t *password)
{
	a_assert(password);

	gstrncpy(websPassword, password, TSZ(websPassword));
}
/******************************************************************************
* ������: websGetPassword
* ��  ��: 
* �����洢����:
* ����:
* ������        ����        ����/���       ����
* ����ֵ: 0 ��������������
* ˵��:
******************************************************************************/
/******************************************************************************/
/*
 *	Get password, return the decoded form
 */

char_t *websGetPassword()
{
	return bstrdup(B_L, websPassword);
}

#define WEB_ACCESS_TYPE_POST 1
#define WEB_ACCESS_TYPE_GET 0

#define LOGON_WEB_NAME "logon.asp"
#define INDEX_WEB_NAME "index.asp"
#define NO_RIGHT_WEB_NAME "nolevel.asp"

/*********************ҳ��Ȩ�޶��ձ�****************************/
/*****************Page Url---��Ȩ��---дȨ��********************/
/***************************************************************/
T_pageAccessLevel spPageAccessLevelMap[] = 
{
	{"/user.asp", 3, 3},
	{"/password", 1, 1},
	{"/reboot.asp", 3, 3},	
};

/******************************************************************************
* ������: getWebAccessType
* ��  ��: 
* �����洢����:
* ����:
* ������        ����        ����/���       ����
* ����ֵ: 0 ��������������
* ˵��:
******************************************************************************/
#if 0
static int getWebAccessType(char_t *url)
{
		if (NULL != strstr(url, "/goform"))
		{
			return WEB_ACCESS_TYPE_POST;	
		}
		
		return WEB_ACCESS_TYPE_GET;
}
#endif
/******************************************************************************
* ������: isAccessPagePub
* ��  ��: 
* �����洢����:
* ����:
* ������        ����        ����/���       ����
* ����ֵ: 0 ��������������
* ˵��:
******************************************************************************/
static unsigned char isAccessPagePub(char_t *url)
{
	if (NULL == strstr(url, "function.asp")
		&&(0 != strcmp(url, "/goform/loginSet")))
	{
		return 0;	
	}	
	
	return 1;
}

/******************************************************************************
* ������: isAccessPageSp
* ��  ��: 
* �����洢����:
* ����:
* ������        ����        ����/���       ����
* ����ֵ: 0 ��������������
* ˵��:
******************************************************************************/
#if 0
static unsigned char isAccessPageSp(char_t *url)
{
		 if ((NULL == strstr(url, "logon.asp"))
			&&(NULL == strstr(url, "logonagain.asp"))
			&&(NULL == strstr(url, "logon_bg02"))
			&&(NULL == strstr(url, "logon.gif"))
			&&(NULL == strstr(url, "logon_chs.gif")))
		{
			return 0;	
		}
		
		return 1;
}
#endif
/******************************************************************************
* ������: ps2WebsSecurityHandler
* ��  ��: 
* �����洢����:
* ����:
* ������        ����        ����/���       ����
* ����ֵ: 0 ��������������
* ˵��:
******************************************************************************/
int ps2WebsSecurityHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, char_t *url, char_t *path, char_t *query)
{

  /*int webAccessType = getWebAccessType(url);
	int ret = 0;
	int i = 0;
	int pageAccessMapLength = (sizeof(spPageAccessLevelMap)/sizeof(T_pageAccessLevel));
	int currentUserLevel = 0;
	char *pageName = websGetVar(wp, T("Html_Name"), T(""));
	*/
	if (isAccessPagePub(url))
	{
		return 1;	
	}
	#if 0
	ret = Web_Auth_Cookie(wp);		/*Web_Auth_Cookie()����0��ͨ��CookieУ�飬����1��δͨ��*/
	
	if (isAccessPageSp(url))
	{
		if (0 == ret)							
		{
			websRedirect(wp, INDEX_WEB_NAME);	
			return 1;
		}
	}
	else
	{
		if (0 != ret)
		{
			websRedirect(wp, LOGON_WEB_NAME);		
			return 1;
		}
	}
	
	currentUserLevel = UserDatabase[UserNumBuff[1]].level;
	
	if (webAccessType == WEB_ACCESS_TYPE_GET)
	{
		for (i=0; i<pageAccessMapLength; i++)
		{
			if (0 == strcmp(spPageAccessLevelMap[i].pageUrl, url))
			{
				if (currentUserLevel < spPageAccessLevelMap[i].pageGetLevel)
				{
					websRedirect(wp, NO_RIGHT_WEB_NAME);
					return 1;	
				}	
			}	
		}	
	}
	else if (webAccessType == WEB_ACCESS_TYPE_POST)
	{
		for (i=0; i<pageAccessMapLength; i++)
		{
			if (0 == strcmp(spPageAccessLevelMap[i].pageUrl+1, pageName))
			{
				if (currentUserLevel < spPageAccessLevelMap[i].pageSetLevel)
				{
					websRedirect(wp, NO_RIGHT_WEB_NAME);
					return 1;	
				}	
			}	
		}
	}
	#endif
	return 1;
}

/******************************************************************************/

