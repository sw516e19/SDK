#pragma once

#define MAIN_PRIORITY 5 /* メインタスクの優先度 */
												/* HIGH_PRIORITYより高くすること */

#define HIGH_PRIORITY 9 /* 並行実行されるタスクの優先度 */
#define MID_PRIORITY 10
#define LOW_PRIORITY 11
//#define PIXYQUEUELOC
#define PIXYQUEUESIZE 3

/*
 *  ターゲットに依存する可能性のある定数の定義
 */

#ifndef STACK_SIZE
#define STACK_SIZE 4096 /* タスクのスタックサイズ */
#endif									/* STACK_SIZE */

#include "ev3api.h"

extern void main_task(intptr_t unused);

typedef struct shooterdata ShooterData_t;
typedef struct rotatedata rot_data_t;

void detectobj(sensor_port_t pixycamPort, uint8_t signature, ShooterData_t *data);
void calculateIntersection(ShooterData_t *data, int16_t triggerDuration, uint16_t yTargetLocation, SYSTIM *dateTime);
void shootobj(motor_port_t motorPort, SYSTIM *fireTime, int rotation, int8_t speed);
void cleanData(ShooterData_t *data, SYSTIM *shootTime);
void rotateMotor(intptr_t datapointer);
void printFireTime();
void setFireTime(intptr_t datapointer);
void printshot(ShooterData_t *data, SYSTIM *shootTime);
void main_shooter();

