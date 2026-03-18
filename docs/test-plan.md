# SimplePicture 测试方案

## 1. 功能测试用例矩阵

| 编号 | 功能模块 | 测试用例 | 操作步骤 | 预期结果 |
|------|----------|----------|----------|----------|
| F-01 | 图片打开 | 通过命令行参数打开图片 | 执行 `simplepicture test.png` | 窗口显示 test.png，标题栏显示文件名 |
| F-02 | 图片打开 | 通过文件关联双击打开 | 双击 .png 文件 | 程序启动并显示该图片 |
| F-03 | 图片打开 | 无参数启动 | 直接运行 simplepicture | 显示空白窗口或欢迎界面，不崩溃 |
| F-04 | 图片显示 | 小图居中显示 | 打开 100x100 的 PNG | 图片在窗口中央完整显示 |
| F-05 | 图片显示 | 大图自适应缩放 | 打开 8000x6000 的 JPG | 图片缩放至窗口内完整可见 |
| F-06 | 滚轮缩放 | 向上滚动放大 | 鼠标在图片上方向上滚轮 | 图片以鼠标位置为中心放大 |
| F-07 | 滚轮缩放 | 向下滚动缩小 | 鼠标在图片上方向下滚轮 | 图片以鼠标位置为中心缩小 |
| F-08 | 滚轮缩放 | 在图片边缘缩放 | 鼠标在图片右下角滚轮放大 | 以鼠标位置为中心放大，不偏移到预期外位置 |
| F-09 | 滚轮缩放 | 缩放至极限 | 持续滚轮放大 | 到达最大缩放倍率后不再放大，无崩溃 |
| F-10 | 滚轮缩放 | 缩小至极限 | 持续滚轮缩小 | 到达最小缩放倍率后不再缩小，无崩溃 |
| F-11 | 键盘导航 | 右方向键下一张 | 打开目录中第1张图，按右键 | 切换到第2张图片 |
| F-12 | 键盘导航 | 左方向键上一张 | 打开目录中第3张图，按左键 | 切换到第2张图片 |
| F-13 | 键盘导航 | 在首张图按左键 | 打开目录中第1张图，按左键 | 不切换或循环到最后一张（取决于设计），不崩溃 |
| F-14 | 键盘导航 | 在末张图按右键 | 打开目录中最后一张图，按右键 | 不切换或循环到第一张（取决于设计），不崩溃 |
| F-15 | 鼠标拖拽 | 放大后拖拽平移 | 放大图片后，按住左键拖拽 | 图片跟随鼠标平移，松开后停止 |
| F-16 | 鼠标拖拽 | 未放大时拖拽 | 原始大小图片，按住左键拖拽 | 图片可平移或无响应（取决于设计），不崩溃 |
| F-17 | 双击复原 | 放大后双击 | 放大图片后双击 | 图片恢复原始大小并居中显示 |
| F-18 | 双击复原 | 缩小后双击 | 缩小图片后双击 | 图片恢复原始大小并居中显示 |
| F-19 | 双击复原 | 原始大小双击 | 未缩放时双击 | 无变化，不崩溃 |
| F-20 | 窗口适配 | 调整窗口大小 | 拖拽窗口边缘改变大小 | 图片显示自动适配新窗口尺寸，无撕裂/闪烁 |

## 2. 图片格式兼容性测试

| 格式 | 测试要点 | 测试用例 |
|------|----------|----------|
| **PNG** | 基础 RGB、带 Alpha 通道、16-bit depth、交错模式 | 打开 RGB PNG: 正常显示；打开透明 PNG: 透明区域显示棋盘格或背景色；打开 16-bit PNG: 不崩溃，正确渲染 |
| **JPG/JPEG** | 基线、渐进式、CMYK 色彩空间、EXIF 方向信息 | 打开基线 JPEG: 正常显示；打开渐进式 JPEG: 正常显示；打开含 EXIF 旋转标记的 JPEG: 按正确方向显示 |
| **BMP** | 1-bit、4-bit、8-bit、24-bit、32-bit 位深 | 打开 24-bit BMP: 正常显示；打开 1-bit BMP: 正常显示为黑白图 |
| **GIF** | 静态 GIF、动态 GIF（多帧） | 打开静态 GIF: 正常显示；打开动态 GIF: 显示第一帧或播放动画（取决于设计） |
| **WEBP** | 有损、无损、带 Alpha | 打开有损 WEBP: 正常显示；打开无损 WEBP: 正常显示；打开透明 WEBP: 透明区域正确处理 |
| **TIFF** | 单页、多页、不同压缩算法（LZW, ZIP） | 打开单页 TIFF: 正常显示；打开多页 TIFF: 显示第一页（或支持翻页） |
| **ICO** | 单尺寸、多尺寸 | 打开含多尺寸的 ICO: 显示最大尺寸图标 |

