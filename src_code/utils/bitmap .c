
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
inline void SetBit(bitmap_t bitmap, uint8_t bit)
{
    bitmap[(bit >> 3)] |= (1 << (bit & 7));
}

/**
 * @brief   Clears a specific bit in a bitmap.
 * @param   [out]   bitmap: bitmap to be modified.
 * @param   [in]    bit: Bit in bitmap to be cleared.
 * @details This function does not perform boundary checks,
 *          That is up to the caller to pre-check.
 */
inline void ClearBit(bitmap_t bitmap, uint8_t bit)
{
    bitmap[(bit >> 3)] &= ~(1 << (bit & 7));
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
inline bool GetBit(bitmap_t bitmap, uint8_t bit)
{
    return (bitmap[bit >> 3] & (1 << (bit & 7)));
}

/**
 * @brief   Finds The earliest cleared bit in bitmap.
 * @param   [in]    bitmap: bitmap to be checked.
 * @param   [in]    min: Starting bit-index to check the bitmap.
 * @param   [in]    max: Threshold maximum value in the check.
 * @return  max if no bit in range was cleared,
 *          index of cleared bit otherwise.
 * @details Search will only go up to max-1.
 */
inline uint32_t FindClear(bitmap_t bitmap, uint32_t min, uint32_t max)
{
    while (min < max) {
        if (!GetBit(bitmap, min))
            return min;
        else
            min++;
    }

    return max;
}

/**
 * @brief   Finds The earliest set bit in bitmap.
 * @param   [in]    bitmap: bitmap to be checked.
 * @param   [in]    min: Starting bit-index to check the bitmap.
 * @param   [in]    max: Threshold maximum value in the check.
 * @return  max if no bit in range was set,
 *          index of set bit otherwise.
 * @details Search will only go up to max-1.
 */
inline uint32_t FindSet(bitmap_t bitmap, uint32_t min, uint32_t max)
{
    while (min < max) {
        if (GetBit(bitmap, min))
            return min;
        else
            min++;
    }

    return max;
}
