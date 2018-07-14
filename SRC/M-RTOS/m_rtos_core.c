#define M_RTOS_GLOBALS
#include "M-RTOS.h"
#undef M_RTOS_GLOBALS

/***************************************************************************************************
																			m_rtos idle task
***************************************************************************************************/
#define IDLE_STACK_SIZE	128
#define IDLE_TASK_PRIORITY	0xffffffff

static   m_rtos_u32   IdleTaskStk[IDLE_STACK_SIZE];	/*��������ջ*/
static	 m_rtos_tcb		IdleTaskTcb;									/*���������*/
/**
 * @brief  idle task
 * @param  none
 * @author ma57457@163.com
 * @date 2018-7-1
 */
void m_rtos_idle_task( void *p_arg )
{
	p_arg = p_arg;
	while(1)
		__ASM("NOP");
}
/**
 * @brief  idle task
 * @param  none
 * @author ma57457@163.com
 * @date 2018-7-1
 */
void m_rtos_idle_init( void *p_arg )
{
	M_RTOS_TaskCreate ((m_rtos_tcb*)      &IdleTaskTcb, 
	              (M_RTOS_TASK_PTR ) 			m_rtos_idle_task, 
	              (void *)       					0,
	              (m_rtos_u32*)     			&IdleTaskStk,
	              (m_rtos_u32) 						IDLE_STACK_SIZE,
								(m_rtos_u32)						IDLE_TASK_PRIORITY);
}
/***************************************************************************************************
																			m_rtos ready task list
***************************************************************************************************/
/**
 * @brief  �����б���ӽڵ�
 * @param  none
 * @author ma57457@163.com
 * @date 2018-6-18
 */
void M_RTOS_InsertNodeTaskList(m_rtos_tcb *p_tcb,m_rtos_task_list **task_list)
{
	m_rtos_task_list *task_prt_new = (m_rtos_task_list*)malloc(sizeof(m_rtos_task_list));
	/*������Ҫ����ڴ��ȡʧ�ܵĴ���*/
	
	if(NULL == (*task_list)){
		(*task_list) = task_prt_new;
		(*task_list)->TaskPtr = p_tcb;
		(*task_list)->NextNode = (*task_list);
	}else{
		task_prt_new->TaskPtr = p_tcb;
		task_prt_new->NextNode = (*task_list)->NextNode;
		(*task_list)->NextNode = task_prt_new;
	}

}
/**
 * @brief  �����б�ڵ�ɾ��
 * @param  none
 * @author ma57457@163.com
 * @date 2018-6-18
 */
void M_RTOS_DeleteNodeTaskList(m_rtos_tcb *p_tcb,m_rtos_task_list **task_list)
{
	m_rtos_task_list *del_node = NULL;
	
	if(NULL == task_list){
		return;
	}else{
		del_node = (*task_list)->NextNode;
		for (;del_node != (*task_list)->NextNode;del_node = (*task_list)->NextNode){
			if (((del_node->TaskPtr) == p_tcb) && (del_node->TaskPtr->Priority != IDLE_TASK_PRIORITY)){
				(*task_list)->NextNode = del_node->NextNode;
				free(del_node);
				break;
			}
		}
	}
}
/**
 * @brief  �����б�ڵ����
 * @param  none
 * @author ma57457@163.com
 * @date 2018-6-18
 */
m_rtos_u8 M_RTOS_SearchNodeTaskList(m_rtos_tcb *p_tcb,m_rtos_task_list **task_list)
{
	m_rtos_u8 ret = NULL;
	m_rtos_task_list *bak_list = *task_list;
	
	if(p_tcb == (*task_list)->TaskPtr){
		ret = 1;
		return ret;
	}
	
	(*task_list) = (*task_list)->NextNode;
	for (; bak_list != (*task_list); (*task_list) = (*task_list)->NextNode){
		if(p_tcb == (*task_list)->TaskPtr){
			ret = 1;
			break;
		}
	}
	
	return ret;
}
/**
 * @brief  �����б�ڵ����,������һ���ڵ�
 * @param  none
 * @author ma57457@163.com
 * @date 2018-6-18
 */
