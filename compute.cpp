#include <cstdint>
#include "compute_kernel_api/common.h"
#include "compute_kernel_api/tile_move_copy.h"
#include "compute_kernel_api/eltwise_unary/eltwise_unary.h"
#include "compute_kernel_api/eltwise_unary/exp.h"
#include "compute_kernel_api/eltwise_unary/fill.h"
#include "compute_kernel_api/eltwise_binary.h"
#include "compute_kernel_api/eltwise_binary_sfpu.h"

namespace NAMESPACE {

void MAIN {
    uint32_t n_tiles = get_arg_val<uint32_t>(0);
    uint32_t cb_in0 = 0;
    uint32_t cb_in1 = 1;
    uint32_t cb_intermediate = 2;
    uint32_t cb_out = 16;

    // dst reg slices to use
    uint32_t idst0 = 0;
    uint32_t idst1 = 1;
    uint32_t idst2 = 2;

    // datatype doesn't change, this should be fine
    init_sfpu(cb_out, cb_out);

    for (uint32_t i = 0; i < n_tiles; i++) {
        tile_regs_acquire();

        // dst[0] = 1.0f
        fill_tile_init();
        fill_tile(idst0, 1.0f);

        // overwrite the above fill_tile
        const uint32_t tile_index = 0;
        copy_tile_init(cb_in0);

        // dst[1] = cb0
        cb_wait_front(cb_in0, 1);
        copy_tile(cb_in0, tile_index, idst1);

        // dst[0] = dst[0] / dst[1]
        div_binary_tile_init();
        div_binary_tile(idst0, idst1);

        tile_regs_commit();
        tile_regs_wait();

        cb_reserve_back(cb_intermediate, 1);
        pack_reconfig_data_format(cb_intermediate);

        // cbi = dst[0]
        pack_tile(idst0, cb_intermediate);

        tile_regs_release();

        // cbi has data! cb0 unneeded furthermore
        cb_push_back(cb_intermediate, 1);
        cb_pop_front(cb_in0, 1);

        // now do the matrix unit step
        cb_wait_front(cb_in1, 1);
        cb_wait_front(cb_intermediate, 1);

        tile_regs_acquire();

        // dst[2] = cb1 * cbi
        mul_tiles_init(cb_in1, cb_intermediate);
        mul_tiles(cb_in1, cb_intermediate, 0, 0, idst2);

        tile_regs_commit();

        // cb1 unneeded furthermore, cbi unneeded furthermore
        cb_pop_front(cb_in1, 1);
        cb_pop_front(cb_intermediate, 1);

        // wait for space in cb_out
        cb_reserve_back(cb_out, 1);

        tile_regs_wait();

        // cb_out = dst[2]
        pack_reconfig_data_format(cb_out);
        pack_tile(idst2, cb_out);
        
        tile_regs_release();

        // cb_out has data!
        cb_push_back(cb_out, 1);
    }
}

} // namespace NAMESPACE
