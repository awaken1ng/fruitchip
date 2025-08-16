#ifndef __FLETCHER16_H__
#define __FLETCHER16_H__

// https://en.wikipedia.org/wiki/Fletcher's_checksum#Straightforward

unsigned short fletcher16(unsigned char *data, int count)
{
   unsigned short sum1 = 0;
   unsigned short sum2 = 0;
   int index;

   for (index = 0; index < count; ++index)
   {
      sum1 = (sum1 + data[index]) % 255;
      sum2 = (sum2 + sum1) % 255;
   }

   return (sum2 << 8) | sum1;
}

#endif // __FLETCHER16_H__
