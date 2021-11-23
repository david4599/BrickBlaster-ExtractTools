/* 
 * Undiet unpacker extracted from the "tombexcavator" GNU GPLv3 project 
 * (https://code.google.com/archive/p/tombexcavator/)
 * 
 * Modified to write decompressed data to file and allow compilation on 
 * Visual Studio 2019
 *
 * david4599 - 2021
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdexcept>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"

/*
   CRC-16

   16 bit, non-reflected CRC using the polynomial 0x1021
   and the initial and final xor values shown below.
   in other words, the CCITT standard CRC-16
*/

#define CRC16_INIT_VALUE	0xffff
#define CRC16_XOR_VALUE		0x0000

static unsigned short crctable[256];

/*
   Generate a table for a byte-wise 16-bit CRC calculation on the polynomial:
   x^16 + x^12 + x^5 + x^0
*/

void make_crc_table(void)
{
    int i, j;
    unsigned long poly, c;
    /* terms of polynomial defining this crc (except x^16): */
    //	static const byte p[] = {0,5,12};
    static const unsigned char p[] = { 0,2,5,6,8,10,11,12,13,15 };
    /* make exclusive-or pattern from polynomial (0x1021) */
    poly = 0L;
    for (i = 0; i < sizeof(p) / sizeof(unsigned char); i++) {
        poly |= 1L << p[i];
    }

    for (i = 0; i < 256; i++) {
        c = i << 8;
        for (j = 0; j < 8; j++) {
            c = (c & 0x8000) ? poly ^ (c << 1) : (c << 1);
        }
        crctable[i] = (unsigned short)c;
    }
}


void CRC16_InitChecksum(unsigned short& crcvalue) {
    crcvalue = CRC16_INIT_VALUE;
}

void CRC16_Update(unsigned short& crcvalue, const unsigned char data) {
    crcvalue = (crcvalue << 8) ^ crctable[(crcvalue >> 8) ^ data];
}

void CRC16_UpdateChecksum(unsigned short& crcvalue, const void* data, int length) {
    unsigned short crc;
    const unsigned char* buf = (const unsigned char*)data;

    crc = crcvalue;
    while (length--) {
        crc = (crc << 8) ^ crctable[(crc >> 8) ^ *buf++];
    }
    crcvalue = crc;
}

void CRC16_FinishChecksum(unsigned short& crcvalue) {
    crcvalue ^= CRC16_XOR_VALUE;
}

unsigned short CRC16_BlockChecksum(const void* data, int length) {
    unsigned short crc;

    CRC16_InitChecksum(crc);
    CRC16_UpdateChecksum(crc, data, length);
    CRC16_FinishChecksum(crc);
    return crc;
}





// =========================================================================
static const long DLZ_HEADER_SIZE = 0x11;

bool get_dlz_info(const char* fname, unsigned int& csize,
    unsigned int& dsize,
    uint16_t& crc)
{
    FILE* f = fopen(fname, "rb");
    if (f == NULL)
    {
        return false;
    }

    unsigned int unk;
    fread(&unk, 1, 4, f);
    unsigned char h[2 + 3];
    fread(&h, 1, 5, f);

    if (h[2] != 'd' || h[3] != 'l' || h[4] != 'z')
    {
        fclose(f);
        return false;
    }

    unsigned char flags;
    fread(&flags, 1, 1, f);

    unsigned short size_low;
    fread(&size_low, 2, 1, f);

    unsigned int size_h = flags & 0xF;
    unsigned int compressed_size = (size_h << 16) | (size_low & 0xFFFF);

    fread(&crc, 2, 1, f);

    unsigned char msize_h;
    fread(&msize_h, 1, 1, f);
    unsigned short msize_low;
    fread(&msize_low, 2, 1, f);

    unsigned int msize_hw = (msize_h >> 2) & 0x3F;
    unsigned int msize_l = msize_low & 0xFFFF;
    unsigned int msize = (msize_hw << 16) | msize_l;

    fclose(f);
    csize = compressed_size;
    dsize = msize;

    return true;
}
// --------------------------------------------------------------------
class dlz_decoder_c
{
public:
    //dlz_decoder_c(const char* ibuff, size_t ibuff_size, const char* tb);
    dlz_decoder_c(const char* ibuff, size_t ibuff_size);
    void decode(char* obuff, size_t obuff_size);
private:
    bool _get_control_bit();
    unsigned char _loadb();
    void _storeb();
private:
    const char* m_ibuff;
    const size_t   m_ibuff_size;
    char* m_obuff;
    uint32_t       m_obuff_ptr;
    size_t         m_obuff_size;
    size_t         m_work_buff_ptr;
    uint16_t       m_code_word;
    unsigned char           m_bits;
    unsigned char           m_data;
    //const char* m_test;
};

