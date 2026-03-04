# EasyPicture - 跨平台图片查看应用

## 项目概述
基于 Qt 5.15.2 的跨平台图片查看器，支持 Windows/Linux，追求极速启动。

## 技术栈
- C++17
- Qt 5.15.2
- CMake 3.16+
- Google Test (单元测试)

## 架构约束
- **禁止使用 .ui 文件**：所有界面用 C++ 类直接实现
- **禁止使用 stylesheet**：通过重写 paintEvent 实现自定义绘制
- **跨平台**：Windows + Linux

## 核心功能
- 极速启动，打开常见图片格式 (PNG, JPG, BMP, GIF, WEBP, TIFF, ICO)
- 鼠标滚轮缩放
- 左右方向键切换上一张/下一张图片

## 编码规范
- 遵循 C++ Core Guidelines
- 类名: PascalCase (如 ImageViewer)
- 方法名: camelCase (如 loadImage)
- 成员变量: m_ 前缀 (如 m_currentIndex)
- 常量: k 前缀 + PascalCase (如 kMaxZoomLevel)
- 命名空间: easypic
- 头文件保护: #pragma once
- 使用 smart pointer，避免裸指针

## 目录结构
```
src/
  core/       - 核心逻辑 (图片加载、缓存、导航)
  ui/         - 界面组件 (主窗口、图片视图)
  main.cpp    - 入口
tests/        - 单元测试
CMakeLists.txt
```

## Commit Message 格式
- 遵循 Conventional Commits: `<type>: <description>`
- description 使用中文
