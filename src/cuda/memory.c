#include <dirent.h>
#include <time.h>

#include "allocator/allocator.h"
#include "include/libcuda_hook.h"
#include "include/memory_limit.h"

extern int pidfound;

const size_t cuarray_format_bytes[33] = {
    0,  // 0x00
    1,  // CU_AD_FORMAT_UNSIGNED_INT8 = 0x01
    2,  // CU_AD_FORMAT_UNSIGNED_INT16 = 0x02
    4,  // CU_AD_FORMAT_UNSIGNED_INT32 = 0x03
    0,  // 0x04
    0,  // 0x05
    0,  // 0x06
    0,  // 0x07
    1,  // CU_AD_FORMAT_SIGNED_INT8 = 0x08
    2,  // CU_AD_FORMAT_SIGNED_INT16 = 0x09
    4,  // CU_AD_FORMAT_SIGNED_INT32 = 0x0a
    0,  // 0x0b
    0,  // 0x0c
    0,  // 0x0d
    0,  // 0x0e
    0,  // 0x0f
    2,  // CU_AD_FORMAT_HALF = 0x10
    0,  // 0x11
    0,  // 0x12
    0,  // 0x13
    0,  // 0x14
    0,  // 0x15
    0,  // 0x16
    0,  // 0x17
    0,  // 0x18
    0,  // 0x19
    0,  // 0x1a
    0,  // 0x1b
    0,  // 0x1c
    0,  // 0x1d
    0,  // 0x1e
    0,  // 0x1f       
    4   // CU_AD_FORMAT_FLOAT = 0x20
};

extern size_t round_up(size_t size,size_t align);
extern void rate_limiter(int grids, int blocks);

int check_oom() {
//    return 0;
    CUdevice dev;
    CHECK_DRV_API(cuCtxGetDevice(&dev));
    return oom_check(dev,0);
}

uint64_t compute_3d_array_alloc_bytes(const CUDA_ARRAY3D_DESCRIPTOR* desc) {
    if (desc==NULL) {
        LOG_WARN("compute_3d_array_alloc_bytes desc is null");
    }else{
        LOG_DEBUG("compute_3d_array_alloc_bytes height=%ld width=%ld",desc->Height,desc->Width);
    }
    uint64_t bytes = desc->Width * desc->NumChannels;
    if (desc->Height != 0) {
        bytes *= desc->Height;
    }
    if (desc->Depth != 0) {
        bytes *= desc->Depth;
    }
    bytes *= cuarray_format_bytes[desc->Format];

    // TODO: take account of alignment and etc
    // bytes ++ ???
    return bytes;
}


uint64_t compute_array_alloc_bytes(const CUDA_ARRAY_DESCRIPTOR* desc) {
    if (desc==NULL) {
        LOG_WARN("compute_array_alloc_bytes desc is null");
    }else{
        LOG_DEBUG("compute_array_alloc_bytes height=%ld width=%ld",desc->Height,desc->Width);
    }

    uint64_t bytes = desc->Width * desc->NumChannels;
    if (desc->Height != 0) {
        bytes *= desc->Height;
    }
    bytes *= cuarray_format_bytes[desc->Format];

    // TODO: take account of alignment and etc
    // bytes ++ ???
    return bytes;
}





CUresult cuMemoryAllocate(CUdeviceptr* dptr, size_t bytesize, size_t* bytesallocated,void* data){
    CUresult res;
    if (bytesallocated!=NULL)
        *bytesallocated = bytesize;
    res = cuMemAllocManaged(dptr,bytesize,CU_MEM_ATTACH_GLOBAL);
    // res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAlloc_v2,dptr,bytesize);
    return res;
}
CUresult cuMemoryFree(CUdeviceptr dptr) {
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemFree_v2,dptr);
    return res;
}


CUresult cuMemAlloc_v2(CUdeviceptr* dptr, size_t bytesize) {
    LOG_INFO("into cuMemAllocing_v2 dptr=%p bytesize=%ld",dptr,bytesize);
    ENSURE_RUNNING();
    // CUresult res = allocate_raw(dptr,bytesize);
    CUresult res = cuMemAllocManaged(dptr,bytesize,CU_MEM_ATTACH_GLOBAL);
    if (res!=CUDA_SUCCESS)
        return res;
    LOG_INFO("res=%d, cuMemAlloc_v2 success dptr=%p bytesize=%lu",0,(void *)*dptr,bytesize);
    return CUDA_SUCCESS;
}


CUresult cuMemAllocManaged(CUdeviceptr* dptr, size_t bytesize, unsigned int flags) {
    LOG_DEBUG("cuMemAllocManaged dptr=%p bytesize=%ld",dptr,bytesize);
    ENSURE_RUNNING();
    CUdevice dev;
    cuCtxGetDevice(&dev);
    if (oom_check(dev,bytesize)){
        return CUDA_ERROR_OUT_OF_MEMORY;
    }
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAllocManaged, dptr, bytesize, flags);
    if (res == CUDA_SUCCESS) {
        add_chunk_only(*dptr,bytesize);
    }
    return res;
}