m_rtos_task_list* M_RTOS_SearchNodeTaskList_Prev(m_rtos_tcb *p_tcb,m_rtos_task_list **task_list)
{
	m_rtos_task_list *ret_addr = (m_rtos_task_list *)NULL;
	m_rtos_task_list *bak_list = *task_list;
	
	if(p_tcb == (*task_list)->NextNode->TaskPtr){
		ret_addr = (*task_list);
		return ret_addr;
	}
	
	(*task_list) = (*task_list)->NextNode;
	for (; bak_list != (*task_list); (*task_list) = (*task_list)->NextNode){
		if(p_tcb == (*task_list)->NextNode->TaskPtr){
			ret_addr = (*task_list);
			return ret_addr;
		}
	}
	
	return ret_addr;
}
/***************************************************************************************************
								m_rtos  ����������������
***************************************************************************************************/
/**
 * @brief  ��������
 * @param  *target_tcb ��Ҫ�����Ŀ������
 * @author ma57457@163.com
 * @date 2018-6-18
 */
void m_rtos_task_move(m_rtos_tcb *target_tcb,m_rtos_task_list **src_list,m_rtos_task_list **tar_list)
{
	m_rtos_task_list *rdy_node = (m_rtos_task_list *)NULL;
	m_rtos_task_list *prev_node = M_RTOS_SearchNodeTaskList_Prev(target_tcb,src_list);
	
	if(prev_node != NULL){
		rdy_node = prev_node->NextNode;
		prev_node->NextNode = prev_node->NextNode->NextNode;
		
		if(NULL == (*tar_list)){
			(*tar_list) = rdy_node;
			(*tar_list)->NextNode = (*tar_list);
		}else{
			rdy_node->NextNode = (*tar_list)->NextNode->NextNode;
			(*tar_list)->NextNode = rdy_node;
		}
	}
		
	
}
/***************************************************************************************************
								m_rtos  cotrex switch
***************************************************************************************************/
/**
 * @brief  ��ȡ��ǰ�����б������ȼ���������飬��һ��ȱ������Ҫ�����б�ȫ������
 * @param  none
 * @author ma57457@163.com
 * @date 2018-6-18
 */
m_rtos_tcb*  m_rtoe_get_highest_rdy_task(void)
{
	m_rtos_tcb *ret_tcb	= M_RTOS_CurPtr;
	m_rtos_task_list *bak_node = M_RTOS_RdyTaskList;
	
	if( NULL == M_RTOS_SearchNodeTaskList(M_RTOS_CurPtr,&M_RTOS_RdyTaskList))
		ret_tcb = M_RTOS_RdyTaskList->TaskPtr;
	
	if(M_RTOS_RdyTaskList == M_RTOS_RdyTaskList->NextNode)/*û����������*/
		return ret_tcb = M_RTOS_RdyTaskList->TaskPtr;

	M_RTOS_RdyTaskList = M_RTOS_RdyTaskList->NextNode;/*���ֻ��һ����������*/
	if ((M_RTOS_RdyTaskList->TaskPtr->Priority <= ret_tcb->Priority) || (M_RTOS_RdyTaskList->TaskPtr->Virtual_Pri <= ret_tcb->Virtual_Pri))
		ret_tcb =  M_RTOS_RdyTaskList->TaskPtr;
		
	for (; M_RTOS_RdyTaskList != bak_node; M_RTOS_RdyTaskList = M_RTOS_RdyTaskList->NextNode){
			if ((M_RTOS_RdyTaskList->TaskPtr->Priority <= ret_tcb->Priority) || (M_RTOS_RdyTaskList->TaskPtr->Virtual_Pri <= ret_tcb->Virtual_Pri))
				ret_tcb =  M_RTOS_RdyTaskList->TaskPtr;
	}
	
	return ret_tcb;
}
/**
 * @brief  cotrex_switch
 * @param  none
 * @author ma57457@163.com
 * @date 2018-6-18
 */
void m_rtos_sched(void)
{  
	M_RTOS_RdyPtr = m_rtoe_get_highest_rdy_task();
	
	if(M_RTOS_RdyPtr == M_RTOS_CurPtr)
		return;
	
	m_rtos_contex_switch();
}
/***************************************************************************************************
																			m_rtos  cotrex switch
***************************************************************************************************/

