```mermaid
graph TD
    %% 节点定义
    A[cuInit(0): 初始化CUDA驱动]
    B[cuDeviceGet(): 获取设备]
    C[cuCtxCreate(): 创建CUDA上下文]
    D[cuvidCreateVideoParser(): 创建视频解析器]
    E[设置回调函数]
    E1[pfnSequenceCallback: 处理视频序列头]
    E2[pfnDecodePicture: 处理解码请求]
    E3[pfnDisplayPicture: 处理解码完成帧]
    F[序列回调中: cuvidCreateDecoder() 创建解码器]
    G[解码回调中: cuvidDecodePicture() 解码帧]
    H[显示回调中: cuvidMapVideoFrame() 访问解码帧]
    I[显示回调中: cuvidUnmapVideoFrame() 释放帧资源]
    J[循环输入视频数据]
    K[cuvidParseVideoData(): 解析数据]
    L[使用CUVIDSOURCEDATAPACKET结构传递数据]
    M[cuvidDestroyVideoParser(): 销毁解析器]
    N[cuvidDestroyDecoder(): 销毁解码器]
    O[cuCtxDestroy(): 销毁CUDA上下文]

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