### 格式通用验证项
- 所有格式均须验证：打开速度、颜色准确性、内存占用合理
- 不支持的格式应给出友好提示而非崩溃

## 3. 性能测试

### 3.1 启动速度基准

| 场景 | 指标 | 目标值 |
|------|------|--------|
| 冷启动（无参数） | 从执行到窗口显示 | < 500ms |
| 冷启动（带图片参数） | 从执行到图片完整显示 | < 800ms |
| 热启动（系统缓存命中） | 从执行到窗口显示 | < 300ms |

**测量方法**: 记录 `main()` 入口时间戳与首次 `paintEvent` 完成时间戳之差。可通过 `QElapsedTimer` 内嵌计时或外部工具（如 Windows Performance Analyzer）测量。

### 3.2 大图加载性能

| 测试图片 | 分辨率 | 文件大小 | 目标加载时间 |
|----------|--------|----------|-------------|
| 普通照片 | 4000x3000 | ~5MB | < 500ms |
| 高分辨率 | 8000x6000 | ~20MB | < 2s |
| 超大图片 | 20000x15000 | ~100MB | < 5s，或给出加载进度 |
| 全景图 | 30000x4000 | ~50MB | < 3s |

### 3.3 缩放流畅度

| 场景 | 指标 | 目标值 |
|------|------|--------|
| 普通图片连续缩放 | 帧率 | >= 30 FPS |
| 大图连续缩放 | 帧率 | >= 20 FPS |
| 缩放操作响应延迟 | 从滚轮事件到画面更新 | < 50ms |

### 3.4 内存占用

| 场景 | 目标值 |
|------|--------|
| 空载（无图片） | < 30MB |
| 打开普通图片（4000x3000） | < 100MB |
| 连续切换 20 张图片后 | 无内存泄漏，占用稳定 |
| 打开超大图片 | 峰值 < 500MB |

## 4. 边界条件测试

| 编号 | 场景 | 操作 | 预期结果 |
|------|------|------|----------|
| E-01 | 空目录 | 打开一个不含任何图片的目录中的非图片文件 | 友好提示，不崩溃 |
| E-02 | 单张图片 | 打开只有一张图片的目录中的图片 | 正常显示，左右键无切换效果 |
| E-03 | 超大图片 | 打开 20000x15000 分辨率的 PNG | 能加载显示或给出内存不足提示，不崩溃 |
| E-04 | 损坏文件 | 打开截断的 JPEG 文件 | 显示部分内容或错误提示，不崩溃 |
| E-05 | 零字节文件 | 打开 0 字节的 .png 文件 | 错误提示，不崩溃 |
| E-06 | 伪装格式 | 打开扩展名为 .png 实际为文本文件 | 错误提示，不崩溃 |
| E-07 | 超长文件名 | 打开路径超过 260 字符的图片（Windows） | 能打开或给出路径过长提示 |
| E-08 | Unicode 路径 | 打开包含中文/日文/特殊字符路径的图片 | 正常加载显示 |
| E-09 | 只读文件 | 打开只读属性的图片 | 正常显示（查看器无需写权限） |
| E-10 | 文件被占用 | 打开正在被其他进程写入的图片 | 显示当前内容或错误提示，不崩溃 |
| E-11 | 同目录大量图片 | 目录中有 10000 张图片 | 导航列表加载不阻塞UI，切换响应正常 |
| E-12 | 1x1 像素图片 | 打开 1x1 像素的 PNG | 正常显示，可缩放 |
| E-13 | 极端宽高比 | 打开 10000x10 的图片 | 正常显示，缩放和拖拽正常 |

