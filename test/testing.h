#ifndef TESTING_H
#define TESTING_H

#include <stdio.h>
#include <stdlib.h>

/** @brief Assert that a condition is true */
#define ASSERT_TRUE(condition, message)                                         \
    do                                                                          \
    {                                                                           \
        if (!(condition))                                                       \
        {                                                                       \
            fprintf(stderr, "[FAIL] %s:%d: %s\n", __FILE__, __LINE__, message); \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

/** @brief Assert that two values are equal */
#define ASSERT_EQUALS(expected, actual, message)                                                                                      \
    do                                                                                                                                \
    {                                                                                                                                 \
        if ((expected) != (actual))                                                                                                   \
        {                                                                                                                             \
            fprintf(stderr, "[FAIL] %s:%d: %s (Expected %d, got %d)\n", __FILE__, __LINE__, message, (int)(expected), (int)(actual)); \
            exit(1);                                                                                                                  \
        }                                                                                                                             \
    } while (0)

char *readDataFromFile(char *filepath, size_t *size);

#endif // TESTING_H