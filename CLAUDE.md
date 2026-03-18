# SimplePicture - 跨平台图片查看应用

## 项目概述
基于 Qt 5 的跨平台图片查看器，支持 Windows/Linux，追求极速启动。

## 技术栈
- C++17
- Qt 5.11+（Windows 使用 5.15.2，Linux 兼容 5.11+）
- CMake 3.16+
- Google Test (单元测试)

## 本地构建 (Windows/MSYS2)
- 工具链：MSYS2 MinGW64 (g++ 15.1.0, Ninja, CMake)
- **关键**：必须先设置 PATH 包含 MinGW64 bin 目录，否则 cc1plus 找不到 libmpfr-6.dll
- 编译命令：`export PATH=/c/msys64/mingw64/bin:$PATH && cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF && cmake --build build --config Release`
- 编译后复制到测试目录：`cp build/SimplePicture.exe installer/dist/`（如进程占用先 `taskkill //F //IM SimplePicture.exe`）
- Qt 插件路径：`/c/msys64/mingw64/share/qt5/plugins/`
- 打包：`/nsis-pack` 或 `bash installer/build_installer.sh`

## 架构约束
- **禁止使用 .ui 文件**：所有界面用 C++ 类直接实现
- **禁止使用 stylesheet**：通过重写 paintEvent 实现自定义绘制
- **跨平台**：Windows + Linux
- **Qt 版本兼容**：使用 `#if QT_VERSION` 宏处理 API 差异

## 核心功能
- 极速启动，打开常见图片格式 (PNG, JPG, BMP, GIF, WEBP, TIFF, ICO, SVG)
- SVG 矢量图无损缩放
- 鼠标滚轮缩放，图片小于窗口时按原始像素大小显示
- 左右方向键切换上一张/下一张图片
- 拖拽文件到窗口打开
- 自动检测文件实际格式（QImageReader::setDecideFormatFromContent）

## 编码规范
- 遵循 C++ Core Guidelines
- 类名: PascalCase (如 ImageViewer)
- 方法名: camelCase (如 loadImage)
- 成员变量: m_ 前缀 (如 m_currentIndex)
- 常量: k 前缀 + PascalCase (如 kMaxZoomLevel)
- 命名空间: simplepic
- 头文件保护: #pragma once
- 使用 smart pointer，避免裸指针

## 目录结构
```
src/
  core/       - 核心逻辑 (图片加载、缓存、导航)
  ui/         - 界面组件 (主窗口、图片视图、更新日志对话框)
  main.cpp    - 入口
tests/        - 单元测试
installer/    - NSIS 安装脚本 (Windows)
debian/       - deb 打包配置 (Linux)
resources/    - 图标、桌面入口文件、Qt 资源
CMakeLists.txt
```

## CI/CD
- GitHub Actions 在每次 push 到 master 时自动构建 Windows + Linux
- 推送 v* 标签时自动创建 Release（含 Setup.exe、Portable.zip、.deb）

## Commit Message 格式
- 遵循 Conventional Commits: `<type>: <description>`
- description 使用中文
