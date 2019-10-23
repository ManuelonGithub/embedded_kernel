/**
 * @file    uart.c
 * @brief   Contains functionality to operate the UART0 driver for the tiva board.
 * @author  Manuel Burnay, Emad Khan (Based on his work)
 * @date    2019.09.18 (Created)
 * @date    2019.10.04 (Last Modified)
 */

#include <string.h>
#include "uart.h"

static uart_descriptor_t* UART0;

/**
 * @brief   Initializes the control registers for UART0 and the UART descriptor
 *          that is accessed by the driver.
 * @param   [in,out] descriptor: pointer to uart descriptor that will be accessed by the driver.
 * @param   [in] echo_en: Specifies if RX echo is enabled at driver level.
 * @param   [in] auto_flush_en: Specifies if driver should automatically send TX data if available.
 *
 * @todo    Convert the boolean configurations into a bit-style configuration.
 */
void UART0_Init(uart_descriptor_t* descriptor)
{
    volatile int wait;

    /* Initialize UART0 */
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCUART_GPIOA;   // Enable Clock Gating for UART0
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCGPIO_UART0;   // Enable Clock Gating for PORTA
    wait = 0; // give time for the clocks to activate

    UART0_CTL_R &= ~UART_CTL_UARTEN;        // Disable the UART
    wait = 0;   // wait required before accessing the UART config regs

    // Setup the BAUD rate
    UART0_IBRD_R = 8;   // IBRD = int(16,000,000 / (16 * 115,200)) = 8.680555555555556
    UART0_FBRD_R = 44;  // FBRD = int(.680555555555556 * 64 + 0.5) = 44.05555555555556

    UART0_LCRH_R = (UART_LCRH_WLEN_8);  // WLEN: 8, no parity, one stop bit, without FIFOs)

    GPIO_PORTA_AFSEL_R = 0x3;        // Enable Receive and Transmit on PA1-0
    GPIO_PORTA_PCTL_R = (0x01) | ((0x01) << 4);         // Enable UART RX/TX pins on PA1-0
    GPIO_PORTA_DEN_R = EN_DIG_PA0 | EN_DIG_PA1;        // Enable Digital I/O on PA1-0

    UART0_CTL_R = UART_CTL_UARTEN;        // Enable the UART
    wait = 0; // wait; give UART time to enable itself.

    UART0 = descriptor;

    circular_buffer_init(&UART0->tx);
    circular_buffer_init(&UART0->rx);

    UART0_InterruptEnable(INT_VEC_UART0);       // Enable UART0 interrupts
    UART0_IntEnable(UART_INT_RX | UART_INT_TX); // Enable Receive and Transmit interrupts
}

/**
 * @brief   Sets the interrupt enable bit for a peripheral in the NVIC register.
 * @param   [in] InterruptIndex: The peripheral's interrupt index in the NVIC register.
 *
 * @todo    Move this onto the cpu.h since it's a general way to enable the interrupt in the NVIC register.
 */
void UART0_InterruptEnable(unsigned long InterruptIndex)
{
	/* Indicate to CPU which device is to interrupt */
	if(InterruptIndex < 32)
	    NVIC_EN0_R = 1 << InterruptIndex;       // Enable the interrupt in the EN0 Register
	else
	    NVIC_EN1_R = 1 << (InterruptIndex - 32);    // Enable the interrupt in the EN1 Register
}

/**
 * @brief   Sets bits in the UART0 interrupt mask register.
 * @param   [in] flags: Determines which bits will be set in the register based on its set bits.
 */
void UART0_IntEnable(unsigned long flags)
{
	/* Set specified bits for interrupt */
    UART0_IM_R |= flags;
}

/**
 * @brief   Interrupt Handler for UART0.
 * @details This handler is shared between all possible interrupt types for the UART peripheral.
 *          The types of interrupts enabled are determined by the interrupt mask register.
 *          This means that the handler needs to have code to handle all enabled interrupt types.
 *          Currently it only handles interrupts for successful RX and TX.
 * @details The handler is what's in charge for acting based on the echo configuration of the UART descriptor.
 */