## 5. 交互测试

### 5.1 缩放极限测试

| 编号 | 场景 | 操作 | 预期结果 |
|------|------|------|----------|
| I-01 | 最大缩放 | 持续放大到极限后继续滚轮 | 停在最大倍率，无异常 |
| I-02 | 最小缩放 | 持续缩小到极限后继续滚轮 | 停在最小倍率，无异常 |
| I-03 | 缩放精度 | 放大 5 次后缩小 5 次 | 回到接近原始大小 |

### 5.2 快速连续操作

| 编号 | 场景 | 操作 | 预期结果 |
|------|------|------|----------|
| I-04 | 快速切换图片 | 持续快速按右方向键 | 图片连续切换，不卡顿、不崩溃、不出现残影 |
| I-05 | 快速缩放 | 快速连续滚轮操作 | 缩放流畅，无跳变 |
| I-06 | 切换时缩放 | 图片切换动画中滚轮缩放 | 无冲突，新图片正常显示 |
| I-07 | 缩放时切换 | 缩放过程中按方向键 | 正常切换到下一张，缩放状态重置或保持 |
| I-08 | 拖拽中切换 | 拖拽平移过程中按方向键 | 正常切换，拖拽状态正确释放 |
| I-09 | 快速双击 | 极快速连续双击 | 正常恢复原始大小，不触发意外行为 |

### 5.3 窗口缩放适配

| 编号 | 场景 | 操作 | 预期结果 |
|------|------|------|----------|
| I-10 | 最大化 | 点击最大化按钮 | 图片自适应全屏显示 |
| I-11 | 从最大化恢复 | 还原窗口大小 | 图片自适应新窗口尺寸 |
| I-12 | 最小化后恢复 | 最小化后恢复窗口 | 图片正常显示，无黑屏 |
| I-13 | 拖拽调整为极窄 | 将窗口宽度拖到最小 | 图片按比例缩小，不溢出 |
| I-14 | 拖拽调整为极矮 | 将窗口高度拖到最小 | 图片按比例缩小，不溢出 |
| I-15 | 多显示器拖拽 | 将窗口从主屏拖到副屏 | 正常显示，DPI 适配正确 |

## 6. 跨平台差异测试

### 6.1 Windows 特有测试项

| 测试项 | 说明 |
|--------|------|
| 高 DPI 支持 | 在 150%/200% 缩放的 Windows 10/11 下图片清晰，UI 元素不模糊 |
| 长路径支持 | 路径超过 MAX_PATH (260) 时的行为 |
| 文件关联 | 双击图片文件能正确启动 SimplePicture |
| 任务栏缩略图 | 任务栏预览缩略图正确显示当前图片 |
| 暗色模式 | Windows 暗色主题下界面显示正常 |

### 6.2 Linux 特有测试项

| 测试项 | 说明 |
|--------|------|
| X11/Wayland | 分别在 X11 和 Wayland 下测试显示和交互 |
| 不同桌面环境 | GNOME、KDE 下窗口装饰和行为正常 |
| 权限 | 打开无读取权限的文件时给出友好提示 |
| 符号链接 | 打开通过符号链接指向的图片 |
| 中文输入法 | 窗口焦点与输入法不冲突 |

### 6.3 跨平台一致性验证

| 验证项 | 要求 |
|--------|------|
| 图片渲染 | 同一图片在 Windows 和 Linux 下显示效果一致 |
| 缩放行为 | 相同滚轮操作产生一致的缩放效果 |
| 键盘导航 | 方向键功能行为一致 |
| 窗口默认尺寸 | 初始窗口大小在两个平台上视觉一致 |

## 7. 单元测试用例设计

### 7.1 ImageLoader 测试

```
测试类: ImageLoaderTest
依赖: Google Test
命名空间: simplepic::test
```

