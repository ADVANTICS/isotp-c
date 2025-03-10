#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include "buffer_pack_unpack_16.h"
#include <time.h>
#include <assert.h>

#define CHAR uint16_t

void test_buffer_pack_00(void) {
    uint16_t src_unpacked[] = {1, 2, 3, 4, 5, 6, 7, 8}; // 8 words, 8 bytes
    uint16_t dst_packed[5] = {0};                       // 6 words, 12 bytes
    uint16_t src_restored[] = {0x4242, 0, 0, 0, 0, 0, 0, 0, 0, 0x4343}; // 10 words, 8 bytes with santinels

    buffer_pack16(dst_packed, 0, src_unpacked, 8);

    assert(dst_packed[0]==0x0201);
    assert(dst_packed[1]==0x0403);
    assert(dst_packed[2]==0x0605);
    assert(dst_packed[3]==0x0807);
    assert(dst_packed[4]==0);

    buffer_unpack16(src_restored + 1, dst_packed, 0, sizeof(src_unpacked)/sizeof(*src_unpacked));
    assert(src_restored[0]==0x4242);
    assert(src_restored[1]==1);
    assert(src_restored[2]==2);
    assert(src_restored[3]==3);
    assert(src_restored[4]==4);
    assert(src_restored[5]==5);
    assert(src_restored[6]==6);
    assert(src_restored[7]==7);
    assert(src_restored[8]==8);
    assert(src_restored[9]==0x4343);
}

void test_buffer_pack_01(void) {
    uint16_t src_unpacked[] = {1, 2, 3, 4, 5, 6, 7, 8}; // 8 words, 8 bytes
    uint16_t dst_packed[6] = {0};                       // 6 words, 12 bytes
    uint16_t src_restored[] = {0x4242, 0, 0, 0, 0, 0, 0, 0, 0, 0x4343}; // 10 words, 8 bytes with santinels

    buffer_pack16(dst_packed, 2, src_unpacked, 8);

    assert(dst_packed[0]==0);
    assert(dst_packed[1]==0x0201);
    assert(dst_packed[2]==0x0403);
    assert(dst_packed[3]==0x0605);
    assert(dst_packed[4]==0x0807);
    assert(dst_packed[5]==0);

    buffer_unpack16(src_restored + 1, dst_packed, 2, sizeof(src_unpacked)/sizeof(*src_unpacked));
    assert(src_restored[0]==0x4242);
    assert(src_restored[1]==1);
    assert(src_restored[2]==2);
    assert(src_restored[3]==3);
    assert(src_restored[4]==4);
    assert(src_restored[5]==5);
    assert(src_restored[6]==6);
    assert(src_restored[7]==7);
    assert(src_restored[8]==8);
    assert(src_restored[9]==0x4343);
}

void test_buffer_pack_02(void) {
    uint16_t src_unpacked[] = {1, 2, 3, 4, 5, 6, 7, 8}; // 8 words, 8 bytes
    uint16_t dst_packed[] = {0x4242, 0, 0, 0, 0, 0x4343}; // 6 words, 12 bytes
    uint16_t src_restored[] = {0x4242, 0, 0, 0, 0, 0, 0, 0, 0, 0x4343}; // 10 words, 8 bytes with santinels

    buffer_pack16(dst_packed, 2, src_unpacked, 8);

    assert(dst_packed[0]==0x4242);
    assert(dst_packed[1]==0x0201);
    assert(dst_packed[2]==0x0403);
    assert(dst_packed[3]==0x0605);
    assert(dst_packed[4]==0x0807);
    assert(dst_packed[5]==0x4343);

    buffer_unpack16(src_restored + 1, dst_packed, 2, sizeof(src_unpacked)/sizeof(*src_unpacked));
    assert(src_restored[0]==0x4242);
    assert(src_restored[1]==1);
    assert(src_restored[2]==2);
    assert(src_restored[3]==3);
    assert(src_restored[4]==4);
    assert(src_restored[5]==5);
    assert(src_restored[6]==6);
    assert(src_restored[7]==7);
    assert(src_restored[8]==8);
    assert(src_restored[9]==0x4343);
}


void test_buffer_pack_03(void) {
    uint16_t src_unpacked[] = {1, 2, 3, 4, 5, 6, 7, 8}; // 8 words, 8 bytes
    uint16_t dst_packed[6] = {0};                       // 6 words, 12 bytes
    uint16_t src_restored[] = {0x4242, 0, 0, 0, 0, 0, 0, 0, 0, 0x4343}; // 10 words, 8 bytes with santinels

    buffer_pack16(dst_packed, 1, src_unpacked, 8);

    assert(dst_packed[0]==0x0100);
    assert(dst_packed[1]==0x0302);
    assert(dst_packed[2]==0x0504);
    assert(dst_packed[3]==0x0706);
    assert(dst_packed[4]==0x0008);
    assert(dst_packed[5]==0);

    buffer_unpack16(src_restored + 1, dst_packed, 1, sizeof(src_unpacked)/sizeof(*src_unpacked));
    assert(src_restored[0]==0x4242);
    assert(src_restored[1]==1);
    assert(src_restored[2]==2);
    assert(src_restored[3]==3);
    assert(src_restored[4]==4);
    assert(src_restored[5]==5);
    assert(src_restored[6]==6);
    assert(src_restored[7]==7);
    assert(src_restored[8]==8);
    assert(src_restored[9]==0x4343);
}

