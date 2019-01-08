#ifndef UTIL_ARM_H
#define UTIL_ARM_H

#if defined __arm__ || defined __ARM__ || defined ARM || defined __ARM || defined __arm || defined __ARM_ARCH ||defined __aarch64__ ||( defined(__sun) && defined(__SVR4))
#  define ARM_COMPLIANT 1
#else
#  define ARM_COMPLIANT 0
#endif

#endif