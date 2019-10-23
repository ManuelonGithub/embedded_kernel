/**
 * @file   circular_buffer.c
 * @brief  C file all function definitions regarding circular buffer operation.
 * @author Manuel Burnay
 * @date   2019.09.17 (Created)
 * @date   2019.10.03 (Last Modified)
 */


#include <string.h>
#include "circular_buffer.h"

/**
 * @brief  Initializes a circular buffer structure.
 * @param  [out] buffer: pointer to circular buffer structure being initialized
 */
void circular_buffer_init(circular_buffer_t* buffer)
{
	buffer->wr_ptr = 0;
	buffer->rd_ptr = 0;
}

/**
 * @brief   Queues a char/byte into a circular buffer.
 * @param   [in, out] buffer: pointer to circular buffer being used.
 * @param   [in] data: char data being queued.
 * @details This method of enqueuing can corrupt the buffer if it's full.
 *          Always check if the queue if full or use enqueuec_s,
 *          which queues into the buffer safely albeit at cost of some overhead.
 */
inline void enqueuec(circular_buffer_t* buffer, char c)
{
	buffer->data[buffer->wr_ptr] = c;
	INC_PTR(buffer->wr_ptr);
}

/**
 * @brief   Safely Queues a char/byte into a circular buffer.
 * @param   [in, out] buffer: pointer to circular buffer being used.
 * @param   [in] data: char data being queued.
 * @param   [in] OVERWRITE: This flag determines the enqueuing behaviour if the queue is full.
 *                          If true, the enqueuing takes place (overwrites the oldest entry),
 *                          If false, the enqueuing will not take place.
 * @return  [bool] True if an enqueue was taken place, False if not.
 */
bool enqueuec_s(circular_buffer_t* buffer, char c, bool OVERWRITE)
{
    bool retval = false;
    uint32_t temp_wr = (buffer->wr_ptr+1) & CIRCULAR_BUFFER_MASK;

    // This prevents the read entry
    if (temp_wr != buffer->rd_ptr) {
        buffer->data[buffer->wr_ptr] = c;
        INC_PTR(buffer->wr_ptr);
        retval = true;
    }
    else if (OVERWRITE) {
        buffer->data[buffer->wr_ptr] = c;
        INC_PTR(buffer->wr_ptr);
        INC_PTR(buffer->rd_ptr);
        retval = true;
    }

    return retval;
}

/**
 * @brief   Enqueues a string into a circular buffer.
 * @param   [in, out] buffer: pointer to circular buffer being used.
 * @param   [in] s: null terminated string pointer.
 * @return  [uint32_t] Amount of bytes enqueued to buffer.
 * @detais  this is simply a wrapper function for enqueue(),
 *          where the string length is obtained and enqueue() is then called.
 */
inline uint32_t enqueues(circular_buffer_t* buffer, char* s)
{
    uint32_t length = strlen(s);

    return enqueue(buffer, s, length);
}

/**
 * @brief   Enqueues a length of bytes into a circular buffer.
 * @param   [in, out] buffer: pointer to circular buffer being used.
 * @param   [in] data: pointer to start of the byte stream being enqueued.
 * @param   [in] length: length of byte stream being enqueued.
 * @return  Amount of bytes queued to buffer.
 * @details It will only enqueue bytes until the buffer is full.
 *          (truncates length if it exceeds available space).
 *          It'll always queue string in order. Byte 0 -> length.
 */
uint32_t enqueue(circular_buffer_t* buffer, char* src_buf, uint32_t length)
{
    uint32_t space = CIRCULAR_BUFFER_SIZE - buffer_size(buffer);

    // truncate length if it's over the free space in the buffer
    if (length > space) {
        length = space;
    }

    if (src_buf != NULL) {
        if ((buffer->wr_ptr + length) > CIRCULAR_BUFFER_MASK) {
            space = CIRCULAR_BUFFER_SIZE - buffer->wr_ptr;
            memcpy((buffer->data + buffer->wr_ptr), src_buf, space);
            memcpy(buffer->data, src_buf+space, length-space);
        }
        else {
            memcpy((buffer->data + buffer->wr_ptr), src_buf, length);
        }

        MOV_PTR(buffer->wr_ptr, length);
    }
    else {
        length = 0;
    }

    return length;
}

/**
 * @brief   Dequeues a character from a circular buffer.
 * @param   [in, out] buffer: pointer to circular buffer being used.
 * @return  char/byte that was dequeued from buffer.
 * @details This function may corrupt the buffer if the buffer is empty.
 *          Always make sure to check if buffer contains data before using this function,
 *          or use dequeue_s(), which will dequeue from the buffer safely,
 *          albeit with added overhead.
 */
char dequeuec(circular_buffer_t* buffer)
{
	char retval = buffer->data[buffer->rd_ptr];
	INC_PTR(buffer->rd_ptr);

	return retval;
}

/**
 * @brief   Safely dequeues a character from a circular buffer.
 * @param   [in, out] buffer: pointer to circular buffer being used.
 * @param   [out] dst: pointer to char where the dequeued character will be placed if a dequeue can take place.
 * @return  [bool] True if a dequeue was taken place, False if not.
 * @details dst can be NULL if you don't wish to retain the dequeued entry.
 */
bool dequeuec_s(circular_buffer_t* buffer, char* dst)
{
    bool retval = false;
    if (buffer->wr_ptr != buffer->rd_ptr) {
        retval = true;
        if (dst != NULL) *dst = buffer->data[buffer->rd_ptr];
        INC_PTR(buffer->rd_ptr);
    }

    return retval;
}

/**
 * @brief   Dequeues a length of bytes.
 * @param   [in, out] buffer: pointer to circular buffer being used.
 * @param   [out] dst: pointer to byte buffer where the dequeued characters will be copied to.
 * @param   [in] length: Amount of bytes to be queued.
 * @return  [uint32_t] Amount of bytes dequeued.
 * @details This function will only dequeue bytes until the buffer is empty.
 */
uint32_t dequeue(circular_buffer_t* buffer, uint8_t* dst_buf, uint32_t length)
{
    uint32_t size = buffer_size(buffer);

    length = (length > size) ? size : length;

    if (dst_buf != NULL) {
        if ((buffer->rd_ptr + length) > CIRCULAR_BUFFER_MASK) {
                size = CIRCULAR_BUFFER_SIZE - buffer->rd_ptr;
                memcpy(dst_buf, (buffer->data + buffer->rd_ptr), size);
                memcpy(dst_buf+size, buffer->data, length-size);
        }
        else {
            memcpy(dst_buf, (buffer->data + buffer->rd_ptr), length);
        }
    }

    MOV_PTR(buffer->rd_ptr, length);

    return length;
}



/**
 * @brief  Get the size of the buffer/How many characters are currently queued.
 * @param  [in] buffer: pointer to circular buffer being used.
 * @return     Size of the buffer.
 */
inline uint32_t buffer_size(circular_buffer_t* buffer)
{
	return ((buffer->wr_ptr - buffer->rd_ptr) & CIRCULAR_BUFFER_MASK);
}
