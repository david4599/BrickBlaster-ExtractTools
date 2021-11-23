/* 
 * Undiet unpacker extracted from the "tombexcavator" GNU GPLv3 project 
 * (https://code.google.com/archive/p/tombexcavator/)
 * 
 * Modified to write decompressed data to file and allow compilation on 
 * Visual Studio 2019
 *
 * david4599 - 2021
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

// ================================================
// Portable types
// ================================================
#ifdef NO_STD_TYPES
typedef char 	       int8_t;
typedef unsigned char  uint8_t;
typedef short	       int16_t;
typedef unsigned short uint16_t;
typedef int	       int32_t;
typedef unsigned int   uint32_t;
#endif

typedef uint8_t  byte;
typedef uint16_t word;
typedef uint32_t dword;
// ================================================
union reg16_t
{
    byte component[2];
    word data;


    reg16_t operator = (word w)
    {
        data = w;
        return *this;
    }
};

union reg32_t
{
    reg16_t component[2];
    dword   data;

    reg32_t operator = (dword w)
    {
        data = w;
        return *this;
    }
};

// ---------------------------------------------------------------------------------------
#define R_LOW(X)  X.component [0]
#define R_HIGH(X) X.component [1]


#endif
