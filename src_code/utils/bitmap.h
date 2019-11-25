
/**
 * @file    bitmap.h
 * @brief   Contains all definitions and function prototypes related to
 *          operating a bitmap.
 * @author  Manuel Burnay
 * @date    2019.11.22  (Created)
 * @date    2019.11.25  (Last Modified)
 */

#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t* bitmap_t;

inline void SetBit(bitmap_t bitmap, uint8_t bit);
inline void ClearBit(bitmap_t bitmap, uint8_t bit);
inline bool GetBit(bitmap_t bitmap, uint8_t bit);

inline uint32_t FindClear(bitmap_t bitmap, uint32_t min, uint32_t max);
inline uint32_t FindSet(bitmap_t bitmap, uint32_t min, uint32_t max);

#endif	// BITMAP_H
