// Copyright (c) 2018 Łukassz Kurowski <crackcomm@gmail.com>

#ifndef DYNAMIC_ARGON2_GPU
#define DYNAMIC_ARGON2_GPU

#define ARGON2_GPU_CUDA cuda
#define ARGON2_GPU_OPENCL opencl

#define ARGON2_GPU ARGON2_GPU_CUDA

// TODO: configure
#define HAVE_CUDA 1

#include "argon2-cuda/cuda-exception.h"
#include "argon2-cuda/processing-unit.h"
#include "argon2-gpu/common.h"
#include "argon2-opencl/processing-unit.h"

using Argon2GPUParams = argon2::Argon2Params;

#if ARGON2_GPU == ARGON2_GPU_CUDA
using Argon2GPU = argon2::cuda::ProcessingUnit;
using Argon2GPUDevice = argon2::cuda::Device;
using Argon2GPUContext = argon2::cuda::GlobalContext;
using Argon2GPUProgramContext = argon2::cuda::ProgramContext;
#elif ARGON2_GPU == ARGON2_GPU_OPENCL
using Argon2GPU = argon2::opencl::ProcessingUnit;
using Argon2GPUDevice = argon2::opencl::Device;
using Argon2GPUContext = argon2::opencl::GlobalContext;
using Argon2GPUProgramContext = argon2::opencl::ProgramContext;
#endif

static const std::vector<Argon2GPUDevice>& GetGPUDevices()
{
    static Argon2GPUContext global;
    return global.getAllDevices();
}

static Argon2GPU GetGPUProcessingUnit(const std::size_t& deviceIndex)
{
    Argon2GPUContext global;
    auto& devices = global.getAllDevices();
    auto& device = devices[deviceIndex];
    Argon2GPUProgramContext context(&global, {device}, argon2::ARGON2_D, argon2::ARGON2_VERSION_10);
    Argon2GPUParams params((std::size_t)OUTPUT_BYTES, 2, 500, 8);
    Argon2GPU processingUnit(&context, &params, &device, 1, false, false);
    return processingUnit;
}

template <class ProcessingUnit>
inline uint256 GetBlockHashGPU(const CBlockHeader& block, const ProcessingUnit& pu) const
{
    static unsigned char pblank[1];
    const auto pBegin = BEGIN(block->nVersion);
    const auto pEnd = END(block->nNonce);
    const void* input = (pBegin == pEnd ? pblank : static_cast<const void*>(&pBegin[0]));

    uint256 hashResult;
    pu->setInputAndSalt(0, (const void*)input, INPUT_BYTES);
    pu->beginProcessing();
    pu->endProcessing();
    pu->getHash(0, (uint8_t*)&hashResult);
    return hashResult;

}

#endif // DYNAMIC_ARGON2_GPU
