/*******************************************************************************
* COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* �ļ�����:  bsp_afc_matrix.c
* ��    ��:  ����ʱ�Ӳ�������ʱ��ɾ������Ĺ���
* ��    ��:  V0.1
* ��д����:  
* ˵    ��:
* �޸���ʷ:
* �޸�����           �޸���  BugID/CRID     �޸�����
*------------------------------------------------------------------------------
*                                                         �����ļ�
*
*
*******************************************************************************/

/******************************* �����ļ����� *********************************/
/**************************** ����ͷ�ļ�* **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
/**************************** ˽��ͷ�ļ�* **********************************/
#include "bsp_types.h"
#include "../inc/bsp_afc_matrix.h"
/******************************* �ֲ��궨�� ***********************************/
#define MAT_TRAN_LENGTH  2880000
#define MAT_INVER_LENGTH  1000
/******************************** ȫ�ֱ������� ********************************/
double matrix_tran_tmp[MAT_TRAN_LENGTH]={0.0};
s32 matrix_inver_is[MAT_INVER_LENGTH]={0};
s32 matrix_inver_js[MAT_INVER_LENGTH]={0};
/******************************* ȫ�ֱ�������/��ʼ�� **************************/
/******************************* �ֲ����������Ͷ��� ***************************/
/******************************* �ֲ�����ԭ������ *****************************/
/******************************* ����ʵ�� *************************************/
/*******************************************************************************
* ��������: afc_matrix_transpose
* ��    ��:
* ����ĵ�:
* ��������:
* ��    ��:
*******************************************************************************/
void afc_matrix_transpose(const double *a,s32 ml,s32 n,double *b)
{
	s32 s32i,j;
	double *c;
	/*c=(double *)malloc((u32)ml*(u32)n*sizeof(double));
	if (c == NULLPTR)
	{
		return;
	}*/
    c = (double *)matrix_tran_tmp;
	for(s32i=0;s32i<n;s32i++)
	{
		for(j=0;j<ml;j++)
		{
			c[s32i*ml+j]=a[j*n+s32i];
		}
	}

	memcpy((void *)b, (void *)c, (u32)ml*(u32)n*sizeof(double));

	//free(c);
}

/*******************************************************************************
* ��������: afc_matrix_mul
* ��    ��: 
* ����ĵ�:
* ��������:
* ��    ��:
*******************************************************************************/
void afc_matrix_mul(const double *a, const double *b,s32 ml,s32 n,s32 k,double *c)
{
	s32 s32i,j,l,u;
	
	for (s32i=0; s32i<=ml-1; s32i++)
	{
		for (j=0; j<=k-1; j++)
		{
			u=s32i*k+j;
			c[u]=0.0;
			
			for (l=0; l<=n-1; l++)
			{
				c[u]=c[u]+a[s32i*n+l]*b[l*k+j];
			}

		}
	}
}

/*******************************************************************************
* ��������: afc_matrix_inver
* ��    ��: 
* ����ĵ�:
* ��������:
* ��    ��:
*******************************************************************************/
void afc_matrix_inver(double *a,s32 n)
{
	s32 *is,*js,s32i,j,k,l,u,v;
	double db,p;

    is = (s32 *)matrix_inver_is;
	js = (s32 *)matrix_inver_js;
	/*is=(s32 *)malloc((u32)n*sizeof(int));
	if (is == NULLPTR)
	{
		return;
	}
	
	memset((void *)is, 0x0, sizeof((u32)n*sizeof(int)));

	js=(s32 *)malloc((u32)n*sizeof(int));
	if (js == NULLPTR)
	{
		free(is);
		return;
	}
	memset((void *)js, 0x0, sizeof((u32)n*sizeof(int)));
	*/

	for (k=0; k<n; k++)
	{
		db=0.0;

		for (s32i=k; s32i<=n-1; s32i++)
		{
			for (j=k; j<=n-1; j++)
			{
				l=s32i*n+j;
				p=fabs(a[l]);
				
				if (p>db)
				{
					db=p;
					is[k]=s32i;
					js[k]=j;
				}
			}
		}

		if (db+1.0==1.0)
		{
			//free(is);
			//free(js);
			printf("err**not inv\n");
			return;
		}

		if (k != is[k])
		{
			for (j=0; j<=n-1; j++)
			{
				u=k*n+j;
				v=is[k]*n+j;
				p=a[u];
				a[u]=a[v];
				a[v]=p;
			}
		}

		if (k != js[k])
		{
			for (s32i=0; s32i<=n-1; s32i++)
			{
				u=s32i*n+k;
				v=s32i*n+js[k];
				p=a[u];
				a[u]=a[v];
				a[v]=p;
			}
		}

		l=k*n+k;
		a[l]=1.0/a[l];

		for (j=0; j<=n-1; j++)
		{
			if (j!=k)
			{
				u=k*n+j;
				a[u]=a[u]*a[l];
			}
		}

		for (s32i=0; s32i<=n-1; s32i++)
		{
			if (s32i!=k)
			{
				for (j=0; j<=n-1; j++)
				{
					if (j!=k)
					{
						u=s32i*n+j;
						a[u]=a[u]-a[s32i*n+k]*a[k*n+j];
					}
				}
			}
		}

		for (s32i=0; s32i<=n-1; s32i++)
		{
			if (s32i!=k)
			{
				u=s32i*n+k;
				a[u]=-a[u]*a[l];
			}
		}
	}

	for (k=n-1; k>=0; k--)
	{
		if (js[k]!=k)
		{
			for (j=0; j<=n-1; j++)
			{
				u=k*n+j;
				v=js[k]*n+j;
				p=a[u];
				a[u]=a[v];
				a[v]=p;
			}
		}
		
		if (is[k]!=k)
		{
			for (s32i=0; s32i<=n-1; s32i++)
			{
				u=s32i*n+k;
				v=s32i*n+is[k];
				p=a[u];
				a[u]=a[v];
				a[v]=p;
			}
		}
	}

	//free(is);
	//free(js);
}

/******************************* Դ�ļ����� ***********************************/

