#ifndef BREADBOARD_RESULT_H
#define BREADBOARD_RESULT_H

/*
 * breadboard_result.h
 *
 * Result codes for the Breadboard structural compiler API.
 * Callers must check the result before interpreting out-parameters.
 */

typedef enum BreadboardResult
{
    /* Operation completed successfully. */
    BREADBOARD_OK = 0,

    /* A required argument was NULL or out of range. */
    BREADBOARD_ERR_INVALID_ARGUMENT = 1,

    /* A handle argument was NULL or does not refer to a live object. */
    BREADBOARD_ERR_INVALID_HANDLE = 2,

    /* The target backend specified is not known or not supported. */
    BREADBOARD_ERR_INVALID_TARGET = 3,

    /* Compilation failed due to structural validation or target legality failures. */
    BREADBOARD_ERR_COMPILE_FAILED = 4,

    /* The requested operation or output requirement is explicitly unsupported. */
    BREADBOARD_ERR_UNSUPPORTED = 5,

    /* An internal compiler error that should not occur under normal conditions. */
    BREADBOARD_ERR_INTERNAL = 6,

    /* The index provided for a list lookup was out of range. */
    BREADBOARD_ERR_OUT_OF_BOUNDS = 7,

    /* A requested descriptor ID or name was not found. */
    BREADBOARD_ERR_NOT_FOUND = 8

}
BreadboardResult;

#endif /* BREADBOARD_RESULT_H */
