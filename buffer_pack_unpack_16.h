#include <stddef.h>
/// @file
/// @brief Packing/unpacking of data into/from buffers with 16-bit elements.
///
/// Some CPU architectures has bytes number of bits not equal to 8. In this case classical 8-bit data storage may be optiomized.
/// For example if byte is 16 bit wide, two bytes may be stored per one native 16 bit byte. The functions below do
/// such packing/unpacking for architectures with 16-bit bytes.
///
/// Used terminology:
/// Byte                          : 8-bit data unit
/// Minimum Addressable Unit, MAU : Minimally possible addressable data unit on a given CPU architecture (16 bits).


/// @brief Unpacks data represented as packed 16-bit values.
/// @param dst_unpacked - Destination buffer for unpacked data.
/// @param src_base_packed - Base pointer to the source buffer with packed data.
/// @param src_offset - Offset to the first actual byte to be unpacked (within src_base_packed buffer)
/// @param n - Number of bytes to unpack
void buffer_unpack16(
    void* dst_unpacked, 
    const void* src_base_packed, 
    const size_t src_offset, 
    size_t n);


/// @brief Packs data represented as unpacked 16-bit values.
/// @param dst_base_packed - Base pointer to the destination buffer for packed data.
/// @param dst_offset - Offset to the start of the packed data (within dst_base_packed buffer)
/// @param src_unpacked - Source buffer with unpacked data.
/// @param n - Number of bytes to pack
/// @warning MAU bits higher than 7 (counting from 0) will be cleared.
void buffer_pack16(
    void* dst_base_packed, 
    const size_t dst_offset, 
    const void* src_unpacked,
    size_t n);
