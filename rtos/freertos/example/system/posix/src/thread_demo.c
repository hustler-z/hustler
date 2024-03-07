/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: thread_demo.c
 * Date: 2023-10-12 10:41:45
 * LastEditTime: 2023-10-12 10:41:45
 * Description:  This file is for freertos posix thread function test
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2023/10/12 first commit
 */


#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
// #include <pthread.h>
#include "FreeRTOS_POSIX/pthread.h"


/*要执行的线程*/
void *test_thread(void *arg)
{
    int num = (unsigned long long)arg; /** sizeof(void*) == 8 and sizeof(int) == 4 (64 bits) */

    printf("This is test thread, arg is %d\n", num);
    sleep(5);
    /*退出线程*/
    pthread_exit(NULL);
	return NULL;
}


void CreateThreadDemoTasks(void)
{
    pthread_t thread;
    void *thread_return;
    int arg = 520;
    int res;

    printf("start create thread\n");

    /*创建线程，线程为test_thread函数*/
    res = pthread_create(&thread, NULL, test_thread, (void*)(arg));
    if(res != 0)
    {
        printf("create thread fail\n");
        exit(res);
    }

    printf("create treads success\n");
    printf("waiting for threads to finish...\n");

    /*等待线程终止*/
    res = pthread_join(thread, &thread_return);
    if(res != 0)
    {
        printf("thread exit fail\n");
        exit(res);
    }

    printf("thread exit ok\n");

}

