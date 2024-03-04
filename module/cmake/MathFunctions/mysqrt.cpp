#include <stdlib.h>

unsigned long sqrt_16(unsigned long a)
{     
    unsigned long rem = 0;
    unsigned long root = 0;
    unsigned long divisor = 0;
    for(int i=0; i<16; i++){
        root <<= 1;
        rem = ((rem << 2) + (a >> 30));
        a <<= 2;
        divisor = (root<<1) + 1;
        if(divisor <= rem){
            rem -= divisor;
            root++;
        }
    }
    return root;//(unsigned short)(root);
}

float mysqrt(float m)
{
    unsigned long s=0;
    float t=0.0;

    if (m>=1.00)
    {
        s=sqrt_16((unsigned long)(m));
        t=s+(m/s-s)/2;
    }
    else {
        t=float(0.4);
        t=t+(m/t-t)/2;  //牛顿切线法  
        t=t+(m/t-t)/2;  //次数越多，精度越高
        t=t+(m/t-t)/2;  
        t=t+(m/t-t)/2;  //精确可以在0.0001开方
    }
    //t=t+(m/t-t)/2;
    t=t+(m/t-t)/2;
    t=t+(m/t-t)/2;
    return t;
}
