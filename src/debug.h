/*
 * debug.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */

#ifndef DEBUG_V4_H_
#define DEBUG_V4_H_

#ifdef DEBUG_ENABLED
#define DEBUG_INIT() Serial.begin(115200)
#define DEBUG(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG_INIT()
#define DEBUG(...)
#endif

#endif // DEBUG_V4_H_
