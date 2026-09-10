#include <criterion/criterion.h>
#include <string.h>
#include "libcnassets.h"

Test(bufrd_u32_be, return_proper_big_endian_value)
{
    const uint8_t test[sizeof(uint32_t)] = {
        0xab, 0xcd, 0xef, 0x12
    };

    cr_assert_eq(bufrd_u32_be((const void *)test), (uint32_t)0xabcdef12);
}


Test(bufrd_u32_le, return_proper_big_endian_value)
{
    const uint8_t test[sizeof(uint32_t)] = {
        0xab, 0xcd, 0xef, 0x12
    };

    cr_assert_eq(bufrd_u32_le((const void *)test), (uint32_t)0x12efcdab);
}