| 编号 | 测试用例 | 输入 | 预期输出 |
|------|----------|------|----------|
| UL-01 | `LoadValidPng` | 有效 PNG 文件路径 | 返回非空 QImage，尺寸与源文件一致 |
| UL-02 | `LoadValidJpg` | 有效 JPG 文件路径 | 返回非空 QImage |
| UL-03 | `LoadValidBmp` | 有效 BMP 文件路径 | 返回非空 QImage |
| UL-04 | `LoadValidGif` | 有效 GIF 文件路径 | 返回非空 QImage（至少第一帧） |
| UL-05 | `LoadValidWebp` | 有效 WEBP 文件路径 | 返回非空 QImage |
| UL-06 | `LoadValidTiff` | 有效 TIFF 文件路径 | 返回非空 QImage |
| UL-07 | `LoadValidIco` | 有效 ICO 文件路径 | 返回非空 QImage |
| UL-08 | `LoadNonExistentFile` | 不存在的文件路径 | 返回空 QImage 或抛出异常，不崩溃 |
| UL-09 | `LoadEmptyFile` | 0 字节文件路径 | 返回空 QImage，不崩溃 |
| UL-10 | `LoadCorruptedFile` | 损坏的 JPEG 文件路径 | 返回空 QImage 或部分数据，不崩溃 |
| UL-11 | `LoadFakeExtension` | 扩展名 .png 实际为文本 | 返回空 QImage，不崩溃 |
| UL-12 | `LoadImageDimensions` | 已知 800x600 的 PNG | 返回 QImage，width=800，height=600 |
| UL-13 | `LoadTransparentPng` | 带 Alpha 通道的 PNG | 返回 QImage，format 包含 Alpha |
| UL-14 | `LoadUnicodePath` | 路径含中文字符的图片 | 正常返回非空 QImage |
| UL-15 | `LoadLargeImage` | 10000x8000 的 JPEG | 返回非空 QImage，耗时 < 5s |
| UL-16 | `SupportedFormats` | 调用获取支持格式列表 | 返回列表包含 png, jpg, bmp, gif, webp, tiff, ico |

### 7.2 ImageCache 测试

```
测试类: ImageCacheTest
依赖: Google Test
命名空间: simplepic::test
```

| 编号 | 测试用例 | 操作 | 预期结果 |
|------|----------|------|----------|
| UC-01 | `CacheHit` | 加载图片 A，再次请求图片 A | 第二次从缓存返回，耗时远小于首次加载 |
| UC-02 | `CacheMiss` | 请求未缓存的图片 B | 触发实际加载，返回有效 QImage |
| UC-03 | `CacheEviction` | 缓存满后加载新图片 | 最久未使用的缓存项被淘汰 |
| UC-04 | `CacheCapacity` | 设置缓存容量为 3，加载 4 张图 | 缓存中保留最近 3 张，第1张被淘汰 |
| UC-05 | `CacheClear` | 加载多张图后清空缓存 | 所有缓存被清除，内存释放 |
| UC-06 | `CacheThreadSafety` | 多线程同时请求不同图片 | 无数据竞争，所有请求正确返回 |
| UC-07 | `CacheMemoryLimit` | 设置内存上限后加载大图 | 超出内存上限时自动淘汰旧条目 |
| UC-08 | `CacheSameImageTwice` | 同一图片路径插入两次 | 不产生重复条目，缓存大小不变 |
| UC-09 | `CacheInvalidImage` | 缓存一个加载失败的结果 | 不缓存失败结果，下次请求重新尝试加载 |
| UC-10 | `CachePreload` | 预加载当前图片前后各 N 张 | 相邻图片提前进入缓存，切换时无加载延迟 |

### 7.3 ImageNavigator 测试

```
测试类: ImageNavigatorTest
依赖: Google Test
命名空间: simplepic::test
```