void test_buffer_pack_04(void) {
    uint16_t src_unpacked[] = {1, 2, 3, 4, 5, 6, 7, 8}; // 8 words, 8 bytes
    uint16_t dst_packed[] = {0x0042, 0, 0, 0, 0x4300, 0}; // 6 words, 12 bytes
    uint16_t src_restored[] = {0x4242, 0, 0, 0, 0, 0, 0, 0, 0, 0x4343}; // 10 words, 8 bytes with santinels

    buffer_pack16(dst_packed, 1, src_unpacked, 8);

    assert(dst_packed[0]==0x0142);
    assert(dst_packed[1]==0x0302);
    assert(dst_packed[2]==0x0504);
    assert(dst_packed[3]==0x0706);
    assert(dst_packed[4]==0x4308);
    assert(dst_packed[5]==0);

    buffer_unpack16(src_restored + 1, dst_packed, 1, sizeof(src_unpacked)/sizeof(*src_unpacked));
    assert(src_restored[0]==0x4242);
    assert(src_restored[1]==1);
    assert(src_restored[2]==2);
    assert(src_restored[3]==3);
    assert(src_restored[4]==4);
    assert(src_restored[5]==5);
    assert(src_restored[6]==6);
    assert(src_restored[7]==7);
    assert(src_restored[8]==8);
    assert(src_restored[9]==0x4343);
}

void test_buffer_pack_05(void) {
    uint16_t src_unpacked[] = { 1 }; // 1 word, 1 bytes
    uint16_t dst_packed[] = {0x4242, 0xDEAD}; // 2 words, 12 bytes
    uint16_t src_restored[] = {0x4242, 0xDEAD};  // 2 words, 12 bytes

    buffer_pack16(dst_packed, 0, src_unpacked, 1);

    assert(dst_packed[0]==0x4201);
    assert(dst_packed[1]==0xDEAD);  // Must be intact

    buffer_unpack16(src_restored, dst_packed, 0, 1);
    assert(src_restored[0]==0x0001);
    assert(src_restored[1]==0xDEAD);
}

void test_buffer_pack_06(void) {
    uint16_t src_unpacked[] = { 1 }; // 1 word, 1 bytes
    uint16_t dst_packed[] = {0x4242, 0xDEAD}; // 2 words, 12 bytes
    uint16_t src_restored[] = {0x4242, 0xDEAD};  // 2 words, 12 bytes

    buffer_pack16(dst_packed, 1, src_unpacked, 1);

    assert(dst_packed[0]==0x0142);
    assert(dst_packed[1]==0xDEAD);  // Must be intact

    buffer_unpack16(src_restored, dst_packed, 1, 1);
    assert(src_restored[0]==0x0001);
    assert(src_restored[1]==0xDEAD);
}

void test_buffer_pack_07(void) {
    uint16_t src_unpacked[] = { 1 , 2 }; // 1 word, 1 bytes
    uint16_t dst_packed[] = {0x4242, 0xDEAD}; // 2 words, 12 bytes
    uint16_t src_restored[] = {0x4242, 0x4343, 0xDEAD};  // 3 words, 6 bytes

    buffer_pack16(dst_packed, 0, src_unpacked, 2);

    assert(dst_packed[0]==0x0201);
    assert(dst_packed[1]==0xDEAD);  // Must be intact

    buffer_unpack16(src_restored, dst_packed, 0, 2);
    assert(src_restored[0]==0x0001);
    assert(src_restored[1]==0x0002);
    assert(src_restored[2]==0xDEAD);
}

void test_buffer_pack_08(void) {
    uint16_t src_unpacked[] = { 1, 2 }; // 1 word, 1 bytes
    uint16_t dst_packed[] = {0x4242, 0x4343, 0xDEAD}; // 3 words, 6 bytes
    uint16_t src_restored[] = {0x4242, 0x4343, 0xDEAD};  // 3 words, 6 bytes

    buffer_pack16(dst_packed, 1, src_unpacked, 2);

    assert(dst_packed[0]==0x0142);
    assert(dst_packed[1]==0x4302);
    assert(dst_packed[2]==0xDEAD);  // Must be intact

    buffer_unpack16(src_restored, dst_packed, 1, 2);
    assert(src_restored[0]==0x0001);
    assert(src_restored[1]==0x0002);
    assert(src_restored[2]==0xDEAD);
}


int main() {

    test_buffer_pack_00();
    test_buffer_pack_01();
    test_buffer_pack_02();
    test_buffer_pack_03();
    test_buffer_pack_04();
    test_buffer_pack_05();
    test_buffer_pack_06();
    test_buffer_pack_07();
    test_buffer_pack_08();
    return 0;
}
