#include "include/libcuda_hook.h"
#include <string.h>
#include "include/libvgpu.h"
#include "include/multi_func_hook.h"


typedef void* (*fp_dlsym)(void*, const char*);
extern fp_dlsym real_dlsym;

cuda_entry_t cuda_library_entry[] = {
    /* Init Part    */ 
    {.name = "cuInit"},
    /* Deivce Part */
    {.name = "cuDeviceTotalMem_v2"},
    {.name = "cuDriverGetVersion"},
    /* Context Part */
    {.name = "cuDevicePrimaryCtxRetain"},
    {.name = "cuDevicePrimaryCtxRelease_v2"},
    /* Memory Part */
    {.name = "cuMemAlloc_v2"},
    {.name = "cuMemAllocManaged"},
    {.name = "cuMemAllocPitch_v2"},
    {.name = "cuMemFree_v2"},
    {.name = "cuMemGetInfo_v2"},
    {.name = "cuPointerGetAttributes"}, 
    //  Launch Kernel
    // {.name = "cuLaunchKernel"},
    // {.name = "cuLaunchCooperativeKernel"},
    {.name = "cuMemAllocAsync"},
    {.name = "cuMemFreeAsync"},
    /* hook */
    {.name = "cuGetProcAddress"},
    {.name = "cuGetProcAddress_v2"},
};

int prior_function(char tmp[500]) {
    char *pos = tmp + strlen(tmp) - 3;
    if (pos[0]=='_' && pos[1]=='v') {
        if (pos[2]=='2')
            pos[0]='\0';
        else
            pos[2]--;
        return 1;
    }
    return 0;
}

void load_cuda_libraries() {
    void *table = NULL;
    int i = 0;
    char cuda_filename[FILENAME_MAX];
    char tmpfunc[500];

    LOG_INFO("Start hijacking");

    snprintf(cuda_filename, FILENAME_MAX - 1, "%s","libcuda.so.1");
    cuda_filename[FILENAME_MAX - 1] = '\0';

    table = dlopen(cuda_filename, RTLD_NOW | RTLD_NODELETE);
    if (!table) {
        LOG_WARN("can't find library %s", cuda_filename);
    }

    for (i = 0; i < CUDA_ENTRY_END; i++) {
        LOG_DEBUG("LOADING %s %d",cuda_library_entry[i].name,i);
        cuda_library_entry[i].fn_ptr = real_dlsym(table, cuda_library_entry[i].name);
        if (!cuda_library_entry[i].fn_ptr) {
            cuda_library_entry[i].fn_ptr=real_dlsym(RTLD_NEXT,cuda_library_entry[i].name);
            if (!cuda_library_entry[i].fn_ptr){
                LOG_INFO("can't find function %s in %s", cuda_library_entry[i].name,cuda_filename);
                memset(tmpfunc,0,500);
                strcpy(tmpfunc,cuda_library_entry[i].name);
                while (prior_function(tmpfunc)) {
                    cuda_library_entry[i].fn_ptr=real_dlsym(RTLD_NEXT,tmpfunc);
                    if (cuda_library_entry[i].fn_ptr) {
                        LOG_INFO("found prior function %s",tmpfunc);
                        break;
                    } 
                }
            }
        }
    }
    LOG_INFO("loaded_cuda_libraries");
    if (cuda_library_entry[0].fn_ptr==NULL){
        LOG_WARN("is NULL");
    }
    dlclose(table);
}