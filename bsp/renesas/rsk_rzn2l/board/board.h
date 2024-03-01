/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-10      Sherman      first version
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rtdef.h>
#include <cp15.h>
#include <hal_data.h>

#define RZ_SRAM_SIZE    1536 /* The SRAM size of the chip needs to be modified */
#define RZ_SRAM_END     (0x10000000 + RZ_SRAM_SIZE * 1024)

#ifdef __ARMCC_VERSION
extern int Image$$RAM_END$$ZI$$Base;
#define HEAP_BEGIN  ((void *)&Image$$RAM_END$$ZI$$Base)
#elif __ICCARM__
#pragma section="CSTACK"
#define HEAP_BEGIN      (__segment_end("CSTACK"))
#else
#define HEAP_BEGIN      (0x10000000)
#endif

#define HEAP_END        RZ_SRAM_END

/*              user defined                */
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define MAX_HANDLERS    (512)   // refer to bsp_mcu_family_cfg.h
#define GIC_IRQ_START   0                               // refer to cortex-r52 manual
#define GIC_ACK_INTID_MASK  (0x000003FFU)
/* number of interrupts on board */
#define ARM_GIC_NR_IRQS     (448)
//#define ARM_GIC_NR_IRQS     (VECTOR_DATA_IRQ_COUNT)
/* only one GIC available */
#define ARM_GIC_MAX_NR      1
/*                  end defined            */

#define GICV3_DISTRIBUTOR_BASE_ADDR             (0x100000)

/* the basic constants and interfaces needed by gic */
rt_inline rt_uint32_t platform_get_gic_dist_base(void)
{
    rt_uint32_t gic_base;

//    __asm volatile ("mrc p15, 4, %0, c15, c0, 0" : "=r"(gic_base));
    /* To access the IMP_CBAR */
    __get_cp(15, 1, gic_base, 15, 3, 0);
    return gic_base + GICV3_DISTRIBUTOR_BASE_ADDR;
//    return gic_base;
}

#ifdef __cplusplus
}
#endif

#endif
