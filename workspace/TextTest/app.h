#pragma once

#define MAIN_PRIORITY	12		/* メインタスクの優先度 */
								/* HIGH_PRIORITYより高くすること */

#define HIGH_PRIORITY	9		/* 並行実行されるタスクの優先度 */
#define MID_PRIORITY	10
#define LOW_PRIORITY	11


/*
 *  ターゲットに依存する可能性のある定数の定義
 */

#ifndef STACK_SIZE
#define	STACK_SIZE		4096		/* タスクのスタックサイズ */
#endif /* STACK_SIZE */

#include "ev3api.h" 

extern int main_task(intptr_t unused); 
extern void highprio_task(intptr_t unused);
extern void midprio_task(intptr_t unused);
extern void lowprio_task(intptr_t unused);
