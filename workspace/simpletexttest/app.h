#pragma once

#define HIGH_PRIORITY 8
#define MAIN_PRIORITY 10
#define LOW_PRIORITY 12

#ifndef STACK_SIZE
#define	STACK_SIZE		4096		/* タスクのスタックサイズ */
#endif /* STACK_SIZE */

extern int main_task(intptr_t unused); 
extern void high_task(intptr_t unused);
extern void low_task(intptr_t unused);
