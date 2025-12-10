#include <cstdint>
#include "debug/dprint.h"

void kernel_main() {
    uint32_t c_addr = get_arg_val<uint32_t>(0);
    uint32_t n_tiles = get_arg_val<uint32_t>(1);

    constexpr uint32_t cb_out = 16;
    constexpr uint32_t bank_id = 0;
    const uint32_t tile_size_bytes = get_tile_size(cb_out);

    auto dst_dram_addr = get_noc_addr_from_bank_id<true>(bank_id, c_addr);

    DPRINT << "Starting writer kernel loop." << ENDL();

    for (uint32_t i = 0; i < n_tiles; i++) {
        DPRINT << "Waiting for page in cb_out..." << ENDL();

        cb_wait_front(cb_out, 1);
        uint32_t cb_out_addr = get_read_ptr(cb_out);

        DPRINT << "Writing data from cb_out into DRAM" << ENDL();

        noc_async_write(cb_out_addr, dst_dram_addr, tile_size_bytes);
        noc_async_write_barrier();

        DPRINT << "Emptying cb_out." << ENDL();

        cb_pop_front(cb_out, 1);
    }

    DPRINT << "Writer kernel complete." << ENDL();
}