/***************************************************************************************************
																			tick
***************************************************************************************************/
/**
 * @brief  tick initialize
 * @param  tick_ms uint is ms
 * @author ma57457@163.com
 * @date 2018-6-18
 */
void m_rtos_tick_init(m_rtos_u32 tick_ms)
{
	SysTick->LOAD = tick_ms*SystemCoreClock/1000-1;
	NVIC_SetPriority(SysTick_IRQn,(1<<__NVIC_PRIO_BITS)-1);
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk|\
									SysTick_CTRL_TICKINT_Msk|\
									SysTick_CTRL_ENABLE_Msk;
}
/**
 * @brief  tick delay
 * @param  tick_ms uint is ms
 * @author ma57457@163.com
 * @date 2018-6-18
 */
void m_rtos_delay(m_rtos_u32 tick)
{
	M_RTOS_CurPtr->DelayTicks = tick;
	
	m_rtos_task_move(M_RTOS_CurPtr,&M_RTOS_RdyTaskList,&M_RTOS_PendTaskList);
	
	m_rtos_sched();
}
/**
 * @brief  m_rtos_pend_list_deal
 * @param  tick_ms uint is ms
 * @author ma57457@163.com
 * @date 2018-6-18
 */
void m_rtos_pend_list_deal()
{
	m_rtos_task_list *bak_node = M_RTOS_PendTaskList;
	m_rtos_task_list *del_dode = NULL;
	
	if(M_RTOS_PendTaskList != NULL){/*�������������*/
		M_RTOS_PendTaskList = M_RTOS_PendTaskList->NextNode;
		/*�ٶ�ֻ��һ��������������*/
			del_dode = M_RTOS_PendTaskList;

			if(del_dode->TaskPtr->Wait_Semaphore != NULL){								 /*�ȴ��ź����Ĵ���*/
				if (NULL == del_dode->TaskPtr->Wait_Semaphore->OwnerTaskPtr){/*��ȡ�ź����ɹ�*/
					del_dode->TaskPtr->Wait_Semaphore->OwnerTaskPtr = del_dode->TaskPtr;
					del_dode->TaskPtr->DelayTicks = 0;
				}else  if(del_dode->TaskPtr->DelayTicks){
					if (del_dode->TaskPtr->DelayTicks != INFINITE_WAIT)				/*���޵ȴ���������ʱʱ��*/
						del_dode->TaskPtr->DelayTicks--;
				}
			}else		if (del_dode->TaskPtr->DelayTicks != NULL){
				del_dode->TaskPtr->DelayTicks--;
			}
			
			if (0 == del_dode->TaskPtr->DelayTicks){
				if(bak_node == del_dode)
					bak_node = bak_node->NextNode;
				m_rtos_task_move(del_dode->TaskPtr,&M_RTOS_PendTaskList,&M_RTOS_RdyTaskList);
			}
		/*�ٶ����������������*/
		bak_node = M_RTOS_PendTaskList;
		for (; M_RTOS_PendTaskList != bak_node; M_RTOS_PendTaskList = M_RTOS_PendTaskList->NextNode){
			del_dode = M_RTOS_PendTaskList;

			if(del_dode->TaskPtr->Wait_Semaphore != NULL){								 /*�ȴ��ź����Ĵ���*/
				if (NULL == del_dode->TaskPtr->Wait_Semaphore->OwnerTaskPtr){/*��ȡ�ź����ɹ�*/
					del_dode->TaskPtr->Wait_Semaphore->OwnerTaskPtr = del_dode->TaskPtr;
					del_dode->TaskPtr->DelayTicks = 0;
				}else  if(del_dode->TaskPtr->DelayTicks){
					if (del_dode->TaskPtr->DelayTicks != INFINITE_WAIT)				/*���޵ȴ���������ʱʱ��*/
						del_dode->TaskPtr->DelayTicks--;
				}
			}else		if (del_dode->TaskPtr->DelayTicks != NULL){
				del_dode->TaskPtr->DelayTicks--;
			}
			
			if (0 == del_dode->TaskPtr->DelayTicks){
				if(bak_node == del_dode)
					bak_node = bak_node->NextNode;
				m_rtos_task_move(del_dode->TaskPtr,&M_RTOS_PendTaskList,&M_RTOS_RdyTaskList);
			}
		}
	}
}
/***************************************************************************************************
																			ϵͳ��ʼ��
	* 1��ջ��ʼ����
	* 2�����������ʼ��
***************************************************************************************************/
/**
 * @brief  m_rtos_initialize
 * @param  none
 * @author ma57457@163.com
 * @date 2018-6-18
 */
