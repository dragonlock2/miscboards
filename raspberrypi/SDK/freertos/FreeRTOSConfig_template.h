#pragma once

#include <stdio.h>

#define configUSE_PREEMPTION                      (1)
#define configUSE_PORT_OPTIMISED_TASK_SELECTION   (0)
#define configUSE_TICKLESS_IDLE                   (0)
#define configTICK_RATE_HZ                        (1000)
#define configMAX_PRIORITIES                      (8)
#define configMINIMAL_STACK_SIZE                  (256) // ARM=128, RISC-V=256
#define configMAX_TASK_NAME_LEN                   (16)
#define configTICK_TYPE_WIDTH_IN_BITS             (TICK_TYPE_WIDTH_32_BITS)
#define configIDLE_SHOULD_YIELD                   (1)
#define configUSE_TASK_NOTIFICATIONS              (1)
#define configTASK_NOTIFICATION_ARRAY_ENTRIES     (3)
#define configUSE_MUTEXES                         (1)
#define configUSE_RECURSIVE_MUTEXES               (0)
#define configUSE_COUNTING_SEMAPHORES             (1)
#define configQUEUE_REGISTRY_SIZE                 (8)
#define configUSE_QUEUE_SETS                      (0)
#define configUSE_TIME_SLICING                    (1)
#define configUSE_NEWLIB_REENTRANT                (0)
#define configENABLE_BACKWARD_COMPATIBILITY       (0)
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS   (4)
#define configUSE_MINI_LIST_ITEM                  (1)
#define configSTACK_DEPTH_TYPE                    uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE          size_t
#define configHEAP_CLEAR_MEMORY_ON_FREE           (0)

#define configSUPPORT_STATIC_ALLOCATION           (1)
#define configSUPPORT_DYNAMIC_ALLOCATION          (1)
#define configKERNEL_PROVIDED_STATIC_MEMORY       (1)
#define configTOTAL_HEAP_SIZE                     (32 * 1024)
#define configAPPLICATION_ALLOCATED_HEAP          (0)
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP (0)

#define configUSE_IDLE_HOOK                       (0)
#define configUSE_TICK_HOOK                       (0)
#define configCHECK_FOR_STACK_OVERFLOW            (0)
#define configUSE_MALLOC_FAILED_HOOK              (0)
#define configUSE_DAEMON_TASK_STARTUP_HOOK        (0)
#define configUSE_SB_COMPLETED_CALLBACK           (0)

#define configGENERATE_RUN_TIME_STATS             (0)
#define configUSE_TRACE_FACILITY                  (1)
#define configUSE_STATS_FORMATTING_FUNCTIONS      (0)

#define configUSE_CO_ROUTINES                     (0)
#define configMAX_CO_ROUTINE_PRIORITIES           (1)

#define configUSE_TIMERS                          (1)
#define configTIMER_TASK_PRIORITY                 (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                  (8)
#define configTIMER_TASK_STACK_DEPTH              (1024)

#define configUSE_EVENT_GROUPS                    (1)
#define configUSE_STREAM_BUFFERS                  (1)

#define configASSERT(x) \
    if ((x) == 0) {                                       \
        printf("error at %s:%d\r\n", __FILE__, __LINE__); \
        while (1);                                        \
    }

#define INCLUDE_vTaskPrioritySet             (1)
#define INCLUDE_uxTaskPriorityGet            (1)
#define INCLUDE_vTaskDelete                  (1)
#define INCLUDE_vTaskSuspend                 (1)
#define INCLUDE_vTaskDelayUntil              (1)
#define INCLUDE_vTaskDelay                   (1)
#define INCLUDE_xTaskGetSchedulerState       (1)
#define INCLUDE_xTaskGetCurrentTaskHandle    (1)
#define INCLUDE_uxTaskGetStackHighWaterMark  (0)
#define INCLUDE_uxTaskGetStackHighWaterMark2 (0)
#define INCLUDE_xTaskGetIdleTaskHandle       (0)
#define INCLUDE_eTaskGetState                (0)
#define INCLUDE_xTimerPendFunctionCall       (1)
#define INCLUDE_xTaskAbortDelay              (0)
#define INCLUDE_xTaskGetHandle               (0)
#define INCLUDE_xTaskResumeFromISR           (1)

// SMP-specific
#define configNUMBER_OF_CORES                (2)
#define configRUN_MULTIPLE_PRIORITIES        (1)
#define configUSE_CORE_AFFINITY              (1)
#define configUSE_TASK_PREEMPTION_DISABLE    (0)
#define configUSE_PASSIVE_IDLE_HOOK          (0)

// RP2350-specific
#if PICO_RP2350
#define configENABLE_MPU                     (0)
#define configENABLE_TRUSTZONE               (0)
#define configRUN_FREERTOS_SECURE_ONLY       (1)
#define configENABLE_FPU                     (1)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (16)
#endif

#include <rp2040_config.h>