| 编号 | 测试用例 | 操作 | 预期结果 |
|------|----------|------|----------|
| UN-01 | `NavigateNext` | 当前索引 0，调用 next() | 当前索引变为 1，返回第 2 张图片路径 |
| UN-02 | `NavigatePrevious` | 当前索引 2，调用 previous() | 当前索引变为 1，返回第 2 张图片路径 |
| UN-03 | `NavigateNextAtEnd` | 当前索引为最后一张，调用 next() | 索引不变或循环到 0（取决于设计），不越界 |
| UN-04 | `NavigatePreviousAtStart` | 当前索引为 0，调用 previous() | 索引不变或循环到最后（取决于设计），不越界 |
| UN-05 | `NavigateEmptyDirectory` | 空目录初始化 Navigator | 当前索引为 -1 或无效，next/previous 不崩溃 |
| UN-06 | `NavigateSingleImage` | 只有 1 张图片 | next/previous 不切换，索引始终为 0 |
| UN-07 | `ScanDirectory` | 扫描包含 5 张图片和 3 个非图片文件的目录 | 仅收录 5 张图片，按文件名排序 |
| UN-08 | `ScanFilterExtensions` | 目录含 .png .txt .jpg .doc | 仅收录 .png 和 .jpg |
| UN-09 | `CurrentImagePath` | 设置索引为 2 | 返回第 3 张图片的完整路径 |
| UN-10 | `TotalImageCount` | 扫描含 10 张图片的目录 | 返回 count = 10 |
| UN-11 | `NavigateToIndex` | 直接跳转到索引 5 | 当前索引变为 5，返回正确路径 |
| UN-12 | `NavigateToInvalidIndex` | 跳转到索引 -1 或超出范围 | 不执行跳转，返回错误或保持当前索引 |
| UN-13 | `SortOrder` | 目录含 c.png, a.png, b.png | 导航顺序为 a.png, b.png, c.png（字典序） |
| UN-14 | `CaseInsensitiveExtension` | 目录含 test.PNG, test.Jpg | 均被识别为图片文件 |
| UN-15 | `NestedDirectoryIgnored` | 目录含子目录且子目录中有图片 | 仅扫描当前目录，不递归子目录 |

### 7.4 示例测试代码骨架

```cpp
// tests/test_image_loader.cpp
#include <gtest/gtest.h>
#include "core/image_loader.h"

namespace simplepic::test {

class ImageLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_loader = std::make_unique<ImageLoader>();
        m_testDataDir = "test_data/";
    }

    std::unique_ptr<ImageLoader> m_loader;
    std::string m_testDataDir;
};

TEST_F(ImageLoaderTest, LoadValidPng) {
    auto image = m_loader->load(m_testDataDir + "valid.png");
    EXPECT_FALSE(image.isNull());
    EXPECT_GT(image.width(), 0);
    EXPECT_GT(image.height(), 0);
}

TEST_F(ImageLoaderTest, LoadNonExistentFile) {
    auto image = m_loader->load("nonexistent.png");
    EXPECT_TRUE(image.isNull());
}

TEST_F(ImageLoaderTest, LoadEmptyFile) {
    auto image = m_loader->load(m_testDataDir + "empty.png");
    EXPECT_TRUE(image.isNull());
}

TEST_F(ImageLoaderTest, LoadImageDimensions) {
    auto image = m_loader->load(m_testDataDir + "800x600.png");
    EXPECT_EQ(image.width(), 800);
    EXPECT_EQ(image.height(), 600);
}

TEST_F(ImageLoaderTest, SupportedFormats) {
    auto formats = m_loader->supportedFormats();
    EXPECT_NE(std::find(formats.begin(), formats.end(), "png"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "jpg"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "bmp"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "gif"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "webp"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "tiff"), formats.end());
    EXPECT_NE(std::find(formats.begin(), formats.end(), "ico"), formats.end());
}

} // namespace simplepic::test
```