CUresult cuMemAllocPitch_v2(CUdeviceptr* dptr, size_t* pPitch, size_t WidthInBytes, 
                                      size_t Height, unsigned int ElementSizeBytes) {
    LOG_DEBUG("cuMemAllocPitch_v2 dptr=%p (%ld,%ld)",dptr,WidthInBytes,Height);
    size_t guess_pitch = (((WidthInBytes - 1) / ElementSizeBytes) + 1) * ElementSizeBytes;
    size_t bytesize = guess_pitch * Height;
    ENSURE_RUNNING();
    CUdevice dev;
    cuCtxGetDevice(&dev);
    if (oom_check(dev,bytesize)){
        return CUDA_ERROR_OUT_OF_MEMORY;
    }
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAllocPitch_v2, dptr, pPitch, WidthInBytes, Height, ElementSizeBytes);
    if (res == CUDA_SUCCESS) {
        add_chunk_only(*dptr,bytesize);
    }
    return res;
}

CUresult cuMemFree_v2(CUdeviceptr dptr) {
    LOG_DEBUG("cuMemFree_v2 dptr=%llx",dptr);
    if (dptr == 0) {  // NULL
        return CUDA_SUCCESS;
    }
    CUresult res = free_raw(dptr);
    LOG_INFO("after free_raw dptr=%p res=%d",(void *)dptr,res);
    return res;
}

CUresult cuPointerGetAttributes ( unsigned int  numAttributes, CUpointer_attribute* attributes, void** data, CUdeviceptr ptr ) {
    LOG_DEBUG("cuPointGetAttribute data=%p ptr=%llx", data, ptr);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuPointerGetAttributes,numAttributes,attributes,data,ptr);
    int cur=0;
    for (cur=0;cur<numAttributes;cur++){
        if (attributes[cur]==CU_POINTER_ATTRIBUTE_MEMORY_TYPE){
            int j = check_memory_type(ptr);
            //*(int *)(data[cur])=1;
            LOG_DEBUG("check result = %d %d",j,*(int *)(data[cur]));
        }else{
            if (attributes[cur]==CU_POINTER_ATTRIBUTE_IS_MANAGED){
                *(int *)(data[cur])=0;    
            }
        }
    }
    return res;
}
#ifdef HOOK_MEMINFO_ENABLE
CUresult cuMemGetInfo_v2(size_t* free, size_t* total) {
    CUdevice dev;
    LOG_DEBUG("cuMemGetInfo_v2");
    ENSURE_INITIALIZED();
    CHECK_DRV_API(cuCtxGetDevice(&dev));
    size_t usage = get_current_device_memory_usage(cuda_to_nvml_map(dev));
    size_t limit = get_current_device_memory_limit(cuda_to_nvml_map(dev));
    if (limit == 0) {
        CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemGetInfo_v2, free, total);
        LOG_INFO("orig free=%ld total=%ld", *free, *total);
        *free = *total - usage;
        LOG_INFO("after free=%ld total=%ld", *free, *total);
        return CUDA_SUCCESS;
    } else if (limit < usage) {
        LOG_WARN("limit < usage; usage=%ld, limit=%ld", usage, limit);
        return CUDA_ERROR_INVALID_VALUE;
    } else {
        CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemGetInfo_v2, free, total);
        LOG_INFO("orig free=%ld total=%ld limit=%ld usage=%ld",
            *free, *total, limit, usage);
        // Ensure total memory does not exceed the physical or imposed limit.
        size_t actual_limit = (limit > *total) ? *total : limit;
        *free = (actual_limit > usage) ? (actual_limit - usage) : 0;
        *total = actual_limit;
        LOG_INFO("after free=%ld total=%ld limit=%ld usage=%ld",
            *free, *total, limit, usage);
        return CUDA_SUCCESS;
    }
}
#endif

CUresult cuMemAllocAsync(CUdeviceptr *dptr, size_t bytesize, CUstream hStream) {
    LOG_DEBUG("cuMemAllocAsync:%ld",bytesize);
    return allocate_async_raw(dptr,bytesize,hStream);
}

CUresult cuMemFreeAsync(CUdeviceptr dptr, CUstream hStream) {
    LOG_DEBUG("cuMemFreeAsync dptr=%llx",dptr);
    if (dptr == 0) {  // NULL
        return CUDA_SUCCESS;
    }
    CUresult res = free_raw_async(dptr,hStream);
    LOG_DEBUG("after free_raw_async dptr=%p res=%d",(void *)dptr,res);
    return res;
}
