/*******************************************************************************
* COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* �ļ�����: bsp_afc_matrix.h
* ��    ��:dd_matrix.c��ͷ�ļ�
* ��    ��: V0.1
* ��д����: 
* ˵    ��:
* �޸���ʷ:
* �޸�����           �޸���  BugID/CRID     �޸�����
*------------------------------------------------------------------------------
*                                                         �����ļ�
*
*
*******************************************************************************/
/******************************** ͷ�ļ�������ͷ ******************************/
#ifndef DD_MATRIX_H
#define DD_MATRIX_H

/******************************** �����ļ����� ********************************/
/******************************** ��ͳ������� ********************************/
/******************************** ���Ͷ��� ************************************/
/******************************** ȫ�ֱ������� ********************************/
/******************************** �ⲿ����ԭ������ ****************************/
void afc_matrix_transpose(const double *a,s32 m,s32 n,double *b);
void afc_matrix_mul(const double *a, const double *b,s32 m,s32 n,s32 k,double *c);
void afc_matrix_inver(double *a,s32 n);
/******************************** ͷ�ļ�������β ******************************/
#endif
/******************************** ͷ�ļ����� **********************************/