//dlz_decoder_c::dlz_decoder_c(const char* ibuff, size_t ibuff_size, const char* tb)
dlz_decoder_c::dlz_decoder_c(const char* ibuff, size_t ibuff_size)
    : m_ibuff(ibuff),
    m_ibuff_size(ibuff_size),
    m_obuff(0),
    m_obuff_ptr(0),
    m_obuff_size(0),
    m_work_buff_ptr(0),
    m_code_word(0),
    m_bits(1),
    m_data(0)
    //m_test(tb)
{

}
// -------------------------------------------------------------------
unsigned char dlz_decoder_c::_loadb()
{
    if (m_work_buff_ptr >= m_ibuff_size)
    {
        printf("offset too big\n");
        throw std::runtime_error("offset too big");
    }
    m_data = (unsigned char)m_ibuff[m_work_buff_ptr];
    m_work_buff_ptr++;
    return m_data;
}
// -------------------------------------------------------------------
void dlz_decoder_c::_storeb()
{
    m_obuff[m_obuff_ptr] = (char)m_data;
    /*unsigned char t = (unsigned char)m_test[m_obuff_ptr];
    if (t != m_data)
    {
        int len = 0;
        int p = m_obuff_ptr;
        while (p >= 0)
        {
            if (m_obuff[p] == t)
            {
                break;
            }
            len++;
            p--;
        }
        printf("ERROR: %d. Expected = %d, Actual = %d, Closest location %d\n",
            m_obuff_ptr, t & 0xFF, m_data & 0xFF, len);
        exit(1);
    }*/
    m_obuff_ptr++;
}
// -------------------------------------------------------------------
bool dlz_decoder_c::_get_control_bit()
{
    bool lsb = ((m_code_word & 0x1) == 0x1);
    m_code_word = m_code_word >> 1;
    m_bits--;

    if (m_bits == 0)
    {
        _loadb();
        unsigned char a = m_data;
        _loadb();
        unsigned char b = m_data;

        reg16_t r;
        R_LOW(r) = a;
        R_HIGH(r) = b;
        m_code_word = r.data;

        m_bits = 0x10;
    }
    return lsb;
}
// --------------------------------------------------------------------
bool rcl(unsigned char& x, bool cf)
{
    const unsigned char mask = 1 << 7;
    bool temp_cf = ((x & mask) == mask);
    x = ((x << 1) + (cf & 0x1));
    return temp_cf;
}
// --------------------------------------------------------------------
void dlz_decoder_c::decode(char* obuff, size_t obuff_size)
{

    reg16_t bx;
    reg16_t cx;
    unsigned char dh = 0;
    bool c;

    size_t offset;

    m_obuff = obuff;
    m_obuff_size = obuff_size;
    _get_control_bit(); // loads code word

u11:;
    cx.data = 0;
    while (true)
    {
        c = _get_control_bit();
        if (!c) break;
        _loadb();
        _storeb();
    }


    c = _get_control_bit();

    R_LOW(bx) = _loadb();
    R_HIGH(bx) = 0xFF;

    if (c)
    {
        c = _get_control_bit();
        rcl(R_HIGH(bx), c);

        c = _get_control_bit();
        if (!c)
        {
            dh = 2;
            R_LOW(cx) = 3;
            while (true)
            {
                c = _get_control_bit();
                if (!c)
                {
                    c = _get_control_bit();
                    rcl(R_HIGH(bx), c);
                    dh = dh << 1;
                    cx.data--;
                    if (cx.data == 0) break;
                }
                else
                {
                    break;
                }
            }
            R_HIGH(bx) = R_HIGH(bx) - dh;
        }
        dh = 2;
        R_LOW(cx) = 4;
    u3:;
        while (true)
        {
            dh++;
            c = _get_control_bit();
            if (!c)
            {
                cx.data--;
                if (cx.data != 0)
                {
                    goto u3;
                }
                else
                {
                    c = _get_control_bit();
                    if (!c)
                    {
                        c = _get_control_bit();
                        if (c)
                        {
                            R_LOW(cx) = _loadb();
                            cx.data += 0x11;
                        }
                        else
                        {
                            R_LOW(cx) = 3;
                            dh = 0;

                            for (int i = 0; i < cx.data; i++)
                            {
                                c = _get_control_bit();
                                rcl(dh, c);
                            }
                            cx.data = 0;

                            dh += 9;
                            R_LOW(cx) = dh;
                        }
                        goto u10;
                    }
                    dh++;
                    c = _get_control_bit();
                    if (c)
                    {
                        dh++;
                    }
                }

            }
            R_LOW(cx) = dh;
            break;

        }
        goto u10;
    }

    c = _get_control_bit();
    if (c)
    {
        for (int i = 0; i < 3; i++)
        {
            c = _get_control_bit();
            rcl(R_HIGH(bx), c);
        }
        R_HIGH(bx)--;
        cx.data = 2;
    }
    else
    {
        if (R_LOW(bx) != R_HIGH(bx))
        {
            R_LOW(cx) = 2;
        }
        else
        {
            c = _get_control_bit();
            if (c) goto u11;
            goto up;
        }
    }

u10:;
    for (int i = 0; i < cx.data; i++)
    {
        int32_t s_bx = 0xFFFF0000 | (bx.data & 0xFFFF);
        offset = m_obuff_ptr + s_bx;
        m_data = (unsigned char)obuff[offset];
        _storeb();
    }
    cx.data = 0;
    goto u11;

up:;
    //printf("%d = 10528\n", m_obuff_ptr);
}

