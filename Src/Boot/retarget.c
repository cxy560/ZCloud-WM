/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/

#include <stdio.h>
#include <rt_misc.h>

#define HR_UART0_INT_MASK           (0x0F000800 + 0x14)
#define HR_UART0_FIFO_STATUS        (0x0F000800 + 0x1C)
#define HR_UART0_TX_WIN             (0x0F000800 + 0x20)
typedef volatile unsigned int TLS_REG; 

//#pragma import(__use_no_semihosting_swi)

static unsigned int std_libspace[__USER_LIBSPACE_SIZE];

void *__user_libspace(void)
{
    return (void *)&std_libspace;
}

extern int sendchar(int ch);

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) 
{
  return (sendchar(ch));
}


int ferror(FILE *f) 
{
  /* Your implementation of ferror */
  return EOF;
}

static __inline void tls_reg_write32(unsigned int reg, unsigned int val)
{
    *(TLS_REG *)reg = val;
}

static __inline unsigned int tls_reg_read32(unsigned int reg)
{
    unsigned int val = *(TLS_REG *)reg;
    return val;
}

int sendchar(int ch)
{
	tls_reg_write32(HR_UART0_INT_MASK, 0x3);
    if(ch == '\n')  
	{
		while (tls_reg_read32(HR_UART0_FIFO_STATUS) & 0x3F);
		tls_reg_write32(HR_UART0_TX_WIN, '\r');
    }
    while(tls_reg_read32(HR_UART0_FIFO_STATUS) & 0x3F);
    tls_reg_write32(HR_UART0_TX_WIN, (char)ch);
    tls_reg_write32(HR_UART0_INT_MASK, 0x0);
    return ch;
}

void _ttywrch(int ch) 
{
  sendchar(ch);
}

void _sys_exit(int return_code) 
{
label:  
	goto label;  /* endless loop */
}


