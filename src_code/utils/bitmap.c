
/**
 * @file    bitmap.c
 * @brief   Contains all functionality related to operating a bitmap.
 * @author  Manuel Burnay
 * @date    2019.11.22  (Created)
 * @date    2019.11.25  (Last Modified)
 */


#include "bitmap.h"

/**
 * @brief   Sets a specific bit in a bitmap.
 * @param   [out]   bitmap: bitmap to be modified.
 * @param   [in]    bit: Bit in bitmap to be set.
 * @details This function does not perform boundary checks,
 *          That is up to the caller to pre-check.
 */
inline void SetBit(bitmap_t* bitmap, uint32_t bit)
{
    bitmap[(bit >> BITMAP_INDEX_MASK)] |= (1 << (bit & BITMAP_BIT_MASK));
}

/**
 * @brief   Clears a specific bit in a bitmap.
 * @param   [out]   bitmap: bitmap to be modified.
 * @param   [in]    bit: Bit in bitmap to be cleared.
 * @details This function does not perform boundary checks,
 *          That is up to the caller to pre-check.
 */
inline void ClearBit(bitmap_t* bitmap, uint32_t bit)
{
    bitmap[(bit >> BITMAP_INDEX_MASK)] &= ~(1 << (bit & BITMAP_BIT_MASK));
}

/**
 * @brief   Sets a range of bits from start to end.
 * @param   [in,out] bitmap: bitmap array to be modified.
 * @param   [in] start: first bit position to be set.
 * @param   [in] end: Bit position to reach when setting bits.
 * @details Bits will be set up to end-1.
 */
inline void SetBitRange(bitmap_t* bitmap, uint32_t start, uint32_t end)
{
    // todo: look into faster algorithms for this
    // if a large amount of bits to be iterated through
    // this will be slow
    while(start < end) {
        SetBit(bitmap, start);
        start++;
    }
}

/**
 * @brief   Clears a range of bits from start to end.
 * @param   [in,out] bitmap: bitmap array to be modified.
 * @param   [in] start: first bit position to be cleared.
 * @param   [in] end: Bit position to reach when clearing bits.
 * @details Bits will be cleared up to end-1.
 */
inline void ClearBitRange(bitmap_t* bitmap, uint32_t start, uint32_t end)
{
    // todo: look into faster algorithms for this
    // if a large amount of bits to be iterated through
    // this will be slow
    while(start < end) {
        ClearBit(bitmap, start);
        start++;
    }
}

/**
 * @brief   Gets thevalue of a specific bit in a bitmap.
 * @param   [in]   bitmap: bitmap to be checked.
 * @param   [in]   bit: Bit in bitmap to be checked.
 * @return  True if bit is set,
 *          False if not.
 * @details This function does not perform boundary checks,
 *          That is up to the caller to pre-check.
 */
inline bool GetBit(bitmap_t* bitmap, uint32_t bit)
{
    return (bitmap[bit >> BITMAP_INDEX_MASK] & (1 << (bit & BITMAP_BIT_MASK)));
}

/**
 * @brief   Finds The earliest set bit in bitmap.
 * @param   [in] bitmap: bitmap to be checked.
 * @param   [in] start: Starting bit position to check the bitmap.
 * @param   [in] end: End bit position of search.
 * @return  'end' if no bit in range was set,
 *          index of set bit otherwise.
 * @details Search will only go up to end-1.
 */
inline uint32_t FindSet(bitmap_t* bitmap, uint32_t start, uint32_t end)
{
    //todo: implement better algorithm for this
    // when a large range is search this'll be slow
    while (start < end) {
        if (GetBit(bitmap, start))
            return start;
        else
            start++;
    }

    return end;
}

/**
 * @brief   Finds The earliest cleared bit in bitmap.
 * @param   [in] bitmap: bitmap to be searched.
 * @param   [in] start: Starting bit-index to check the bitmap.
 * @param   [in] end: End bit position of search.
 * @return  'end' if no bit in range was cleared,
 *          index of cleared bit otherwise.
 * @details Search will only go up to end-1.
 */
inline uint32_t FindClear(bitmap_t* bitmap, uint32_t start, uint32_t end)
{
    while (start < end) {
        if (!GetBit(bitmap, start))
            return start;
        else
            start++;
    }

    return end;
}
