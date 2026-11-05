# 声纳配置与性能调优

根据实验选择覆盖范围、细节程度和更新频率。[多波束声纳参考](sonar.md)提供快速入门、SDF 和参数；本指南介绍配置对比及 CUDA 环境准备。

## 所需环境

使用配有 NVIDIA GPU 的 Ubuntu、受支持的 WSL 2，或能访问 GPU 的容器，并准备[对应的 Linux 终端](terminals.md)。编辑和编译均在该环境内完成，`POSIM_WS` 指向其工作空间。macOS 的 Docker/RDP 配置无法提供 CUDA GPU。请在配备 NVIDIA GPU 的 Linux 或 WSL 2 主机上运行此声纳。

## 建立基准测量

启动 [BlueView P900 示例](sonar.md#run-a-sonar-example)，固定场景、目标距离和传感器位姿。先确认收到图像与原始数据，再测量吞吐量。如果缺少 CUDA，应先完成下方工具链配置。

在另一个准备好的终端中测量接收频率：

{{CODE:sonar-tuning:0}}

取得稳定样本后按 Ctrl+C 停止。单独查看仿真速度：

{{CODE:sonar-tuning:1}}

世界内部名称为 `default`。同时记录 `real_time_factor`：请求 30 Hz 仿真频率，并不保证每个实际秒接收 30 帧。按 Ctrl+C 停止查看。

## 每次对比一个变量

复制[传感器描述](sonar.md#sensor-configuration)，修改一项设置，重新编译并加载环境，再从相同位姿启动。

| 实验 | 对比 | 观察内容 |
|---|---|---|
| 角向细节 | 视场角不变，512 与 256 条水平波束 | 相邻目标的分离及接收频率 |
| 垂直采样 | 300 与 150 条射线 | 目标连续性和计算时间 |
| 覆盖范围 | 同时调整射线范围与 `maxDistance` | 最远目标及距离轴刻度 |
| 更新频率 | 30 与 10 Hz | 接收频率和实时因子 |
| 射线跳采样 | `raySkips` 为 10 与 5 | 垂直细节和计算开销 |

实际测量每组对比结果。测吞吐量时关闭 `writeLog` 和 `debugFlag`，需要分析原始回波或计时时再单独开启。保存 SDF、POSIM 版本、GPU/CUDA 版本和观测结果。[ROS 2](ros.md#visualization-and-recording)介绍话题记录方法。

## 检查工具链

[插件](sonar.md)需要 NVIDIA 驱动、CUDA 编译器和运行时、cuFFT、cuBLAS，以及 Gazebo 渲染。在实际运行声纳的环境中执行：

{{CODE:sonar-tuning:2}}

分别检查 GPU 可见性和编译器可用性。Linux Docker Engine 用户需在主机配置 [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)。Windows 用户按 [Docker Desktop/WSL 2 GPU 指南](https://docs.docker.com/desktop/features/gpu/)配置。容器镜像中也需要 CUDA 库。

## 选择 CUDA 架构

将 `CMAKE_CUDA_ARCHITECTURES` 设为 GPU 和工具链支持的 SM 版本。以下以 SM 75 为例；将 `75` 替换为目标 GPU 的架构，在准备好的工作空间中运行：

{{CODE:sonar-tuning:3}}

`nvcc --list-gpu-code` 列出支持的编译目标。若不明确指定，CMake 使用编译器默认值。存在多个 CUDA 安装时，用 `-DCUDAToolkit_ROOT=/path/to/cuda` 选择，并将其 `bin` 目录加入 `PATH`。

## 检查安装结果

{{CODE:sonar-tuning:4}}

即使 CUDA 编译目标被跳过，功能包也可能已注册。继续检查库文件：

{{CODE:sonar-tuning:5}}

每条命令应返回退出码 0。检查是否存在跳过目标或加载错误，然后按[多波束声纳](sonar.md)接收图像与原始数据。

## 解读性能结果

计时前先让渲染器稳定运行。在相同场景和位姿下，记录每帧处理时间、接收频率、实时因子、CPU/GPU 使用率和内存。区分物理计算、渲染、CUDA 计算、发布和日志开销。

`blazingSonarImage` 用于选择变化的随机种子或固定种子 `1234`，可据此比较重复性。修改数值内核时保留基准数据，除图像外，还应比较强度与距离。原始研究来源见[引用](citation.md)。

## GPU 处理流程

{{MEDIA:sonar-tuning}}

处理包括射线求和、加窗与校正、FFT。性能分析时，应区分各阶段耗时与 GPU 内存传输耗时。图片来源：POSIM Notion Wiki。
