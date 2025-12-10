#include <cmath>
#include <tt-metalium/host_api.hpp>
#include <tt-metalium/device.hpp>

using namespace tt;
using namespace tt::tt_metal;


void print_first_tile(const std::vector<float>& v, int elems);


int main() {
    IDevice* device = CreateDevice(0);
    CommandQueue& cq = device->command_queue();
    Program program = CreateProgram();
    constexpr CoreCoord core = {0, 0};

    constexpr uint32_t n_tiles = 1;
    constexpr uint32_t elements_per_tile = 32 * 32;

    // FP32 => 4 bytes
    constexpr uint32_t tile_size_bytes = 4 * elements_per_tile;

    std::vector<float> result_vec(elements_per_tile, 5.0f);

    printf("\nAllocated initial vector:\n");
    print_first_tile(result_vec, 32 * 32);

    constexpr uint32_t buffer_size = tile_size_bytes * n_tiles;

    InterleavedBufferConfig dram_config {
        .device = device,
        .size = buffer_size,
        .page_size = buffer_size,
        .buffer_type = BufferType::DRAM
    };

    // Allocate a device DRAM buffer for the result
    std::shared_ptr<Buffer> dst_dram_buffer = CreateBuffer(dram_config);

    // Allocate a circular buffer for holding the output tile
    constexpr uint32_t cb_out = CBIndex::c_16;
    constexpr uint32_t num_output_tiles = 1;

    CircularBufferConfig cb_out_config = CircularBufferConfig(
        buffer_size, {{cb_out, DataFormat::Float32}}
    ).set_page_size(cb_out, buffer_size);

    CreateCircularBuffer(program, core, cb_out_config);

    KernelHandle compute = CreateKernel(
        program,
        "compute.cpp",
        core,
        ComputeConfig {
            .math_fidelity = MathFidelity::HiFi4,
            .fp32_dest_acc_en = false,
            .math_approx_mode = false,
        }
    );

    KernelHandle writer = CreateKernel(
        program,
        "writer.cpp",
        core,
        DataMovementConfig {
            .processor = DataMovementProcessor::RISCV_0,
            .noc = NOC::RISCV_0_default,
            .compile_args = {}
        }
    );

    SetRuntimeArgs(program, compute, core, {n_tiles});

    // completely contained within the first dram bank as no interleaving
    // done, because we set .size == .page_size in the buffer config
    SetRuntimeArgs(program, writer, core, {dst_dram_buffer->address(), n_tiles});

    EnqueueProgram(cq, program, false);
    Finish(cq);

    EnqueueReadBuffer(cq, dst_dram_buffer, result_vec, true);

    CloseDevice(device);

    print_first_tile(result_vec, 32 * 32);

    return 0;
}

void print_first_tile(const std::vector<float>& v, int elems) {
    for (int i = 0; i < elems; i++) {
        // print every other element for readability
        if (i % 2 == 1) { continue; }

        if (i % 32 == 0) {
            printf("\n");
        }

        printf("%.1f ", v[i]);
    }
    printf("\n");
}
