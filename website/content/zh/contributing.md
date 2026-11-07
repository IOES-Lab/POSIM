# 参与 POSIM 开发

欢迎改进代码和文档。请从具体、可复现的小改动开始，并保持模型、控制器配置和指南一致。

## 在分支中工作

{{CODE:contributing:0}}

修改仿真功能时，请使用合适的[安装环境](requirements.md)。不要将生成的构建目录、安装目录或本地凭据加入提交。

## 验证修改后的行为

选择相关的最小示例，检查资源解析、模型生成、仿真时间推进、数据接收、必要的控制器行为，以及正常退出。记录源码修订版本、渲染器、架构和执行命令。

传感器或动力学修改需要合适的数值对比。截图只能展示外观，不能证明物理模型的准确性。修改继承代码时，请保留原作者及来源声明。

## 更新全部八种语言

网站支持英语、韩语、西班牙语、法语、德语、意大利语、日语和中文。英语位于 `/`，其他语言使用 `/ko/`、`/es/`、`/fr/`、`/de/`、`/it/`、`/ja/` 和 `/zh/`。

请同步更新 `website/content/<语言代码>` 中对应的文件。命令、单位、前提条件和限制必须一致，文字则应使用自然表达和通行的机器人领域术语。共享的 `CODE` 与 `MEDIA` 占位符可复用可执行示例和图片。每种语言都必须具备完整页面，不会用英文替代缺失的正文。

修改公开功能时：

- 将[功能目录](features.md)和使用指南与代码一起更新。
- 说明安装与启用方法、默认值、单位、输入输出、配置作用范围、可观察的示例及模型限制。也要覆盖可选效果和适配器。
- 在 `website/scripts/pages.mjs` 中登记新页面，并在 `website/i18n/<语言代码>.json` 中添加标题。同时更新各语言 README。

各语言共用页面顺序，世界与物体目录从源码生成。请描述当前产品。内部笔记、计划和验证报告不要加入仓库；改动讨论与验证结果放在 Issue 和 Pull Request 中。

## 检查网站

准备 Node.js 24 和 pnpm 10.11.0，然后在 `website` 中运行：

{{CODE:contributing:1}}

打开 `http://127.0.0.1:4174`，检查链接、命令复制、搜索、语言切换和移动端导航。发布与维护说明见 [website/README.md](https://github.com/IOES-Lab/POSIM/blob/main/website/README.md)。

## 提交 Pull Request

说明具体问题、修改后的行为和已执行的验证。仿真修改应附相关命令与日志；布局修改应附桌面端和移动端视图。请区分已验证与尚未验证的内容。

可复现的问题通过 [GitHub Issues](https://github.com/IOES-Lab/POSIM/issues)报告，代码改动通过 [Pull Requests](https://github.com/IOES-Lab/POSIM/pulls)审查。项目由 [Korea Maritime & Ocean University 的 IOES-Lab](https://lab.wschoi.com)维护。
