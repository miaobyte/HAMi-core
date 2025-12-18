#ifndef __LIBCUDA_HOOK_H__
#define __LIBCUDA_HOOK_H__

#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#define NVML_NO_UNVERSIONED_FUNC_DEFS
#include <cuda.h>
#include <pthread.h>
#include "include/log_utils.h"

typedef struct {
  void *fn_ptr;
  char *name;
} cuda_entry_t;

#define FILENAME_MAX 4096

#define CONTEXT_SIZE 104857600

typedef CUresult (*cuda_sym_t)();

#define CUDA_OVERRIDE_ENUM(x) OVERRIDE_##x

#define CUDA_FIND_ENTRY(table, sym) ({ (table)[CUDA_OVERRIDE_ENUM(sym)].fn_ptr; })

#define CUDA_OVERRIDE_CALL(table, sym, ...)                                    \
  ({    \
    LOG_DEBUG("Hijacking %s", #sym);                                           \
    cuda_sym_t _entry = (cuda_sym_t)CUDA_FIND_ENTRY(table, sym);               \
    if (_entry == NULL) {                                                      \
      LOG_ERROR("Hijack failed: %s is NULL", #sym);                            \
    }                                                                          \
    _entry(__VA_ARGS__);                                                       \
  })

typedef enum {
    /* cuInit Part */
    CUDA_OVERRIDE_ENUM(cuInit),
    /* cuDeivce Part */
    CUDA_OVERRIDE_ENUM(cuDeviceTotalMem_v2),
    CUDA_OVERRIDE_ENUM(cuDriverGetVersion),
    /* cuContext Part */
    CUDA_OVERRIDE_ENUM(cuDevicePrimaryCtxRetain),
    CUDA_OVERRIDE_ENUM(cuDevicePrimaryCtxRelease_v2),
    /* cuMemory Part */
    CUDA_OVERRIDE_ENUM(cuMemAlloc_v2),
    CUDA_OVERRIDE_ENUM(cuMemAllocManaged),
    CUDA_OVERRIDE_ENUM(cuMemAllocPitch_v2),
    CUDA_OVERRIDE_ENUM(cuMemFree_v2),
    CUDA_OVERRIDE_ENUM(cuMemGetInfo_v2),
    CUDA_OVERRIDE_ENUM(cuPointerGetAttributes),
    //  Launch Kernel
    // CUDA_OVERRIDE_ENUM(cuLaunchKernel),
    // CUDA_OVERRIDE_ENUM(cuLaunchCooperativeKernel),
    /* Virtual Memory Part */
    CUDA_OVERRIDE_ENUM(cuMemAllocAsync),
    CUDA_OVERRIDE_ENUM(cuMemFreeAsync),
    /* cuGetProcAddress Part */
    CUDA_OVERRIDE_ENUM(cuGetProcAddress),
    CUDA_OVERRIDE_ENUM(cuGetProcAddress_v2),
    CUDA_ENTRY_END
}cuda_override_enum_t;

extern cuda_entry_t cuda_library_entry[];

#endif

#undef cuGetProcAddress
CUresult cuGetProcAddress( const char* symbol, void** pfn, int  cudaVersion, cuuint64_t flags );
#undef cuGraphInstantiate
CUresult cuGraphInstantiate(CUgraphExec *phGraphExec, CUgraph hGraph, CUgraphNode *phErrorNode, char *logBuffer, size_t bufferSize);

