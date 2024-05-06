/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/******************************************************************************
 * This project provides two demo applications.  A simple blinky style project,
 * and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the simply blinky version.
 *
 * This file only contains the source code that is specific to the basic demo.
 * Generic functions, such FreeRTOS hook functions, are defined in main.c.
 ******************************************************************************
 *
 * main_blinky() creates one queue, one software timer, and two tasks.  It then
 * starts the scheduler.
 *
 * The Queue Send Task:
 * The queue send task is implemented by the prvQueueSendTask() function in
 * this file.  It uses vTaskDelayUntil() to create a periodic task that sends
 * the value 100 to the queue every 200 (simulated) milliseconds.
 *
 * The Queue Send Software Timer:
 * The timer is an auto-reload timer with a period of two (simulated) seconds.
 * Its callback function writes the value 200 to the queue.  The callback
 * function is implemented by prvQueueSendTimerCallback() within this file.
 *
 * The Queue Receive Task:
 * The queue receive task is implemented by the prvQueueReceiveTask() function
 * in this file.  prvQueueReceiveTask() waits for data to arrive on the queue.
 * When data is received, the task checks the value of the data, then outputs a
 * message to indicate if the data came from the queue send task or the queue
 * send software timer.
 */

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY    ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK_PRIORITY       ( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The times are converted from
 * milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS         pdMS_TO_TICKS( 200UL )
#define mainTIMER_SEND_FREQUENCY_MS        pdMS_TO_TICKS( 2000UL )

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH                   ( 2 )

/* The values sent to the queue receive task from the queue send task and the
 * queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK           ( 100UL )
#define mainVALUE_SENT_FROM_TIMER          ( 200UL )

QueueHandle_t xQueue;

/*-----------------------------------------------------------*/

static void vSenderTask( void * pvParameters )
{
    unsigned int lValueToSend;
    BaseType_t xStatus;

    lValueToSend = ( int32_t ) pvParameters;

    for( ;; )
    {
        xStatus = xQueueSendToBack( xQueue, &lValueToSend, 0 );

        if( xStatus != pdPASS )
        {
            printf( "ERROR: Couldn't write to the queue...\r\n" );
        }
    }
}

static void vReceiverTask( void * pvParameters )
{
    unsigned int lReceivedValue;
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS( 100 );

    for( ;; )
    {
        /* this task will empty the the queue of any data
           that is writtent to the queue */
        if( uxQueueMessagesWaiting( xQueue ) != 0 )
        {
            printf( "Queue should have been empty!\r\n" );
        }

        xStatus = xQueueReceive( xQueue, &lReceivedValue, xTicksToWait );

        if( xStatus == pdPASS)
        {
            printf( "Received = %x\r\n", lReceivedValue );
        }
        else
        {
            printf( "ERROR: Could not receive from the queue\r\n" );
        }
    }
}

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
void main_learn( void )
{
    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details.  NOTE: This demo uses static allocation
     * for the idle and timer tasks so this line should never execute. */
    
    xQueue = xQueueCreate( 5, sizeof( int32_t ) );

    if( xQueue != NULL )
    {
        xTaskCreate( vSenderTask, "Sender1", 1000, ( void * ) 100, mainQUEUE_SEND_TASK_PRIORITY, NULL );
        xTaskCreate( vSenderTask, "Sender2", 1000, ( void * ) 200, mainQUEUE_SEND_TASK_PRIORITY, NULL );

        xTaskCreate( vReceiverTask, "Receiver", 1000, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );

        vTaskStartScheduler();
    }
    else
    {
        printf("ERROR: Unable to create Queue Struct. Just going to stop and pass things to the Idle task\r\n");
        printf("ERROR: SO, just kill me here cuz im just running in QEMU\r\n");
    }

    for( ; ; );
}
