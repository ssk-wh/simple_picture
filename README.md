# EasyPicture

轻量级跨平台图片查看器，基于 Qt 5.15，追求极速启动体验。

## 功能特性

- 支持常见图片格式：PNG、JPG、BMP、GIF、WEBP、TIFF、ICO、SVG
- 鼠标滚轮缩放
- 左右方向键切换上一张/下一张图片
- Windows 文件关联，右键「打开方式」直接使用

## 构建

### 依赖

- C++17 编译器
- CMake 3.16+
- Qt 5.15.2
- Ninja（推荐）

### 编译

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### 打包安装程序（Windows）

需要安装 [NSIS](https://nsis.sourceforge.io/)：

```bash
bash installer/build_installer.sh
```

## 项目结构

```
src/
  core/       核心逻辑（图片加载、缓存、导航）
  ui/         界面组件（主窗口、图片视图）
  main.cpp    入口
tests/        单元测试
installer/    NSIS 安装脚本
```

## License

MIT
