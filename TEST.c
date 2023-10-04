/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #2
*********************************************************************************************************
*/


 
#include <stdio.h>
/*******************************************************************************************************/


#include "includes.h"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/ 

#define          TASK_STK_SIZE     512                /* Size of each task's stacks (# of WORDs)       */

#define          TASK_START_ID       0                /* Application tasks IDs                         */
#define          TASK_CLK_ID         1
#define          TASK_1_ID           2
#define          TASK_2_ID           3
#define          TASK_3_ID           4
#define          TASK_4_ID           5
#define          TASK_5_ID           6

#define          TASK_START_PRIO    10                /* Application tasks priorities                  */
#define          TASK_CLK_PRIO      11
#define          TASK_1_PRIO        12
#define          TASK_2_PRIO        13
#define          TASK_3_PRIO        14
#define          TASK_4_PRIO        15
#define          TASK_5_PRIO        16

 
#define          PERIODIC_TASK_START_ID      20   
#define          PERIODIC_TASK_START_PRIO    20                                 


 
typedef struct {
    INT32U RemainTime;
    INT32U ExecutionTime;
    INT32U Period;
    INT32U Deadline;
    INT8U TaskID;
} TASK_EXTRA_DATA;
/*******************************************************************************************************/


/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/ 

OS_STK        TaskStartStk[TASK_STK_SIZE];            /* Startup    task stack                         */
OS_STK        TaskClkStk[TASK_STK_SIZE];              /* Clock      task stack                         */
OS_STK        Task1Stk[TASK_STK_SIZE];                /* Task #1    task stack                         */
OS_STK        Task2Stk[TASK_STK_SIZE];                /* Task #2    task stack                         */
OS_STK        Task3Stk[TASK_STK_SIZE];                /* Task #3    task stack                         */
OS_STK        Task4Stk[TASK_STK_SIZE];                /* Task #4    task stack                         */
OS_STK        Task5Stk[TASK_STK_SIZE];                /* Task #5    task stack                         */


OS_EVENT     *AckMbox;                                /* Message mailboxes for Tasks #4 and #5         */
OS_EVENT     *TxMbox;
OS_EVENT     *sem;
OS_STK TaskStk[8][TASK_STK_SIZE];
TASK_EXTRA_DATA  TaskExtraData[8]; 
INT8U NumberOfTasks; 
INT8U ExecutionTime[8]; 
INT8U PeriodTime[8]; 

INT8U *TaskList;
INT8U *TempPeriodTime;
// INT8U *priority;
INT32U MyStartTime;
INT32U task_display_counter = 0;
/*******************************************************************************************************/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  TaskStart(void *data);                  /* Function prototypes of tasks                  */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);
        void  TaskClk(void *data);
    void taskTime();
 
	void  PeriodicTask(void *data); 
	void  quickSort (INT8U *PeriodTime , INT8U *TaskList,const INT8U left, const INT8U right);
	void  swap(INT8U* a,INT8U *b);
void selectionSort(INT8U *PeriodTime , INT8U *TaskList, INT8U size);
    //void selectionSort(INT8U *exeTime , INT8U *TaskList, INT8U size,INT8U *PeriodTime);
/*******************************************************************************************************/

/*$PAGE*/
/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/
 


