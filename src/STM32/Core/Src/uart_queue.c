#include "uart_queue.h"
#include <string.h>

#define UART_QUEUE_SIZE 20

// Queue to store moisture and power values between tasks
static UART_Message_t queue[UART_QUEUE_SIZE];
static volatile uint8_t head = 0;
static volatile uint8_t tail = 0;

void UQ_Init(void)
{
    head = 0;
    tail = 0;
}

bool UQ_Push(UART_Message_t msg)
{
    uint8_t next = (head + 1) % UART_QUEUE_SIZE;

    if (next == tail)
        return false;

    queue[head] = msg;
    head = next;
    return true;
}

bool UQ_Pop(UART_Message_t *out)
{
    if (head == tail)
        return false;

    *out = queue[tail];
    tail = (tail + 1) % UART_QUEUE_SIZE;

    return true;
}
