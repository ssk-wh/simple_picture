# EasyPicture

轻量级跨平台图片查看器，基于 Qt 5，支持 Windows / Linux，追求极速启动体验。

## 功能特性

- 支持常见图片格式：PNG、JPG、BMP、GIF、WEBP、TIFF、ICO、SVG
- SVG 矢量图无损缩放，任意放大不模糊
- 鼠标滚轮缩放，图片小于窗口时按原始像素大小显示
- 左右方向键切换上一张/下一张图片
- 拖拽图片文件到窗口打开
- 命令行参数指定文件打开
- 自动检测文件实际格式（后缀与内容不匹配时仍可正常打开）
- Windows 安装包，支持右键「打开方式」文件关联
- Linux deb 包，安装后自动添加系统菜单图标
- 按 F1 查看更新日志

## 构建

### 依赖

- C++17 编译器
- CMake 3.16+
- Qt 5.11+（推荐 5.15）
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

### 打包 deb（Linux）

```bash
dpkg-buildpackage -us -uc -b
```

## CI/CD

推送代码到 master 或添加 `v*` 标签时，GitHub Actions 会自动：

- 编译 Windows 版本并生成 NSIS 安装包 + 便携版
- 编译 Linux 版本并生成 deb 包
- 标签推送时自动创建 GitHub Release

## 项目结构

```
src/
  core/       核心逻辑（图片加载、缓存、导航）
  ui/         界面组件（主窗口、图片视图、更新日志对话框）
  main.cpp    入口
tests/        单元测试
installer/    NSIS 安装脚本
debian/       deb 打包配置
resources/    图标、桌面入口文件
```

## License

MIT