// --------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Usage : %s <input file> <output file>\n", argv[0]);
        return -1;
    }

    char* input_filename = argv[1];
    char* output_filename = argv[2];

    unsigned int csize;
    unsigned int dsize;
    uint16_t crc;


    // Parse header
    if (!get_dlz_info(input_filename, csize, dsize, crc))
    {
        printf("BAD DLZ\n");
        return -2;
    }

    /*printf("Compressed Size   : %d\n", csize);
    printf("Decompressed Size : %d\n", dsize);
    printf("CRC               : %X\n", crc);*/


    // Read compressed file (without header)
    FILE* f = fopen(input_filename, "rb");
    fseek(f, DLZ_HEADER_SIZE, SEEK_SET);

    const size_t ibuff_size = csize;
    char* ibuff = new char[ibuff_size];

    const size_t obuff_size = dsize;
    char* obuff = new char[obuff_size];

    fread(ibuff, ibuff_size, 1, f);
    fclose(f);

    /*char* tb = new char[obuff_size];
    f = fopen(argv[2], "rb");
    fread(tb, obuff_size, 1, f);
    fclose(f);*/


    // Decode compressed data
    try
    {
        dlz_decoder_c d(ibuff, ibuff_size);
        d.decode(obuff, obuff_size);
    }
    catch (std::exception & e)
    {
        printf("EXCEPTION: %s\n", e.what());
        return -3;
    }


    // Calc checksum (not used)
    /*make_crc_table();
    uint16_t mycrc = CRC16_BlockChecksum(obuff, obuff_size);
    printf("CRC = %X\n", mycrc);*/


    // Write decompressed data to file
    f = fopen(output_filename, "wb");
    fwrite(obuff, obuff_size, 1, f);
    fclose(f);

    return 0;
}

