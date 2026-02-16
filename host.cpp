#include <tt-metalium/host_api.hpp>
#include <tt-metalium/device.hpp>

#include "../utils-tt/tt-utils.h"

using namespace tt;
using namespace tt::tt_metal;


int main() {
    IDevice* device = CreateDevice(0);
    CommandQueue& cq = device->command_queue();
    Program program = CreateProgram();
    constexpr CoreCoord core = {0, 0};

    constexpr uint32_t n_tiles = 1;
    constexpr uint32_t elements_per_tile = 32 * 32;

    // FP32 => 4 bytes
    constexpr uint32_t tile_size_bytes = 4 * elements_per_tile;

    // will compute 0.7 * 1.0 / 0.3 == 0.7 * 3.333 == 2.333
    std::vector<float> in0_vector(elements_per_tile, 0.3f);
    std::vector<float> in1_vector(elements_per_tile, 0.7f);
    std::vector<float> result_vec(elements_per_tile, 5.0f);

    printf("\nAllocated initial vector:\n");
    print_first_tile(result_vec);

    constexpr uint32_t buffer_size = tile_size_bytes * n_tiles;

    InterleavedBufferConfig dram_config {
        .device = device,
        .size = buffer_size,
        .page_size = buffer_size,
        .buffer_type = BufferType::DRAM
    };

    // Allocate an input DRAM buffers and CBs
    std::shared_ptr<Buffer> in0_dram_buffer = CreateBuffer(dram_config);
    constexpr uint32_t cb_in0 = CBIndex::c_0;

    CircularBufferConfig cb_in0_config = CircularBufferConfig(
        buffer_size, {{cb_in0, DataFormat::Float32}}
    ).set_page_size(cb_in0, buffer_size);

    CreateCircularBuffer(program, core, cb_in0_config);

    // second
    std::shared_ptr<Buffer> in1_dram_buffer = CreateBuffer(dram_config);
    constexpr uint32_t cb_in1 = CBIndex::c_1;

    CircularBufferConfig cb_in1_config = CircularBufferConfig(
        buffer_size, {{cb_in1, DataFormat::Float32}}
    ).set_page_size(cb_in1, buffer_size);

    CreateCircularBuffer(program, core, cb_in1_config);

    // intermediate CB
    constexpr uint32_t cb_intermediate = CBIndex::c_2;
    CircularBufferConfig cb_intermediate_config = CircularBufferConfig(
        buffer_size, {{cb_intermediate, DataFormat::Float32}}
    ).set_page_size(cb_intermediate, buffer_size);

    CreateCircularBuffer(program, core, cb_intermediate_config);

    // Do the same for the result data 
    std::shared_ptr<Buffer> dst_dram_buffer = CreateBuffer(dram_config);
    constexpr uint32_t cb_out = CBIndex::c_16;

    CircularBufferConfig cb_out_config = CircularBufferConfig(
        buffer_size, {{cb_out, DataFormat::Float32}}
    ).set_page_size(cb_out, buffer_size);

    CreateCircularBuffer(program, core, cb_out_config);

    // Move data from host to device DRAM, blocking
    EnqueueWriteBuffer(cq, in0_dram_buffer, in0_vector, true);
    EnqueueWriteBuffer(cq, in1_dram_buffer, in1_vector, true);

    KernelHandle reader = CreateKernel(
        program,
        "reader.cpp",
        core,
        DataMovementConfig {
            .processor = DataMovementProcessor::RISCV_1,
            .noc = NOC::RISCV_1_default,
            .compile_args = {}
        }
    );

    KernelHandle compute = CreateKernel(
        program,
        "compute.cpp",
        core,
        ComputeConfig {
            .math_fidelity = MathFidelity::LoFi,
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
    SetRuntimeArgs(program, reader, core, {in0_dram_buffer->address(), in1_dram_buffer->address(), n_tiles});
    SetRuntimeArgs(program, writer, core, {dst_dram_buffer->address(), n_tiles});

    EnqueueProgram(cq, program, false);
    Finish(cq);

    EnqueueReadBuffer(cq, dst_dram_buffer, result_vec, true);

    CloseDevice(device);

    printf("Expected values: 2.3f\n");
    print_first_tile(result_vec);

    return 0;
}