void UART0_IntHandler(void)
{
    if (UART0_MIS_R & UART_INT_RX) {
        /* RECV done - clear interrupt and make char available to application */
        UART0_ICR_R |= UART_INT_RX;

        enqueuec(&UART0->rx, UART0_DR_R);

        if (UART0->echo) {
            enqueuec(&UART0->tx, UART0_DR_R);
        }
    }

    if (UART0_MIS_R & UART_INT_TX) {
        /* XMIT done - clear interrupt */
        UART0_ICR_R |= UART_INT_TX;
    }

    if (buffer_size(&UART0->tx) != BUFFER_EMPTY) {
        UART0_putc(dequeuec(&UART0->tx));
    }
}

/**
 * @brief   Send a character to UART 0.
 * @param   [in] c: Character to be transmitted.
 * @details It sends a character to UART0's data register when the UART0 peripheral is ready to transmit.
 *          This function is blocking program progression while UART0 isn't ready to transmit.
 */
inline void UART0_putc(char c)
{
    while(!UART0_TxReady());
    UART0_DR_R = c;
}

/**
 * @brief   Determines if UART 0 is ready to transmit.
 * @return  [bool] True if ready, false if busy.
 * @details TX ready is based in the BUSY flag in the UART 0's flag register.
 */
inline bool UART0_TxReady(void)
{
    return !(UART0_FR_R & UART_FR_BUSY);
}

/**
 * @brief   Sends char string to UART 0.
 * @details This function will block if at the time of call,
 *          The TX buffer cannot hold the whole string.
 *          Function will block until the whole string has been queued to send.
 */
void UART0_puts(char* str)
{
    uint32_t length = strlen(str);
    uint32_t bytes_sent = 0;

    while (bytes_sent != length) {
        /*
         * Although there is no issues with calling UART0_send when the buffer is full,
         * doing so might be worst for code progression than to only call it once there is room
         * to queue more characters from the string.
         */
        if (buffer_size(&UART0->tx) != BUFFER_FULL) 
            bytes_sent += UART0_put(str+bytes_sent, length-bytes_sent);
    }
}

/**
 * @brief   Sends byte stream to UART 0.
 * @param   [in] data: pointer to string of bytes to be sent.
 * @param   [in] length: amount of bytes in the byte stream.
 * @return  [uint32_t] Returns amount of bytes successfully sent to UART 0.
 * @details This function does not guarantee that all bytes in the string are sent.
 *          if there isn't enough space in the TX buffer, the byte stream is truncated.
 */
uint32_t UART0_put(char* data, uint8_t length)
{
    uint8_t bytes_sent = enqueue(&UART0->tx, data, length);

    UART0_putc(dequeuec(&UART0->tx));

    return bytes_sent;
}

/**
 * @brief   Retrieves string from UART 0.
 * @param   [out] str: where the string will be copied onto.
 * @param   [in] MAX_BYTES: max size of the destination string buffer.
 * @return  [uint32_t] Amount of bytes copied into the buffer.
 * @details This function copies bytes from the UART's rx buffer until
 *          an end of a string has been reached,
 *          or the max amount of bytes that the buffer supports have been read.
 * @details The string copied onto str will always be null-terminated.
 * @details This function will block code progression until a valid string has been
 *          retrieved from UART, or until the max amount of bytes have been read.
 */
uint32_t UART0_gets(char* str, uint32_t MAX_BYTES)
{
    uint32_t bytes_read = 0;
    bool str_done = false;
    char c;

    while (bytes_read < MAX_BYTES && !str_done) {
        if (buffer_size(&UART0->rx) != BUFFER_EMPTY) {
            c = dequeuec(&UART0->rx);
            str[bytes_read++] = c;
            str_done = (c == '\n' || c == '\0' || c == '\r');
        }
    }

    if (bytes_read == MAX_BYTES) {
        str[(bytes_read-1)] = '\0';
    }
    else {
        str[bytes_read++] = '\0';
    }

    return bytes_read;
}
