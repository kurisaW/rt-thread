/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2021-10-10     Sherman       first version
 */

#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>

#define LED_PIN    BSP_IO_PORT_20_PIN_1 /* Onboard LED pins */

int thread_sample(void);


void hal_entry(void)
{
    rt_kprintf("Hello RT-Thread!\n");
    rt_kprintf("Hello Renesas!\n");

    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
}

void th1_entry(void)
{
    rt_err_t sample = RT_FALSE;
    RT_ASSERT(sample);
}

void trigger_hardfault(void)
{
    // 通过访问一个非法地址来触发 HardFault
    volatile int *ptr = (volatile int *)0xFFFFFFFF;
    int value = *ptr;
}

void thread_test(void)
{
    rt_thread_t thread;

    // 创建一个线程，该线程在启动后会触发 HardFault
    thread = rt_thread_create("hardfault_thread", trigger_hardfault, RT_NULL, 1024, 10, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }

    // 启动 RT-Thread
    rt_thread_startup(rt_thread_self());
}
MSH_CMD_EXPORT(thread_test,thread_test);


void func(void)
{
    int *p;
    p = __builtin_frame_address(0);
    rt_kprintf("func frame:%p\n",p);
    p = __builtin_frame_address(1);
    rt_kprintf("main frame:%p\n",p);
}
void fp_test(void)
{
    int *p;
    p = __builtin_frame_address(0);
    rt_kprintf("main frame:%p\n",p);
    rt_kprintf("\n");
    func();
}
MSH_CMD_EXPORT(fp_test,fp_test);

