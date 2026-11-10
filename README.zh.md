# POSIM — Platform for Ocean Simulation

[English](README.md) · [Español](README.es.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [Italiano](README.it.md) · [한국어](README.ko.md) · [日本語](README.ja.md) · [中文](README.zh.md)

[![构建并发布POSIM Docker镜像](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml)

POSIM是由IOES-Lab维护的开源海洋机器人仿真库。
通过Gazebo Sim，可以搭建海洋环境，运行水下和水面机器人，并使用仿真传感器和ROS 2控制接口。

## 文档与入门

**[阅读中文文档](https://ioes-lab.github.io/POSIM/zh/)**

POSIM使用ROS 2 Lyrical和Gazebo Sim Jetty。请根据操作系统选择安装方式：

- **Ubuntu 26.04：** 推荐[原生安装](https://ioes-lab.github.io/POSIM/zh/install.html)。在Linux上，也可以选择[通过Docker安装](https://ioes-lab.github.io/POSIM/zh/install-ubuntu-docker.html)。
- **Windows：** [WSL](https://ioes-lab.github.io/POSIM/zh/install-windows.html)或[WSL + Docker](https://ioes-lab.github.io/POSIM/zh/install-windows-docker.html)。
- **macOS：** [Docker + RDP](https://ioes-lab.github.io/POSIM/zh/install-macos.html)。

在[选择运行环境](https://ioes-lab.github.io/POSIM/zh/requirements.html)中比较各条安装路径，
然后[准备终端](https://ioes-lab.github.io/POSIM/zh/terminals.html)，
运行[第一次仿真](https://ioes-lab.github.io/POSIM/zh/quickstart.html)。

[功能与示例](https://ioes-lab.github.io/POSIM/zh/features.html)汇集了机器人、仿真世界、传感器和控制的使用指南，
也包括[水下渲染效果与海洋雪](https://ioes-lab.github.io/POSIM/zh/underwater-rendering.html)
和[海洋环境中的GNSS](https://ioes-lab.github.io/POSIM/zh/gnss.html)。

## Docker镜像

下载当前镜像：

```bash
docker pull ioeslab/posim:latest
```

Docker会自动选择与主机匹配的AMD64或ARM64镜像。如果需要使用同一版本复现实验，
请从[Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags)选择`sha-<12-character-commit>`格式的标签，
其中包含提交ID的前12个字符。
创建容器前，请参阅[选择运行环境](https://ioes-lab.github.io/POSIM/zh/requirements.html)，
完成Linux、Windows WSL或macOS对应的图形显示配置。

## 相关应用

[WWW-POSIM](https://www-posim.vercel.app/)将POSIM与地理地形、路径规划和机器人控制库相结合，
提供图形界面以及网页端和桌面端应用。应用的具体操作方法见WWW-POSIM文档。

## 参与贡献

欢迎在[IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM)报告问题或提交改进。
[贡献指南](https://ioes-lab.github.io/POSIM/zh/contributing.html)介绍了开发时需要执行的检查和文档更新方法。

## 许可证与来源

POSIM包含[Project DAVE](https://github.com/Field-Robotics-Lab/dave)的源代码，并保留其作者及版权声明。
POSIM自行编写的代码采用[Apache License 2.0](LICENSE)。第三方代码和资源仍遵循各自原有的条款。
外部依赖Wave Sim从其上游仓库构建，适用GPL条款。
详情请参阅[Wave Sim的许可证与分发说明](extras/surface/README.md#licensing)。
