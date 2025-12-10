#include <cstdint>

void kernel_main() {
    const uint32_t addr = get_arg_val<uint32_t>(0);
    const uint32_t num_tiles = get_arg_val<uint32_t>(1);

    constexpr uint32_t cb_in0 = 0;
    constexpr uint32_t bank_id = 0;
    const uint32_t tile_size_bytes = get_tile_size(cb_in0);

    auto in0_dram_addr = get_noc_addr_from_bank_id<true>(bank_id, addr);

    for (uint32_t i = 0; i < num_tiles; i++) {
        cb_reserve_back(cb_in0, 1);
        uint32_t cb_in0_addr = get_write_ptr(cb_in0);

        noc_async_read(in0_dram_addr, cb_in0_addr, tile_size_bytes);
        noc_async_read_barrier();

        cb_push_back(cb_in0, 1);
    }
}

