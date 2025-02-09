/*
 * Copyright (C) <year> <author>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/* clib includes */

#include "embUnit.h"

#include "i_list.h"

#include "tests-i_list.h"

#include <stdio.h>

#include "string.h"

#include <math.h>

#include <stdbool.h>


#define ELEMENT_NAME_MAX_LEN 30
#define MAX_NUMBER_ELEMENTS  50

typedef struct tests_i_list{
    char name[ELEMENT_NAME_MAX_LEN];
    
    i_list_node node_management;
} Element;

i_list_node *head;
Element list_of_elements[MAX_NUMBER_ELEMENTS];


static void test_add_list_entry_at_the_beginning(void) {
    
    memset(list_of_elements, 0, sizeof(list_of_elements));

    Element first_element = { .name = "first_element", .node_management.location_nr = 1 };

    list_of_elements[first_element.node_management.location_nr - 1] = first_element;

    i_list_node *node_ptr = &list_of_elements[first_element.node_management.location_nr - 1].node_management; 

    /* Add the first element in the list */
    add_list_entry_at_the_beginning(&head, &node_ptr);


    bool condition = (head == node_ptr) && (node_ptr->next == NULL) && (node_ptr->previous == NULL);


    TEST_ASSERT(condition);
}

static void test_append_list_entry(void) {
    /* ... */
    memset(list_of_elements, 0, sizeof(list_of_elements));

    Element first_element = { .name = "first_element", .node_management.location_nr = 1 };

    list_of_elements[first_element.node_management.location_nr - 1] = first_element;

    i_list_node *node_ptr_first = &list_of_elements[first_element.node_management.location_nr - 1].node_management; 

    /* Add the first element in the list */
    add_list_entry_at_the_beginning(&head, &node_ptr_first);


    Element second_element = { .name = "second_element", .node_management.location_nr = 2 };

    list_of_elements[second_element.node_management.location_nr - 1] = second_element;

    i_list_node *node_ptr_second = &list_of_elements[second_element.node_management.location_nr - 1].node_management;

    /* Append the second element in the list */
    append_list_entry(&head, &node_ptr_second, &node_ptr_first);


    bool condition = (head == node_ptr_first) && (node_ptr_first->previous == NULL) &&
                     (node_ptr_first->next == node_ptr_second) && (node_ptr_second->previous == node_ptr_first) &&
                     (node_ptr_second->next == NULL);

    TEST_ASSERT(condition);
}


static void test_add_list_entry_in_the_middle(void) {


    memset(list_of_elements, 0, sizeof(list_of_elements));

    Element first_element = { .name = "first_element", .node_management.location_nr = 1 };

    list_of_elements[first_element.node_management.location_nr - 1] = first_element;

    i_list_node *node_ptr_first = &list_of_elements[first_element.node_management.location_nr - 1].node_management; 

    /* Add the first element in the list */
    add_list_entry_at_the_beginning(&head, &node_ptr_first);


    Element fourth_element = { .name = "fourth_element", .node_management.location_nr = 4 };

    list_of_elements[fourth_element.node_management.location_nr - 1] = fourth_element;

    i_list_node *node_ptr_fourth = &list_of_elements[fourth_element.node_management.location_nr - 1].node_management;

    /* Append the fourth element in the list */
    append_list_entry(&head, &node_ptr_fourth, &node_ptr_first);


    Element middle_element = { .name = "middle_element", .node_management.location_nr = 2 };

    list_of_elements[middle_element.node_management.location_nr - 1] = middle_element;

    i_list_node *node_ptr_middle = &list_of_elements[middle_element.node_management.location_nr - 1].node_management;

    /* Insert the element in the middle between the first and fourth */
    add_list_entry_in_the_middle(&node_ptr_middle, &node_ptr_first);


    bool condition = (head == node_ptr_first) && (node_ptr_first->previous == NULL) && (node_ptr_first->next == node_ptr_middle)
                && (node_ptr_middle->previous == node_ptr_first) && (node_ptr_middle->next == node_ptr_fourth) &&
                (node_ptr_fourth->previous == node_ptr_middle) && (node_ptr_fourth->next == NULL);


    TEST_ASSERT(condition);
}

static void test_remove_list_entry_at_the_beginning(void) {
   
    memset(list_of_elements, 0, sizeof(list_of_elements));

    Element first_element = { .name = "first_element", .node_management.location_nr = 1 };

    list_of_elements[first_element.node_management.location_nr - 1] = first_element;

    i_list_node *node_ptr_first = &list_of_elements[first_element.node_management.location_nr - 1].node_management; 

    /* Add the first element in the list */
    add_list_entry_at_the_beginning(&head, &node_ptr_first);


    Element fourth_element = { .name = "fourth_element", .node_management.location_nr = 4 };

    list_of_elements[fourth_element.node_management.location_nr - 1] = fourth_element;

    i_list_node *node_ptr_fourth = &list_of_elements[fourth_element.node_management.location_nr - 1].node_management;

    /* Append the fourth element in the list */
    append_list_entry(&head, &node_ptr_fourth, &node_ptr_first);


    Element middle_element = { .name = "middle_element", .node_management.location_nr = 2 };

    list_of_elements[middle_element.node_management.location_nr - 1] = middle_element;

    i_list_node *node_ptr_middle = &list_of_elements[middle_element.node_management.location_nr - 1].node_management;

    /* Insert the element in the middle between the first and fourth */
    add_list_entry_in_the_middle(&node_ptr_middle, &node_ptr_first);


    remove_list_entry(&head, &node_ptr_first);

    bool condition = (head == node_ptr_middle) && (node_ptr_middle->previous == NULL) && (node_ptr_middle->next == node_ptr_fourth) &&
                        (node_ptr_fourth->previous == node_ptr_middle) && (node_ptr_fourth->next == NULL) == (node_ptr_first->previous == NULL)
                    && (node_ptr_first->next == NULL);

    TEST_ASSERT(condition);
}

