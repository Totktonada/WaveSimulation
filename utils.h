#ifndef UTILS_H_SENTRY
#define UTILS_H_SENTRY

#define UNUSED(x) (void)(x)

#define STR_EQUAL(str1, str2) \
    (strcmp((str1), (str2)) == 0)

/* Returns:
 * null-terminated string, if success;
 * NULL, otherwise. */
char * getTextFileContent(const char * path, long * lengthP);

struct timeval;

float timeval_diff_replace(struct timeval * tv1);

#endif /* UTILS_H_SENTRY */
