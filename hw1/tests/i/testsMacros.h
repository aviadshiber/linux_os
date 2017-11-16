#ifndef TESTSMACROS_H_
#define TESTSMACROS_H_

#include <stdio.h>

#define EPSILON 0.0001

#define TEST_EQUALS(result, a, b) if ((result) && ((a) != (b))) { \
								result = false; \
							}

#define TEST_EQUALS_FLOAT(result, a, b) do { \
				if ((result) && ((a) != (b))) { \
					float diff = a - b; \
					result = (diff < EPSILON) && (-diff < EPSILON);\
				} \
			} while (0)



#define TEST_DIFFERENT(result, a, b) if ((result) && ((a) == (b))) { \
								result = false; \
							}

#define TEST_TRUE(result, bool) if ((result) && !(bool)) { \
								result = false; \
							}

#define TEST_FALSE(result, bool) if ((result) && (bool)) { \
								result = false; \
							}

#define RUN_TEST(name)  printf("\tRunning "); \
						printf(#name);		\
						printf("... ");		\
						if (!name()) { \
							printf("[FAILED]\n");		\
							return -1; \
						}								\
						printf("[SUCCESS]\n");

//0 is success return value
#define RUN_TEST_FILE(name)  	printf("Testing file: "); \
								printf(#name);		\
								printf(".c\n");		\
								if (name() != 0) { \
									printf("[FAILED]\n\n");		\
									return -1; \
								}								\
								printf("[SUCCESS]\n\n");

#endif /* TESTSMACROS_H_ */
