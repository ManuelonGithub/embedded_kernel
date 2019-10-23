/**
 * @file	circular_buffer.h
 * @brief	Header file with definitions, macros and function prototypes
 *			used to operate a circular buffer.
 * @author Manuel Burnay
 * @date	2019.09.17 (Created)
 * @date	2019.10.03 (Last Modified)
 */

#ifndef CIRCULAR_BUFFER_H
	#define CIRCULAR_BUFFER_H

	#include <stdlib.h>
    #include <stdint.h>
    #include <stdbool.h>
	#include <stdio.h>


	#define CIRCULAR_BUFFER_SIZE 128    /// Size of the circular buffer
	#define CIRCULAR_BUFFER_MASK (CIRCULAR_BUFFER_SIZE-1)   /// Circular buffer size mask. Used to constrain the pointers within the buffer array.

	#define BUFFER_FULL CIRCULAR_BUFFER_MASK
	#define BUFFER_EMPTY 0

	/**
	 * @brief   Pointer increment macro.
	 *          Safely increments a circular buffer pointer &
	 *          wraps it to the beginning when overflowing at the end of the buffer size.
	 */
	#define INC_PTR(ptr) (ptr = (ptr + 1) & CIRCULAR_BUFFER_MASK)

    /**
     * @brief   Pointer decrement macro.
     *          Safely decrements a circular buffer pointer &
     *          wraps it to the beginning when 'overflowing' at the beginning of the buffer size.
     */
    #define DEC_PTR(ptr) (ptr = (ptr - 1) & CIRCULAR_BUFFER_MASK)

    /**
     * @brief   Pointer move macro.
     *          Safely move a circular buffer pointer &
     *          wraps it to the beginning when 'overflowing' either at the end or beginning of the buffer size.
     */
    #define MOV_PTR(ptr, i) (ptr = (ptr + i) & CIRCULAR_BUFFER_MASK)

	/**
	 * @brief	circular buffer structure.
	 * @details The size of the buffer is determined by the CIRCULAR_BUFFER_SIZE
	 */
	typedef struct circular_buffer_{
		char data[CIRCULAR_BUFFER_SIZE];
		uint32_t rd_ptr;
		uint32_t wr_ptr;
	} circular_buffer_t;


	// Circular buffer function prototypes
	void circular_buffer_init(circular_buffer_t* buffer);

	inline void enqueuec(circular_buffer_t* buffer, char c);
	bool enqueuec_s(circular_buffer_t* buffer, char c, bool OVERWRITE);
	inline uint32_t enqueues(circular_buffer_t* buffer, char* s);
	uint32_t enqueue(circular_buffer_t* buffer, char* src_buf, uint32_t length);

	char dequeuec(circular_buffer_t* buffer);
	bool dequeuec_s(circular_buffer_t* buffer, char* dst);
	uint32_t dequeue(circular_buffer_t* buffer, uint8_t* dst_buf, uint32_t length);

	inline uint32_t buffer_size(circular_buffer_t* buffer);

#endif	// CIRCULAR_BUFFER_H
