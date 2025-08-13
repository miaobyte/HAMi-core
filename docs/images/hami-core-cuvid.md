

``` mermaid
graph TD
    subgraph 1. 初始化阶段
        A[cuInit(0): 初始化CUDA驱动] --> B[cuDeviceGet(): 获取设备]
        B --> C[cuCtxCreate(): 创建CUDA上下文]
    end

    subgraph 2. 创建解析器
        D[cuvidCreateVideoParser(): 创建视频解析器] --> E[设置回调函数]
        E --> E1[pfnSequenceCallback: 处理视频序列头]
        E --> E2[pfnDecodePicture: 处理解码请求]
        E --> E3[pfnDisplayPicture: 处理解码完成帧]
    end

    subgraph 3. 解码流程
        F[序列回调中: cuvidCreateDecoder() 创建解码器] --> G[解码回调中: cuvidDecodePicture() 解码帧]
        G --> H[显示回调中: cuvidMapVideoFrame() 访问解码帧]
        H --> I[显示回调中: cuvidUnmapVideoFrame() 释放帧资源]
    end

    subgraph 4. 数据处理
        J[循环输入视频数据] --> K[cuvidParseVideoData(): 解析数据]
        K --> L[使用CUVIDSOURCEDATAPACKET结构传递数据]
    end

    subgraph 5. 清理阶段
        M[cuvidDestroyVideoParser(): 销毁解析器] --> N[cuvidDestroyDecoder(): 销毁解码器]
        N --> O[cuCtxDestroy(): 销毁CUDA上下文]
    end

    % 流程连接
    C --> D
    E --> F
    I --> J
    L --> F
    J --> M
```