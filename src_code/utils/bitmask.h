

#ifndef BITMASK_H
#define BITMASK_H

#include <stdint.h>

typedef uint8_t* bitmask_t;

inline void SetBit(uint8_t* bitmask, uint8_t bit)
{
    pid_bitmap[(pid >> 3)] |= (1u << (pid & 7u));
}

inline void ClearBit(uint8_t* bitmask, uint8_t bit)
{
    pid_bitmap[(pid >> 3)] &= ~(1u << (pid & 7u));
}

inline uint8_t GetBit(uint8_t* bitmask, uint8_t bit)
{
    return (pid_bitmap[i >> 3] & (1 << (i & 7)))
}


#endif	// BITMASK_H