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
 * @brief   Generates tests for <header1>.h
 *
 * @return  embUnit tests if successful, NULL if not.
 */
Test *tests_<module>_<header1>_tests(void);

/**
 * @brief   Generates tests for <header2>.h
 *
 * @return  embUnit tests if successful, NULL if not.
 */
Test *tests_<module>_<header2>_tests(void);

/* ... */

#ifdef __cplusplus
}
#endif

#endif /* TESTS_<MODULE>_H */
/** @} */