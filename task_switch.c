#include <vxWorks.h>
#if defined(PRJ_BUILD)
#include "prjParams.h"
#endif /* defined PRJ_BUILD */

#include "stdio.h"
#include "cpld/cpldFunc.h"
#include "VpltSerialComm/VpltSerialComm.h"
#include "VpltCommon/VpltCommon.h"
#include <iprip.h>
#include <ipftps.h>
#include "asb8360.h"
#include "msgQLibCommon.h"
#include "semLib.h"

#define LOOP (500)

typedef union timeBase
{
    struct
    {
        UINT32 hi; /* high value */
        UINT32 lo; /* low value */
    } val32;
    UINT64 val64; /* full 64 bits integer value */
} TIME_BASE;

// usRecord[tloop] = (UINT64)(((record[tloop][1].val64 - record[tloop][0].val64) * 1000000 * 4) / (UINT64)SYS_CLK_FREQ);

/******************************************************************************
 *
 * usrAppInit - initialize the users application
 */

char *task1_name = "task1", *task2_name = "task2", *test_name = "test";
int task1_prio, task2_prio = 0;
int test_prio = 1;
int task1_ss, task2_ss, test_ss = 1024 * 2;
int task1_id, task2_id, test_id = 0;

int task_1(void)
{
    int self_id = 0;
    int uCounter = 0;

    self_id = taskIdSelf();
    taskSuspend(self_id);

    sched_yield();

    printf("task 1 will start the operation\n\r");

    while ((uCounter++) < LOOP)
    {
        sched_yield();
    }
}

int task_2(void)
{
    int self_id = 0;
    int uCounter = 0;
    TIME_BASE tt_start, tt_end = {0};
    UINT64 uUs = 0;

    taskResume(task1_id);

    printf("task 2 will start the operation\n\r");

    vxTimeBaseGet(&tt_start.val32.hi, &tt_start.val32.lo);
    while ((uCounter++) < LOOP)
    {
        sched_yield();
    }
    vxTimeBaseGet(&tt_end.val32.hi, &tt_end.val32.lo);
    // usRecord[tloop] = (UINT64)(((record[tloop][1].val64 - record[tloop][0].val64) * 1000000 * 4) / (UINT64)SYS_CLK_FREQ);
    uUs = (UINT64)(((tt_end.val64 - tt_start.val64) * 1000000 * 4) / (UINT64)SYS_CLK_FREQ);

    sleep(2);

    printf("the loop %d times switch time is %lld \n\r", uCounter, uUs);
}

int test_task(void)
{

    /* calculate the loop overhead */
    {
        int uCounter = 0;
        TIME_BASE tt_start, tt_end = {0};
        UINT64 uUs = 0;

        vxTimeBaseGet(&tt_start.val32.hi, &tt_start.val32.lo);
        while ((uCounter++) < 2 * LOOP)
        {
        }
        vxTimeBaseGet(&tt_end.val32.hi, &tt_end.val32.lo);
        uUs = (UINT64)(((tt_end.val64 - tt_start.val64) * 1000000 * 4) / (UINT64)SYS_CLK_FREQ);
        printf("loop overhead : %lld \n\r", uUs);
    }

    /* calculate the dir overhead */
    printf("now, ignore the effect of dir overhead\n\r");

    /* intialize the task1 and task2 */
    task1_id = taskCreate(task1_name, task1_prio, 0, task1_ss, task_1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    task2_id = taskCreate(task2_name, task2_prio, 0, task2_ss, task_2, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    taskActivate(task1_id);
    taskActivate(task2_id);
}

void usrAppInit(void)
{
    test_id = taskCreate(test_name, test_prio, 0, test_ss, test_task, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    taskActivate(test_id);
}
