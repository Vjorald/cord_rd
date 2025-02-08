/*
 * Copyright (C) <year> <author>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  unittests
 * @{
 *
 * @file
 * @brief       Unittests for the ``module`` module
 *
 * @author      <author>
 */
#ifndef TESTS_RD_H
#define TESTS_RD_H
#include "embUnit/embUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Generates tests for rd.h
 *
 * @return  embUnit tests if successful, NULL if not.
 */
Test *tests_rd_tests(void);

/* ... */

#ifdef __cplusplus
}
#endif

#endif /* TESTS_RD_H */
/** @} */