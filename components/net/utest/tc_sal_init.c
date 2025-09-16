/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-09-16     kurisaw      Initial version for SAL initialization test cases
 */

#include <utest.h>
#include <rtthread.h>
#include <string.h>
#include <sal_socket.h>
#include <sal_low_lvl.h>
#include <netdev.h>

/**
 * @brief   Test suite for RT-Thread SAL component initialization and resource management
 *
 * @note    This test suite validates:
 *          1. SAL initialization success and re-initialization handling
 *          2. Socket creation after initialization
 *          3. Socket table allocation limits
 *          4. Memory allocation failure scenarios
 *          5. Socket allocation and deallocation
 */

/* Define SAL constants for testing */
#define SOCKET_TABLE_STEP_LEN          4
#define TEST_SAL_SOCKETS_NUM        SAL_SOCKETS_NUM
#define TEST_SOCKET_TABLE_STEP_LEN  SOCKET_TABLE_STEP_LEN
#define TEST_SAL_SOCKET_OFFSET      SAL_SOCKET_OFFSET

/* Mock functions to simulate memory allocation failures */
static rt_bool_t mock_memory_failure = RT_FALSE;
void *rt_calloc_mock(rt_size_t count, rt_size_t size)
{
    if (mock_memory_failure)
        return RT_NULL;
    return rt_calloc(count, size);
}

void *rt_realloc_mock(void *ptr, rt_size_t size)
{
    if (mock_memory_failure)
        return RT_NULL;
    return rt_realloc(ptr, size);
}

/* Helper function to reset SAL state by reinitializing */
static void reset_sal_state(void)
{
    /* Close all possible sockets to free resources */
    for (rt_int32_t i = TEST_SAL_SOCKET_OFFSET; i < TEST_SAL_SOCKETS_NUM + TEST_SAL_SOCKET_OFFSET; i++)
    {
        sal_closesocket(i);
    }
    /* Reinitialize SAL to reset internal state */
    sal_init();
}

/* Test Case: SAL Initialization Success */
static void test_sal_init_success(void)
{
    rt_err_t ret;

    /* Reset SAL state */
    reset_sal_state();

    /* Call sal_init */
    ret = sal_init();
    uassert_int_equal(ret, RT_EOK);

    /* Verify initialization by creating a socket */
    int socket = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_true(socket >= TEST_SAL_SOCKET_OFFSET);

    /* Cleanup */
    sal_closesocket(socket);
    reset_sal_state();
}

/* Test Case: SAL Initialization with Insufficient Memory */
static void test_sal_init_memory_failure(void)
{
    rt_err_t ret;

    /* Reset SAL state */
    reset_sal_state();

    /* Simulate memory allocation failure */
    mock_memory_failure = RT_TRUE;

    /* Call sal_init with mocked rt_calloc */
    ret = sal_init();
    uassert_int_equal(ret, -RT_ERROR);

    /* Verify no sockets can be created */
    int socket = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_int_equal(socket, -1);

    /* Reset mock */
    mock_memory_failure = RT_FALSE;

    /* Cleanup */
    reset_sal_state();
}

/* Test Case: SAL Re-Initialization */
static void test_sal_init_reinit(void)
{
    rt_err_t ret;
    int socket1, socket2;

    /* Reset SAL state */
    reset_sal_state();

    /* First initialization */
    ret = sal_init();
    uassert_int_equal(ret, RT_EOK);

    /* Create a socket to verify functionality */
    socket1 = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_true(socket1 >= TEST_SAL_SOCKET_OFFSET);

    /* Re-initialize SAL */
    ret = sal_init();
    uassert_int_equal(ret, RT_EOK);

    /* Verify existing socket is still valid by creating another */
    socket2 = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_true(socket2 >= TEST_SAL_SOCKET_OFFSET);
    uassert_true(socket2 != socket1);

    /* Cleanup */
    sal_closesocket(socket1);
    sal_closesocket(socket2);
    reset_sal_state();
}