void main (void){
    OS_STK *ptos;
    OS_STK *pbos;
    INT32U  size;
	INT8U testt;
    INT8U temp;

 
    FILE *InputFile; 
    INT8U i;
    INT8U j;
    InputFile = fopen("Input1.txt","r");
    fscanf(InputFile, "%d", &NumberOfTasks);
    TaskList = (int*)malloc(sizeof(int)*NumberOfTasks);
    for(i=0; i< NumberOfTasks; i++)
    {
         // read file
	fscanf(InputFile,"%d %d", &PeriodTime[i], &ExecutionTime[i]); 
        TaskExtraData[i].ExecutionTime = ExecutionTime[i] * OS_TICKS_PER_SEC;
        TaskExtraData[i].Period = PeriodTime[i] * OS_TICKS_PER_SEC; 
        TaskExtraData[i].Deadline = PeriodTime[i] * OS_TICKS_PER_SEC; 
        TaskExtraData[i].RemainTime = ExecutionTime[i] * OS_TICKS_PER_SEC;
        TaskExtraData[i].TaskID=i+1;
        TaskList[i] = i; 
    }
    fclose(InputFile);

    //sorting file data
	selectionSort(PeriodTime,TaskList,NumberOfTasks);


/*******************************************************************************************************/


    PC_DispClrScr(DISP_FGND_WHITE);                        /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    PC_ElapsedInit();                                      /* Initialized elapsed time measurement     */
	
    ptos        = &TaskStartStk[TASK_STK_SIZE - 1];        /* TaskStart() will use Floating-Point      */
    pbos        = &TaskStartStk[0];
    size        = TASK_STK_SIZE;
    OSTaskStkInit_FPE_x86(&ptos, &pbos, &size);  
sem=OSSemCreate(1);          
    OSTaskCreateExt(TaskStart,
                   (void *)0,
                   ptos,
                   TASK_START_PRIO,
                   TASK_START_ID,
                   pbos,
                   size,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

    OSStart();                                             /* Start multitasking                       */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               STARTUP TASK
*********************************************************************************************************
*/
void swap(INT8U* a,INT8U *b){
    INT8U temp = *a;
    *a = *b;
    *b = temp;
}


// toby: selection sort
void selectionSort(INT8U *PeriodTime , INT8U *TaskList, INT8U size){
    INT8U i, j, min_idx, temp;
    for (i=0; i<size-1; i++) {
        min_idx = i;
        for (j=i+1; j<size; j++) {
            if (PeriodTime[j] < PeriodTime[min_idx]) {
                min_idx = j;
            }
        }
        swap(&PeriodTime[min_idx],&PeriodTime[i]);
        swap(&TaskList[min_idx],&TaskList[i]);
	//PC_DispStr( 0,  12, PeriodTime[i], DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    }
}

 
void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT16S     key;
    INT16U K;
    INT16U i;
    INT16U j;
    pdata = pdata;                                         /* Prevent compiler warning                 */

    TaskStartDispInit();                                   /* Setup the display                        */

    OS_ENTER_CRITICAL();                                   /* Install uC/OS-II's clock tick ISR        */
    PC_VectSet(0x08, OSTickISR);
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();
    
    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    AckMbox = OSMboxCreate((void *)0);                     /* Create 2 message mailboxes               */
    TxMbox  = OSMboxCreate((void *)0);

    TaskStartCreateTasks();                                /* Create all other tasks                   */

    for (;;) {
        TaskStartDisp();                                   /* Update the display                       */

        if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Yes, return to DOS                       */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDly(200);                       /* Wait one second                          */
                             /* Wait one second                          */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/
  
 
 
static  void  TaskStartDispInit (void)
{

    char s[80]; 
    INT8U i;
    INT8U j;

    PC_DispStr( 0,  0, "                            Final Project on uC/OS-II                           ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr( 0,  1, "                                                                                ", DISP_FGND_WHITE + DISP_BGND_BLACK);
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_CYAN + DISP_BGND_BLUE);
    PC_DispStr( 0,  3, "                              B0928012                                          ", DISP_FGND_LIGHT_GRAY + DISP_BGND_BLUE);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_CYAN);
    PC_DispStr( 0,  5, "                        SJF  NoPreempt Scheduling Results                             ", DISP_FGND_BLACK + DISP_BGND_CYAN);
    PC_DispStr( 0,  6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  8, "Task           Start Time  End Time    Deadline    Period      Excution    Run  ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  9, "-------------  ----------  ----------  ----------  ----------  ----------  ---- ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    for(i=0; i< NumberOfTasks; i++)
    {
        INT8U p_time = TaskExtraData[i].Period/OS_TICKS_PER_SEC;
        INT8U e_time = TaskExtraData[i].ExecutionTime/OS_TICKS_PER_SEC;
        sprintf(s,"Task%1d()    :                                       %2d          %2d               ", TaskExtraData[i].TaskID,p_time , e_time);
        PC_DispStr(0, 10+i, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    }
    for(j=(NumberOfTasks+10); j<22; j++)
    {
        PC_DispStr( 0, j, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    }


    PC_DispStr( 0, 22, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 23, "#Task switch/sec:                                                               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 24, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);
  
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           UPDATE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDisp (void)
{
    char   s[80];


    sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
    PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%3d", OSCPUUsage);                                 /* Display CPU usage in %               */
    PC_DispStr(36, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%5d", OSCtxSwCtr);                                 /* Display #context switches per second */
    PC_DispStr(18, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "V%4.2f", (float)OSVersion() * 0.01);               /* Display uC/OS-II's version number    */
    PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    switch (_8087) {                                               /* Display whether FPU present          */
        case 0:
             PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 1:
             PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 2:
             PC_DispStr(71, 22, "80287 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 3:
             PC_DispStr(71, 22, "80387 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
    INT8U i;
    char s [80];
    INT8U taskID;
    MyStartTime = OSTimeGet(); 
    // MyStartTime = 0;
    OSTaskCreateExt(TaskClk,
                   (void *)0,
                   &TaskClkStk[TASK_STK_SIZE - 1],
                   TASK_CLK_PRIO,
                   TASK_CLK_ID,
                   &TaskClkStk[0],
                   TASK_STK_SIZE,
                   (void *)0,
                   OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);


    for(i=0; i< NumberOfTasks; i++)
    {
         
        taskID = TaskList[i]; //將優先權較高的任務放置前面
        OSTaskCreateExt(PeriodicTask,
                       (void *)0,
                       &TaskStk[taskID][TASK_STK_SIZE-1],
                       (PERIODIC_TASK_START_PRIO + i),
                       (PERIODIC_TASK_START_ID + taskID),
                       &TaskStk[taskID][0],
                       TASK_STK_SIZE,
                       &TaskExtraData[taskID],
                       OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

        //sprintf(s,"           Task%1d is created with piority %1d                                   ", i+1 , PERIODIC_TASK_START_PRIO + i);
	//PC_DispStr(0, 15+i, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);

    }
}
/*******************************************************************************************************/

/*$PAGE*/
/*
*********************************************************************************************************
*                                               CLOCK TASK
*********************************************************************************************************
*/
 
void  TaskClk (void *data)
{
    char s[40];
    INT32U i;

    data = data;
    for (;;) {
        PC_GetDateTime(s);
        PC_DispStr(60, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDly(OS_TICKS_PER_SEC);
       
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                        Periodic TASK   
*********************************************************************************************************
*   **********************************************************************************************/
INT32U  totaltime=0;
    INT32U start;
    INT32U end;
void  PeriodicTask (void *pdata)
{
    
    INT8U  x;
    INT8U  err;
    TASK_EXTRA_DATA *MyPtr; 
    char s[80];
    char p[34];
    INT32U i;
    INT32U j;

    
    INT32U waitTime;
 
    pdata=pdata;
    MyPtr = OSTCBCur->OSTCBExtPtr;
    x =0;
    MyPtr->Deadline = MyStartTime + MyPtr->Period;    
    MyPtr->RemainTime = MyPtr->ExecutionTime;
    
    for (;;){
	OSSemPend(sem,0,&err);

        // 顯示任務ID和context switching 的start time         
        
	sprintf(s, "%d->",MyPtr->TaskID);        
        PC_DispStr( task_display_counter,  18, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	totaltime=OSTimeGet()/OS_TICKS_PER_SEC-1;
	sprintf(s, "%d->",totaltime);
	PC_DispStr( task_display_counter,  20, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);

	task_display_counter = task_display_counter+4; 
	x++;
	sprintf(s, "%4d",x); 
        PC_DispStr(75, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	sprintf(s, "%10d ",OSTimeGet()/OS_TICKS_PER_SEC-1); 
        PC_DispStr(15, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY); 
	sprintf(s, "%10d ", MyPtr->Deadline/OS_TICKS_PER_SEC-1); 
        PC_DispStr(39, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
         
        // 執行for loop 去fit 1秒
	start=OSTimeGet();
	for(i=0;i<MyPtr->ExecutionTime;i++)
	{
		for(j=0;j<3100000;j++)
		{

		}
	}
	end=OSTimeGet();
	sprintf(s, "%d",(end-start)); 
	PC_DispStr( 0,17, s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
        
	MyPtr->Deadline = MyPtr->Deadline + MyPtr->Period;

	sprintf(s, "%10d ",OSTimeGet()/OS_TICKS_PER_SEC-1); 
        PC_DispStr(27, 10 + TaskList[OSPrioCur - PERIODIC_TASK_START_PRIO], s, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	OSSemPost(sem);
        waitTime=MyPtr->Deadline-MyPtr->Period-OSTimeGet();//想一下每個工作要wait 幾秒 這裡要改喔
        if(waitTime>0)
        {
            OSTimeDly(waitTime);
        }   
	
    }
}

/*******************************************************************************************************/