void m_rtos_init(void)
{
	m_rtos_set_interrupt_stop();
	
	m_rtos_idle_init(NULL);/*���������ʼ��*/
	
	M_RTOS_CurPtr = (m_rtos_tcb *)0;
	M_RTOS_RdyPtr = &IdleTaskTcb;
	M_RTOS_PendTaskList = (m_rtos_task_list *)0;
	
	m_rtos_tick_init(10);/*10ms slice*/
}
/**
 * @brief  task stack initialize
 * @param  none
 * @author ma57457@163.com
 * @date 2018-6-18
 */
m_rtos_u32 *M_RTOS_TaskStackInit (M_RTOS_TASK_PTR  p_task,
                        void         *p_arg,
                        m_rtos_u32      *p_stk_base,
                        m_rtos_u32 stk_size)
{
	m_rtos_u32  *p_stk;

	p_stk = &p_stk_base[stk_size];
															/* �쳣����ʱ�Զ�����ļĴ���                              */
	*--p_stk = (m_rtos_u32)0x01000000u;                        /* xPSR��bit24������1                                     */
	*--p_stk = (m_rtos_u32)p_task;                             /* �������ڵ�ַ                                         */
	*--p_stk = (m_rtos_u32)0x14141414u;                        /* R14 (LR)                                               */
	*--p_stk = (m_rtos_u32)0x12121212u;                        /* R12                                                    */
	*--p_stk = (m_rtos_u32)0x03030303u;                        /* R3                                                     */
	*--p_stk = (m_rtos_u32)0x02020202u;                        /* R2                                                     */
	*--p_stk = (m_rtos_u32)0x01010101u;                        /* R1                                                     */
	*--p_stk = (m_rtos_u32)p_arg;                              /* R0 : �����β�                                          */
															/* �쳣����ʱ���ֶ�����ļĴ���                            */
	*--p_stk = (m_rtos_u32)0x11111111u;                        /* R11                                                    */
	*--p_stk = (m_rtos_u32)0x10101010u;                        /* R10                                                    */
	*--p_stk = (m_rtos_u32)0x09090909u;                        /* R9                                                     */
	*--p_stk = (m_rtos_u32)0x08080808u;                        /* R8                                                     */
	*--p_stk = (m_rtos_u32)0x07070707u;                        /* R7                                                     */
	*--p_stk = (m_rtos_u32)0x06060606u;                        /* R6                                                     */
	*--p_stk = (m_rtos_u32)0x05050505u;                        /* R5                                                     */
	*--p_stk = (m_rtos_u32)0x04040404u;                        /* R4                                                     */
	return (p_stk);
}
/**
 * @brief  initialize task
 * @param  *p_tcb task control bulk point
 * @param  p_task task point
 * @param  *p_arg �������
 * @param	 *p_stk_base ����ջ����ַ
 * @param  stk_size	ջ��С
 * @param  stk_pri �������ȼ�
 * @author ma57457@163.com
 * @date 2018-6-18
 */