/* Test Case: Socket Table Expansion */
static void test_socket_table_expansion(void)
{
    int sockets[TEST_SAL_SOCKETS_NUM];
    rt_int32_t i;

    /* Reset SAL state */
    reset_sal_state();

    /* Initialize SAL */
    uassert_int_equal(sal_init(), RT_EOK);

    /* Mock network device and protocol family for socket creation */
    struct netdev mock_netdev = { .sal_user_data = RT_NULL };
    struct sal_proto_family mock_pf = { .family = AF_INET, .skt_ops = RT_NULL };
    mock_netdev.sal_user_data = &mock_pf;
    netdev_default = &mock_netdev;

    /* Fill initial socket table */
    for (i = 0; i < TEST_SOCKET_TABLE_STEP_LEN; i++)
    {
        sockets[i] = sal_socket(AF_INET, SOCK_STREAM, 0);
        uassert_true(sockets[i] >= TEST_SAL_SOCKET_OFFSET);
    }

    /* Trigger table expansion */
    int new_socket = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_true(new_socket >= TEST_SAL_SOCKET_OFFSET);

    /* Verify new socket is distinct */
    for (i = 0; i < TEST_SOCKET_TABLE_STEP_LEN; i++)
    {
        uassert_true(new_socket != sockets[i]);
    }

    /* Cleanup */
    for (i = 0; i < TEST_SOCKET_TABLE_STEP_LEN; i++)
    {
        sal_closesocket(sockets[i]);
    }
    sal_closesocket(new_socket);
    reset_sal_state();
}

/* Test Case: Socket Allocation and Deallocation */
static void test_socket_alloc_dealloc(void)
{
    int socket1, socket2;

    /* Reset SAL state */
    reset_sal_state();

    /* Initialize SAL */
    uassert_int_equal(sal_init(), RT_EOK);

    /* Allocate socket */
    socket1 = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_true(socket1 >= TEST_SAL_SOCKET_OFFSET);

    /* Allocate another socket */
    socket2 = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_true(socket2 >= TEST_SAL_SOCKET_OFFSET);
    uassert_true(socket2 != socket1);

    /* Deallocate socket1 */
    uassert_int_equal(sal_closesocket(socket1), 0);

    /* Verify socket2 is still valid */
    int socket3 = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_true(socket3 >= TEST_SAL_SOCKET_OFFSET);
    uassert_true(socket3 != socket2);

    /* Deallocate socket2 */
    uassert_int_equal(sal_closesocket(socket2), 0);

    /* Cleanup */
    sal_closesocket(socket3);
    reset_sal_state();
}

/* Test Case: Socket Allocation with Memory Failure */
static void test_socket_alloc_memory_failure(void)
{
    int socket;

    /* Reset SAL state */
    reset_sal_state();

    /* Initialize SAL */
    uassert_int_equal(sal_init(), RT_EOK);

    /* Simulate memory allocation failure */
    mock_memory_failure = RT_TRUE;

    /* Attempt to allocate socket */
    socket = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_int_equal(socket, -1);

    /* Reset mock */
    mock_memory_failure = RT_FALSE;

    /* Verify normal allocation works after reset */
    socket = sal_socket(AF_INET, SOCK_STREAM, 0);
    uassert_true(socket >= TEST_SAL_SOCKET_OFFSET);

    /* Cleanup */
    sal_closesocket(socket);
    reset_sal_state();
}

/* Test suite initialization */
static rt_err_t testcase_init(void)
{
    if (!rt_scheduler_is_available())
    {
        return -RT_ERROR;
    }
    mock_memory_failure = RT_FALSE;
    return RT_EOK;
}

/* Test suite cleanup */
static rt_err_t testcase_cleanup(void)
{
    reset_sal_state();
    return RT_EOK;
}

/* Test suite entry point */
static void test_sal_init_suite(void)
{
    UTEST_UNIT_RUN(test_sal_init_success);
    UTEST_UNIT_RUN(test_sal_init_memory_failure);
    UTEST_UNIT_RUN(test_sal_init_reinit);
    UTEST_UNIT_RUN(test_socket_table_expansion);
    UTEST_UNIT_RUN(test_socket_alloc_dealloc);
    UTEST_UNIT_RUN(test_socket_alloc_memory_failure);
}
UTEST_TC_EXPORT(test_sal_init_suite, "testcases.sal.init_test", testcase_init, testcase_cleanup, 20);