```cpp
// tests/test_image_cache.cpp
#include <gtest/gtest.h>
#include "core/image_cache.h"

namespace simplepic::test {

class ImageCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_cache = std::make_unique<ImageCache>(/* capacity= */ 5);
    }

    std::unique_ptr<ImageCache> m_cache;
};

TEST_F(ImageCacheTest, CacheHit) {
    QImage img(100, 100, QImage::Format_RGB32);
    m_cache->put("test.png", img);
    auto cached = m_cache->get("test.png");
    EXPECT_FALSE(cached.isNull());
    EXPECT_EQ(cached.size(), img.size());
}

TEST_F(ImageCacheTest, CacheMiss) {
    auto cached = m_cache->get("nonexistent.png");
    EXPECT_TRUE(cached.isNull());
}

TEST_F(ImageCacheTest, CacheEviction) {
    // 容量为 5，加载 6 张图
    for (int i = 0; i < 6; ++i) {
        QImage img(100, 100, QImage::Format_RGB32);
        m_cache->put("img" + std::to_string(i) + ".png", img);
    }
    // 第一张应被淘汰
    EXPECT_TRUE(m_cache->get("img0.png").isNull());
    // 最后一张应在缓存中
    EXPECT_FALSE(m_cache->get("img5.png").isNull());
}

TEST_F(ImageCacheTest, CacheClear) {
    QImage img(100, 100, QImage::Format_RGB32);
    m_cache->put("test.png", img);
    m_cache->clear();
    EXPECT_TRUE(m_cache->get("test.png").isNull());
}

} // namespace simplepic::test
```

```cpp
// tests/test_image_navigator.cpp
#include <gtest/gtest.h>
#include "core/image_navigator.h"

namespace simplepic::test {

class ImageNavigatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_navigator = std::make_unique<ImageNavigator>();
        m_testDir = "test_data/navigation/";
    }

    std::unique_ptr<ImageNavigator> m_navigator;
    std::string m_testDir;
};

TEST_F(ImageNavigatorTest, ScanDirectory) {
    m_navigator->scan(m_testDir);
    EXPECT_GT(m_navigator->count(), 0);
}

TEST_F(ImageNavigatorTest, NavigateNext) {
    m_navigator->scan(m_testDir);
    EXPECT_EQ(m_navigator->currentIndex(), 0);
    m_navigator->next();
    EXPECT_EQ(m_navigator->currentIndex(), 1);
}

TEST_F(ImageNavigatorTest, NavigatePrevious) {
    m_navigator->scan(m_testDir);
    m_navigator->next();
    m_navigator->next();
    m_navigator->previous();
    EXPECT_EQ(m_navigator->currentIndex(), 1);
}

TEST_F(ImageNavigatorTest, NavigateEmptyDirectory) {
    m_navigator->scan("test_data/empty_dir/");
    EXPECT_EQ(m_navigator->count(), 0);
    // 不应崩溃
    m_navigator->next();
    m_navigator->previous();
}

TEST_F(ImageNavigatorTest, SortOrder) {
    m_navigator->scan(m_testDir);
    auto files = m_navigator->fileList();
    EXPECT_TRUE(std::is_sorted(files.begin(), files.end()));
}

} // namespace simplepic::test
```

## 附录: 测试数据准备

### 所需测试图片清单

| 文件 | 说明 | 用途 |
|------|------|------|
| `valid.png` | 800x600 RGB PNG | 基础加载测试 |
| `transparent.png` | 带 Alpha 通道 PNG | 透明度测试 |
| `800x600.png` | 已知尺寸 PNG | 尺寸验证 |
| `valid.jpg` | 标准 JPEG | 格式测试 |
| `progressive.jpg` | 渐进式 JPEG | 渐进式加载测试 |
| `exif_rotated.jpg` | 带 EXIF 旋转的 JPEG | 方向校正测试 |
| `valid.bmp` | 24-bit BMP | 格式测试 |
| `valid.gif` | 静态 GIF | 格式测试 |
| `animated.gif` | 多帧动画 GIF | 动画测试 |
| `valid.webp` | WEBP 图片 | 格式测试 |
| `valid.tiff` | 标准 TIFF | 格式测试 |
| `valid.ico` | 多尺寸 ICO | 格式测试 |
| `large.jpg` | 10000x8000 JPEG | 大图性能测试 |
| `corrupted.jpg` | 截断的 JPEG | 损坏文件测试 |
| `empty.png` | 0 字节文件 | 空文件测试 |
| `fake.png` | 实际为文本文件 | 伪装格式测试 |
| `1x1.png` | 1x1 像素 PNG | 极小图片测试 |
| `wide.png` | 10000x10 PNG | 极端宽高比测试 |
| `c.png, a.png, b.png` | 命名测试图片 | 排序测试 |
