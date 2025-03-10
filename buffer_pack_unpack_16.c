#include "buffer_pack_unpack_16.h"
#include <stdint.h>


#define BYTE_UNPACK_SHIFT 8
void buffer_unpack16(void* dst_unpacked, const void* src_base_packed, const size_t src_offset, size_t n) {
    size_t src_byte_offset = (src_offset & 1) * BYTE_UNPACK_SHIFT;
    uint16_t* src_packed = (uint16_t*) src_base_packed + (src_offset / 2);
    uint16_t* dest_unpacked_start = (uint16_t*) dst_unpacked;
    uint16_t* dest_unpacked_end = dest_unpacked_start + n;

    for ( ; dest_unpacked_start != dest_unpacked_end; 
        dest_unpacked_start++) {
        *dest_unpacked_start = (*src_packed >> src_byte_offset) & 0x00FF;
        src_packed += !!src_byte_offset;
        src_byte_offset ^= BYTE_UNPACK_SHIFT;
    }
}

void buffer_pack16(void* dst_base_packed, const size_t dst_offset, const void* src_unpacked, size_t n) {
    size_t dst_byte_offset = (dst_offset & 1) * BYTE_UNPACK_SHIFT;
    uint16_t* dest_packed = (uint16_t*) dst_base_packed + (dst_offset / 2);
    uint16_t* src_unpacked_start = (uint16_t*) src_unpacked;
    uint16_t* src_unpacked_end = src_unpacked_start + n;

    for ( ; src_unpacked_start != src_unpacked_end; 
        src_unpacked_start++) {
        uint16_t mask = 0xFF00 >> dst_byte_offset;
        *dest_packed = ( (*dest_packed) & mask ) | ( (*src_unpacked_start & 0x00FF) << dst_byte_offset );;
        dest_packed += !!dst_byte_offset;
        dst_byte_offset ^= BYTE_UNPACK_SHIFT;
    }
}

