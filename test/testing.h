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

char *readDataFromFile(char *filepath, size_t *size)
{
    // Open file in read binary mode
    FILE *filePTR = fopen(filepath, "rb");

    // If there was an error opening the file return NULL
    if (filePTR == NULL)
    {
        printf("Unable to open file: %s\n", filepath);
        return NULL;
    }

    // Find length of file
    fseek(filePTR, 0, SEEK_END);
    long int length = ftell(filePTR);

    // Check the file size pointer is provided before attempting to set it
    if (size != NULL)
    {
        // Set the given size pointer to the length
        *size = length;
    }

    // Go back to the start of the file
    fseek(filePTR, 0, SEEK_SET);

    // Create buffer with the required length including a null terminator
    char *buffer = malloc(sizeof(char) * (length + 1));

    if (buffer == NULL)
    {
        printf("Error creating buffer\n");
        return NULL;
    }

    // Read entire file into buffer
    fread(buffer, 1, length, filePTR);

    // Close the file
    fclose(filePTR);

    // Null terminator
    buffer[length] = '\0';

    return buffer;
}

#endif // TESTING_H