#include <cstdint>
#include "debug/dprint.h"

void kernel_main() {
    const uint32_t addr0 = get_arg_val<uint32_t>(0);
    const uint32_t addr1 = get_arg_val<uint32_t>(1);
    const uint32_t num_tiles = get_arg_val<uint32_t>(2);

    constexpr uint32_t cb_in0 = 0;
    constexpr uint32_t cb_in1 = 0;
    constexpr uint32_t bank_id = 0;
    const uint32_t tile_size_bytes = get_tile_size(cb_in0);

    auto in0_dram_addr = get_noc_addr_from_bank_id<true>(bank_id, addr0);
    auto in1_dram_addr = get_noc_addr_from_bank_id<true>(bank_id, addr1);

    for (uint32_t i = 0; i < num_tiles; i++) {
        cb_reserve_back(cb_in0, 1);
        uint32_t cb_in0_addr = get_write_ptr(cb_in0);

        noc_async_read(in0_dram_addr, cb_in0_addr, tile_size_bytes);
        noc_async_read_barrier();

        cb_push_back(cb_in0, 1);

        DPRINT << "First CB handled by reader" << ENDL();

        // second input
        cb_reserve_back(cb_in1, 1);
        uint32_t cb_in1_addr = get_write_ptr(cb_in1);

        noc_async_read(in1_dram_addr, cb_in1_addr, tile_size_bytes);
        noc_async_read_barrier();

        cb_push_back(cb_in1, 1);

        DPRINT << "Second CB handled by reader" << ENDL();
    }
       
    DPRINT << "Reader kernel complete." << ENDL();
}

