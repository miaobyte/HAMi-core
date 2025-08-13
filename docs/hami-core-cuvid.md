一.cuvid 的函数列表

``` shell
libvgpu#ldconfig -p | grep libnvcuvid
        libnvcuvid.so.1 (libc6,x86-64) => /lib/x86_64-linux-gnu/libnvcuvid.so.1

```

```
libvgpu#nm -D /lib/x86_64-linux-gnu/libnvcuvid.so.1 | cut -c 17- | grep -E '(Nv|cuvid)'
 T NvToolCreateInterface
 T NvToolDestroyInterface
 T NvToolGetApiFunctionCount
 T NvToolGetApiID
 T NvToolGetApiNames
 T NvToolGetInterface
 T NvToolSetApiID
 T NvToolSetInterface
 T cuvidConvertYUVToRGB
 T cuvidConvertYUVToRGBArray
 T cuvidCreateDecoder
 T cuvidCreateVideoParser
 T cuvidCreateVideoSource
 T cuvidCreateVideoSourceW
 T cuvidCtxLock
 T cuvidCtxLockCreate
 T cuvidCtxLockDestroy
 T cuvidCtxUnlock
 T cuvidDecodePicture
 T cuvidDestroyDecoder
 T cuvidDestroyVideoParser
 T cuvidDestroyVideoSource
 T cuvidGetDecodeStatus
 T cuvidGetDecoderCaps
 T cuvidGetSourceAudioFormat
 T cuvidGetSourceVideoFormat
 T cuvidGetVideoSourceState
 T cuvidMapVideoFrame
 T cuvidMapVideoFrame64
 T cuvidParseVideoData
 T cuvidPrivateOp
 T cuvidReconfigureDecoder
 T cuvidSetVideoSourceState
 T cuvidUnmapVideoFrame
 T cuvidUnmapVideoFrame64
```

```mermaid
flowchart TD
    %% CUDA 初始化阶段
    A["cuInit(0): 初始化CUDA驱动"]
    B["cuDeviceGet(): 获取设备"]
    C["cuCtxCreate(): 创建CUDA上下文"]

    %% 视频解析器创建与回调设置
    D["cuvidCreateVideoParser(): 创建视频解析器"]
    E["设置CUVIDPARSERPARAMS回调函数"]
    E1["pfnSequenceCallback: 处理视频序列头"]
    E2["pfnDecodePicture: 处理解码请求"]
    E3["pfnDisplayPicture: 处理解码完成帧"]

    %% 解码器创建与解码流程
    F["序列回调: cuvidCreateDecoder() 创建解码器"]
    G["解码回调: cuvidDecodePicture() 解码帧"]
    H["显示回调: cuvidMapVideoFrame() 访问解码帧"]
    I["显示回调: cuvidUnmapVideoFrame() 释放帧资源"]

    %% 数据输入与解析
    J["循环输入视频数据"]
    K["cuvidParseVideoData(): 解析数据"]
    L["CUVIDSOURCEDATAPACKET: 数据包结构传递"]

    %% 资源销毁阶段
    M["cuvidDestroyVideoParser(): 销毁解析器"]
    N["cuvidDestroyDecoder(): 销毁解码器"]
    O["cuCtxDestroy(): 销毁CUDA上下文"]

    %% 流程连接
    A --> B
    B --> C
    C --> D
    D --> E
    E --> E1
    E --> E2
    E --> E3
    E1 --> F
    F --> G
    G --> H
    H --> I
    I --> J
    J --> K
    K --> L
    L --> F
    J --> M
    M --> N
    N --> O
```