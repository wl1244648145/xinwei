#ifndef __IncMemOpReloadH_
#define __IncMemOpReloadH_

/*�������ڴ�ʹ��״��*/
struct usrMemPart
{
    int taskID;
    int allocate;
    int free;
    /*int hold;*/
};
#define M_MAX_TASK_NUM (60)
#define M_OP_NEW  (1)
#define M_OP_FREE (0)
#endif
