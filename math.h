// Math code for the TI-99/4A
// Free to use

#ifndef MATH_H
#define MATH_H

#ifdef __cplusplus
extern "C" {
#endif

//#ifndef CLASSIC99

int sqrt(int x);

inline int abs(int x) {
    return x > 0 ? x : (-1 * x);
}
//#endif

#ifdef __cplusplus
}   // extern C
#endif

#endif /* MATH_H */
