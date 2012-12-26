#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "utils.h"

/* File must be opened. */
long getFileLength(FILE * file)
{
    long length;
    long pos = ftell(file);

    fseek(file, 0L, SEEK_END);
    length = ftell(file);
    fseek(file, pos, SEEK_SET);

    return length;
}

/* Based on: see LINKS file: [4]. */
char * getTextFileContent(const char * path, long * lengthP)
{
    char * content = NULL;
    FILE * file;
 
    long length = 0;
 
    if (path == NULL)
    {
        return NULL;
    }

    file = fopen(path, "rt");
 
    if (file == NULL)
    {
        return NULL;
    }

    length = getFileLength(file);

    if (length <= 0)
    {
        fclose(file);
        return NULL;
    }

    content = (char *) malloc(sizeof(char) * (length + 1));
    length = fread(content, sizeof(char), length, file);
    content[length] = '\0';

    if (lengthP != NULL)
    {
        *lengthP = length;
    }

    return content;
}

float timeval_diff_replace(struct timeval * tv1)
{
    struct timeval tv2;
    float res;

    gettimeofday(&tv2, NULL);

    res = (tv2.tv_sec - tv1->tv_sec) +
        (tv2.tv_usec - tv1->tv_usec) / 1000000.0;

    tv1->tv_sec = tv2.tv_sec;
    tv1->tv_usec = tv2.tv_usec;

    return res;
}
