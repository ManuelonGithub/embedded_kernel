
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

//

//typedef uint8_t bitmap8_t;
//#define BITMAP_WIDTH        8  /// Amount of bits in a bitmap field (bitmap_t)
//#define BITMAP_INDEX_MASK   3   /// log2(BITMAP_WIDTH)
//#define FIELD_BIT_MASK      BITMAP8_WIDTH-1

typedef uint32_t bitmap_t;

#define BITMAP_WIDTH        32  /// Amount of bits in a bitmap entry (bitmap_t)
#define BITMAP_INDEX_MASK   5   /// Mask to find the position of a bit in the bitmap array. log2(BITMAP_WIDTH).
#define BITMAP_BIT_MASK     BITMAP_WIDTH-1  /// Mask to find the position of a bit in a bitmap entry.

inline void SetBit(bitmap_t* bitmap, uint32_t bit);
inline void ClearBit(bitmap_t* bitmap, uint32_t bit);

inline void SetBitRange(bitmap_t* bitmap, uint32_t start, uint32_t end);
inline void ClearBitRange(bitmap_t* bitmap, uint32_t start, uint32_t end);

inline bool GetBit(bitmap_t* bitmap, uint32_t bit);

inline uint32_t FindSet(bitmap_t* bitmap, uint32_t start, uint32_t end);
inline uint32_t FindClear(bitmap_t* bitmap, uint32_t start, uint32_t end);


#endif	// BITMAP_H
