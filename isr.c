#include <xc.h>
extern unsigned char return_time;
extern unsigned char sec;
void __interrupt() isr(void)
{
    static unsigned int count =0;
    if (TMR2IF == 1)
    {
        if(++count == 20000)
        {
            count = 0;
            if(sec > 0)
                sec--;
            if(return_time > 0)
                return_time--;
        }
        TMR2IF = 0;
    }
}