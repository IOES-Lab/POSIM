# 声呐配置与调优

先选择计算后端，再比较覆盖范围、细节和更新频率。[多波束声呐参考](sonar.md)提供快速入门、SDF 和参数表。

## 选择运行环境

使用[准备好的 POSIM 终端](terminals.md)，在同一个 Ubuntu、WSL 或 Docker 环境中编辑、编译和运行。`POSIM_WS` 是该环境的工作空间路径。所有计算后端都需要 Gazebo 渲染传感器。

| 环境 | 计算方式与条件 |
| --- | --- |
| 原生 Linux | CPU；具有可用 Vulkan 适配器的 WGPU；具有兼容 NVIDIA GPU 和工具包的 CUDA |
| WSL 2 或 Linux 容器 | CPU；GPU 计算要求在该环境**内部**能够访问适配器和驱动程序 |
| macOS Docker/RDP | 从 CPU 开始。Linux 容器不提供 Mac 的 Metal 设备。 |
| 原生 macOS | 单独配置原生 ROS/Gazebo 工具链后，WGPU 可使用 Metal；[macOS 安装指南](install-macos.md)采用 Docker/RDP。 |

`auto` 按 CUDA → WGPU → CPU 的顺序选择。进行对照实验时，请明确指定 `cpu`、`wgpu` 或 `cuda`。指定的后端不可用时，传感器初始化失败。请参阅[选项表](sonar.md#compute-backend)。

## 编译声呐包 {#build-the-sonar-packages}

先完成[源码安装](install.md)及其依赖安装。即使使用 CPU，`wgpu_vendor` 也会编译 Rust 库。检查 Rust 和 Cargo；若未安装，请按 [Rust 官方说明](https://www.rust-lang.org/tools/install)操作。

{{CODE:sonar-tuning:0}}

在已加载环境的工作空间中编译库、传感器、世界系统和示例：

{{CODE:sonar-tuning:1}}

CUDA 是可选项。若要跳过已安装的 CUDA 工具包，仅编译 CPU/WGPU，请追加 `--cmake-args -DCMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit=ON`。Rust 依赖版本由 `Cargo.lock` 固定，首次下载需要联网。

{{CODE:sonar-tuning:2}}

确认四个包均可找到，然后运行[声呐示例](sonar.md#run-a-sonar-example)，接收图像和原始数据。包出现在列表中并不代表传感器已成功加载。请在容器内部检查；缺少包时从源码编译，不要仅凭镜像标签判断是否包含。

## 选择 WGPU 适配器

在 Linux 中，请在运行 Gazebo 的环境内执行 `vulkaninfo --summary`，并准备系统对应的 Vulkan 驱动和工具。llvmpipe 等软件适配器能执行 Vulkan，但不提供硬件加速。WGPU 启动输出会显示所选适配器。

{{CODE:sonar-tuning:3}}

| 环境变量 | 用途 |
| --- | --- |
| `WGPU_BACKEND` | Linux 使用 `vulkan`，原生 macOS 进程使用 `metal`；省略时使用平台默认值 |
| `WGPU_ADAPTER_NAME` | 选择名称包含指定文本的第一个适配器，不区分大小写；使用驱动或启动输出中的名称 |
| `WGPU_POWER_PREF` | `high`、`low` 或 `none`；未指定适配器名称时默认为 `high` |

在启动前设置变量。初始化状态在进程内共享，因此修改后应停止并重新启动仿真。[wgpu 项目](https://github.com/gfx-rs/wgpu)介绍了相关 API。若初始化或资源检查失败，请查看输出中的适配器和限制，然后选择 `cpu` 或减小传感器规模。

## 按需配置 CUDA

需要 NVIDIA 驱动、CUDA 编译器和运行时、cuFFT、cuBLAS。请在实际运行环境中检查：

{{CODE:sonar-tuning:4}}

Linux 容器需在主机配置 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)。Windows 请遵循 [CUDA on WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html)，使用 Docker 时还需查看 [Docker Desktop GPU 配置](https://docs.docker.com/desktop/features/gpu/)。显示驱动应安装在 Windows 中，不要在 WSL 内安装 Linux 显示驱动。容器内部也需要 CUDA 库。

选择 GPU 与工具包共同支持的架构。下例为 SM 75，请把 `75` 换成自己 GPU 的值。`OFF` 会重新启用此前 CPU/WGPU 专用编译关闭的工具包检测。

{{CODE:sonar-tuning:5}}

`nvcc --list-gpu-code` 列出支持的目标。不指定架构时采用编译器默认值。若有多个工具包，用 `-DCUDAToolkit_ROOT=/path/to/cuda` 选择，并将相应 `bin` 加入 `PATH`。用 `compute_backend:=cuda` 运行示例。

## 测量基准结果

固定场景、目标距离和传感器位姿。在终端 B 中测量图像接收频率：

{{CODE:sonar-tuning:6}}

按 Ctrl+C 停止，再查看仿真速度：

{{CODE:sonar-tuning:7}}

世界实体名为 `default`。记录接收频率及 `real_time_factor`。仿真时间中请求的 30 Hz，并不保证实际时间每秒收到 30 帧。按 Ctrl+C 退出。

## 调整采样与距离

复制[传感器 SDF](sonar.md#sensor-configuration)，每次只改一个参数，重新编译并加载环境，然后以相同位姿运行。

| 实验 | 对比 | 观察内容 |
|---|---|---|
| 角向细节 | 视场角不变，512 与 256 条水平波束 | 相邻目标的分离及接收频率 |
| 垂直采样 | 300 与 150 条射线 | 目标连续性和计算时间 |
| 覆盖范围 | 同时调整射线范围与 `maxDistance` | 最远目标及距离轴刻度 |
| 更新频率 | 30 与 10 Hz | 接收频率和实时因子 |
| 射线跳采样 | `raySkips` 为 10 与 5 | 垂直细节和计算开销 |

距离间隔为 `soundSpeed / (2 × bandwidth)`，网格单元数为 `ceil(maxDistance / 间隔) + 1`。1500 m/s、29.9 kHz、15 m 对应 599 个单元。最后一个坐标可能略大于 `maxDistance`，但超出设定范围的回波会被排除。请从原始消息或 CSV 中读取实际距离数组和波束数。

WGPU 保留请求的变换长度，包括非 2 的幂长度，并自动选择 FFT 阶段。增大距离和带宽会增加内存与计算量。在默认网格模式下，CPU 将回波分配到最近的距离单元；GPU 在频域中处理。比较后端时，还应分别测量幅值和距离响应。

## 记录可复现的对比

等待渲染稳定后再测量。测试吞吐量时关闭 `writeLog` 和 `debugFlag`，使用 [CSV](sonar.md) 或 [ROS](visualization.md) 另行记录数据。`blazingSonarImage=false` 将随机种子固定为 1234。

保存 SDF、POSIM 版本、后端与适配器、驱动和工具包版本、每帧耗时、接收频率、实时因子及内存用量。除了截图，还应比较原始强度与距离数据。研究模型来源见[引用说明](citation.md)。

## GPU 处理流程

{{MEDIA:sonar-tuning}}

处理包括射线求和、加窗与校正、FFT 等阶段。请分别测量计算与内存传输耗时。图片来源：POSIM Notion Wiki。

## 原生 macOS 的 ROS 传输设置 {#native-macos-ros-transport}

在原生 macOS 上使用 Fast DDS 时，如果大型 best-effort 声呐消息丢失，可使用已安装的本地配置文件。该配置使用 16 MiB 共享内存段和 1 MiB 传输消息缓冲区；localhost UDP 使用 8192 字节数据报及 2 MiB 发送和接收套接字缓冲区。启动仿真器、桥接程序和订阅者之前，请在各自的终端中设置：

{{CODE:sonar-tuning:8}}

此配置为可选项，仅用于同一台 Mac 上的进程间通信。与其他计算机通信时，请使用适合该网络的配置文件。如果已设置 `FASTDDS_DEFAULT_PROFILES_FILE`，请将共享内存设置合并到现有配置中。这些 ROS 传输设置同时适用于 CPU 和 WGPU 声呐。
