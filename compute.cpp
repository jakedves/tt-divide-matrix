#include <cstdint>
#include "compute_kernel_api/common.h"
#include "compute_kernel_api/tile_move_copy.h"
#include "compute_kernel_api/eltwise_unary/eltwise_unary.h"
#include "compute_kernel_api/eltwise_unary/exp.h"
#include "compute_kernel_api/eltwise_unary/fill.h"
#include "compute_kernel_api/eltwise_binary.h"

namespace NAMESPACE {

void MAIN {
    uint32_t n_tiles = get_arg_val<uint32_t>(0);
    uint32_t cb_in0 = 0;
    uint32_t cb_out = 16;
    uint32_t idst = 0;

    init_sfpu(cb_out, cb_out);

    for (uint32_t i = 0; i < n_tiles; i++) {
        cb_wait_front(cb_in0, 1);
        copy_tile_init(cb_in0);

        tile_regs_acquire();
        fill_tile_init();
        fill_tile(idst, 1.0f);

        // overwrite the above fill_tile
        const uint32_t tile_index = 0;
        copy_tile(cb_in0, tile_index, idst);

        tile_regs_commit();
        tile_regs_wait();

        cb_reserve_back(cb_out, 1);

        pack_reconfig_data_format(cb_out);
        pack_tile(idst, cb_out);

        tile_regs_release();
        cb_push_back(cb_out, 1);
        cb_pop_front(cb_in0, 1);
    }
}

} // namespace NAMESPACE