static void test_remove_list_entry_in_the_middle(void) {
    
    memset(list_of_elements, 0, sizeof(list_of_elements));

    Element first_element = { .name = "first_element", .node_management.location_nr = 1 };

    list_of_elements[first_element.node_management.location_nr - 1] = first_element;

    i_list_node *node_ptr_first = &list_of_elements[first_element.node_management.location_nr - 1].node_management; 

    /* Add the first element in the list */
    add_list_entry_at_the_beginning(&head, &node_ptr_first);


    Element fourth_element = { .name = "fourth_element", .node_management.location_nr = 4 };

    list_of_elements[fourth_element.node_management.location_nr - 1] = fourth_element;

    i_list_node *node_ptr_fourth = &list_of_elements[fourth_element.node_management.location_nr - 1].node_management;

    /* Append the fourth element in the list */
    append_list_entry(&head, &node_ptr_fourth, &node_ptr_first);


    Element middle_element = { .name = "middle_element", .node_management.location_nr = 2 };

    list_of_elements[middle_element.node_management.location_nr - 1] = middle_element;

    i_list_node *node_ptr_middle = &list_of_elements[middle_element.node_management.location_nr - 1].node_management;

    /* Insert the element in the middle between the first and fourth */
    add_list_entry_in_the_middle(&node_ptr_middle, &node_ptr_first);


    remove_list_entry(&head, &node_ptr_middle);

    bool condition = (head == node_ptr_first) && (node_ptr_middle->previous == NULL) && (node_ptr_middle->next == NULL) &&
                        (node_ptr_fourth->previous == node_ptr_first) && (node_ptr_fourth->next == NULL) == (node_ptr_first->previous == NULL)
                    && (node_ptr_first->next == node_ptr_fourth);

    TEST_ASSERT(condition);
}

static void test_remove_list_entry_in_the_end(void) {
    
    memset(list_of_elements, 0, sizeof(list_of_elements));

    Element first_element = { .name = "first_element", .node_management.location_nr = 1 };

    list_of_elements[first_element.node_management.location_nr - 1] = first_element;

    i_list_node *node_ptr_first = &list_of_elements[first_element.node_management.location_nr - 1].node_management; 

    /* Add the first element in the list */
    add_list_entry_at_the_beginning(&head, &node_ptr_first);


    Element fourth_element = { .name = "fourth_element", .node_management.location_nr = 4 };

    list_of_elements[fourth_element.node_management.location_nr - 1] = fourth_element;

    i_list_node *node_ptr_fourth = &list_of_elements[fourth_element.node_management.location_nr - 1].node_management;

    /* Append the fourth element in the list */
    append_list_entry(&head, &node_ptr_fourth, &node_ptr_first);


    Element middle_element = { .name = "middle_element", .node_management.location_nr = 2 };

    list_of_elements[middle_element.node_management.location_nr - 1] = middle_element;

    i_list_node *node_ptr_middle = &list_of_elements[middle_element.node_management.location_nr - 1].node_management;

    /* Insert the element in the middle between the first and fourth */
    add_list_entry_in_the_middle(&node_ptr_middle, &node_ptr_first);


    remove_list_entry(&head, &node_ptr_fourth);

    bool condition = (head == node_ptr_first) && (node_ptr_middle->previous == node_ptr_first) && (node_ptr_middle->next == NULL) &&
                        (node_ptr_fourth->previous == NULL) && (node_ptr_fourth->next == NULL) == (node_ptr_first->previous == NULL)
                    && (node_ptr_first->next == node_ptr_middle);

    TEST_ASSERT(condition);
}

Test *tests_i_list_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_add_list_entry_at_the_beginning),
        new_TestFixture(test_append_list_entry),
        new_TestFixture(test_add_list_entry_in_the_middle),
        new_TestFixture(test_remove_list_entry_at_the_beginning),
        new_TestFixture(test_remove_list_entry_in_the_middle),
        new_TestFixture(test_remove_list_entry_in_the_end),
        /* ... */
    };

    EMB_UNIT_TESTCALLER(i_list_tests, NULL, NULL, fixtures);

    return (Test *)&i_list_tests;
}

int main(void) {
    TestRunner_start();
    TestRunner_runTest(tests_i_list_tests());
    TestRunner_end();
    return 0;
}