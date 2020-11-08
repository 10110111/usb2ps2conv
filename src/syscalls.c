#include <sys/stat.h>
#include "stm32f4xx_hal_conf.h"
#include "syscalls.h"
#include "led.h"

#ifdef ENABLE_DEBUG_OUTPUT
extern UART_HandleTypeDef uartDbgHandle;
#endif
int __errno;

int _kill(int pid, int sig)
{
    return 0;
}

int _getpid(void)
{
    return 0;
}

int _close(int file)
{
    return 0;
}

int _fstat(int file, struct stat *st)
{
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(const char *name, int flags, int mode)
{
    return -1;
}

int _read(int file, char *ptr, int len)
{
    return -1;
}

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr asm ("sp");

caddr_t _sbrk_r(struct _reent *r, int incr)
{
    extern char   end asm ("end"); /* Defined by the linker.  */
    static char*  heap_end;
    char*         prev_heap_end;

    if(heap_end == NULL)
        heap_end = &end;

    prev_heap_end = heap_end;

    if(heap_end + incr > stack_ptr)
    {
        //errno = ENOMEM;
        return (caddr_t)-1;
    }

    heap_end += incr;

    return (caddr_t)prev_heap_end;
}

int _write(int file, char *ptr, int len)
{
#ifdef ENABLE_DEBUG_OUTPUT
    HAL_UART_Transmit(&uartDbgHandle, (uint8_t*)ptr, len, 5000);
#endif
    return len;
}

void _exit (int status)
{
    while(1);
}

void abort()
{
    ledsOn();
    while(1);
}