void M_RTOS_TaskCreate (m_rtos_tcb 				*p_tcb, 
												M_RTOS_TASK_PTR   p_task, 
												void          		*p_arg,
												m_rtos_u32        *p_stk_base,
												m_rtos_u32  			stk_size,
												m_rtos_u32	 			stk_pri)
{
	m_rtos_u32       *p_sp;

	p_sp = M_RTOS_TaskStackInit (p_task,
	p_arg,
	p_stk_base,
	stk_size);
	p_tcb->StkPtr = p_sp;
	p_tcb->StkSize = stk_size;
	p_tcb->Priority = stk_pri;
	p_tcb->Virtual_Pri = IDLE_TASK_PRIORITY;/*�������ȼ�ֱ���������*/
	p_tcb->Wait_Semaphore = NULL;
	M_RTOS_InsertNodeTaskList(p_tcb,&M_RTOS_RdyTaskList);
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @author ma57457@163.com
  * @date 2018-7-14
  */
void SysTick_Handler(void)
{
	m_rtos_pend_list_deal();
	m_rtos_sched();
}
/***************************************************************************************************
																			semaphore 
	*	1��ֻ�����˵��������ź�������������������ж�������군�ˣ�ϵͳֱ�ӹҵ�
	* 2����Ϣ����û��д
***************************************************************************************************/
/**
  * @brief  semaphore initialize
  * @param  None
  * @author ma57457@163.com
  * @date 2018-7-14
  */
void M_RTOS_SemaphoreInit(m_rtos_semaphore *semaphore,m_rtos_u8 semaphore_type)
{
	if(0 == semaphore_type){//��ֵ�ź���
		semaphore->OwnerTaskPtr = NULL;
		semaphore->Type = MUTEX;
		semaphore->Value = NULL;
	}else if(1 == semaphore_type){
		/*��һ��д*/
	}
}
/**
  * @brief  semaphore pend
	* @param  semaphore:�ź���ָ�� wait_time����ʱ�ȴ�ʱ��
	* @param  return MUTEX_GET_FILE ��ȡʧ�� MUTEX_GET_SUCCESS ��ȡ�ɹ�
  * @author ma57457@163.com
  * @date 2018-7-14
  */
m_rtos_u8 M_RTOS_SemaphorePend(m_rtos_semaphore *semaphore,m_rtos_u32 wait_time)
{
	m_rtos_u8 ret=SEMAPHORE_GET_FILE;
	
	if(NULL == semaphore)
		return ret;
	
	switch(semaphore->Type)
	{
		case MUTEX:{
			m_rtos_set_interrupt_stop();
			if (NULL == semaphore->OwnerTaskPtr || M_RTOS_CurPtr == semaphore->OwnerTaskPtr){
				semaphore->OwnerTaskPtr = M_RTOS_CurPtr;
				ret = SEMAPHORE_GET_SUCCESS;
			}else{
				M_RTOS_CurPtr->Wait_Semaphore = semaphore;
				if (semaphore->OwnerTaskPtr->Priority < M_RTOS_CurPtr->Priority)
					semaphore->OwnerTaskPtr->Virtual_Pri = M_RTOS_CurPtr->Priority;
				m_rtos_set_interrupt_start();
				m_rtos_delay (wait_time);/*��������*/
				
				if (M_RTOS_CurPtr->Wait_Semaphore == semaphore)
					ret = SEMAPHORE_GET_SUCCESS;
			}
			m_rtos_set_interrupt_start();
		}
			break;
		case MAIL_BOX:
			/*�ڶ�����д*/
			break;
		default:
			break;
	}
	return ret;
}
/**
  * @brief  semaphore post
	* @param  semaphore:�ź���ָ��
	* @param  return MUTEX_GET_FILE ��ȡʧ�� MUTEX_GET_SUCCESS ��ȡ�ɹ�
  * @author ma57457@163.com
  * @date 2018-7-14
  */
m_rtos_u8 M_RTOS_SemaphorePost(m_rtos_semaphore *semaphore)
{
	m_rtos_u8 ret=SEMAPHORE_GET_FILE;
	
	if(NULL == semaphore)
		return ret;
	m_rtos_set_interrupt_stop();
	switch(semaphore->Type)
	{
		case MUTEX:{
			semaphore->OwnerTaskPtr = NULL;
			M_RTOS_CurPtr->Wait_Semaphore = NULL;
			M_RTOS_CurPtr->Virtual_Pri = IDLE_TASK_PRIORITY;
			ret = SEMAPHORE_GET_SUCCESS;
		}
			break;
		case MAIL_BOX:
			/*�ڶ�����д*/
			break;
		default:
			break;
	}
	m_rtos_set_interrupt_start();
	return ret;
}
/***************************************************************************************************
																			memory manage
	*	1���ڶ�����룬û�и������ô�����Ƭ���⣬��ǰ���ñ������ṩ�Ĵպ�һ��
***************************************************************************************************/

