# SimplePicture 架构设计文档

## 1. 整体架构

采用分层架构，自下而上分为三层：

```
┌─────────────────────────────────┐
│           UI Layer              │
│   MainWindow  /  ImageView      │
├─────────────────────────────────┤
│          Core Layer             │
│ ImageLoader / ImageCache /      │
│ ImageNavigator                  │
├─────────────────────────────────┤
│        Platform / Qt            │
│  QImage, QPixmap, QDir, QFile   │
└─────────────────────────────────┘
```

### 模块划分

| 模块 | 目录 | 职责 |
|------|------|------|
| Core | `src/core/` | 图片加载、缓存、目录导航，无 UI 依赖 |
| UI | `src/ui/` | 主窗口、图片视图控件，负责渲染和交互 |
| Entry | `src/main.cpp` | 应用入口，最小化启动逻辑 |

### 数据流

```
文件路径 → ImageNavigator(目录扫描/排序)
         → ImageLoader(同步/异步加载)
         → ImageCache(LRU缓存)
         → ImageView(渲染显示)
```

### 命名空间

所有类位于 `simplepic` 命名空间下。

---

## 2. 类设计

### 2.1 ImageLoader

**职责**：负责从磁盘加载图片，支持同步和异步两种模式。

- 同步加载用于首图快速显示
- 异步加载用于预加载前后图片，避免阻塞 UI
- 支持格式：PNG, JPG, BMP, GIF, WEBP, TIFF, ICO

**关键设计**：
- 异步加载使用 `QtConcurrent::run` + `QFutureWatcher`，避免手动线程管理
- 返回 `QPixmap` 供 UI 直接使用，加载在工作线程中用 `QImage` 完成后转换
- 加载失败返回 null QPixmap，由调用方处理

### 2.2 ImageCache

**职责**：基于 LRU 策略的图片缓存，避免重复加载。

- 以文件路径为 key，`QPixmap` 为 value
- 可配置最大缓存条目数（默认 20 张）
- 提供 `get` / `put` / `contains` / `clear` 接口
- 线程安全（`QMutex` 保护）

**关键设计**：
- 使用 `QHash` + 双向链表实现 O(1) 查找和淘汰
- 利用 Qt 的 `QCache` 简化实现（`QCache` 内部就是 LRU）
- 缓存命中时将条目移至链表头部

### 2.3 ImageNavigator

**职责**：管理当前目录中的图片文件列表，提供导航能力。

- 扫描指定目录下的所有支持格式图片
- 按文件名自然排序（natural sort）
- 提供 current / next / previous / jumpTo 接口
- 打开单个文件时，自动扫描其所在目录

**关键设计**：
- 文件过滤使用 `QDir::entryList` + name filters
- 自然排序确保 `img2.png` 在 `img10.png` 前面（使用 `QCollator` 的 numeric mode）
- 导航到达边界时不循环（到头了就停）

### 2.4 ImageView

**职责**：图片显示控件，处理所有用户交互。

- 继承 `QWidget`，重写 `paintEvent` 手动绘制
- 鼠标滚轮缩放（以鼠标位置为中心）
- 鼠标拖拽平移
- 双击恢复原始大小（1:1）
- 键盘左右方向键发出导航信号

**关键设计**：
- 维护一个 `m_offset`（平移偏移）和 `m_scale`（缩放比例）
- `paintEvent` 中用 `QPainter::drawPixmap` 配合变换矩阵绘制
- 滚轮缩放时，根据鼠标位置调整 offset，保证鼠标指向的图片点不动
- 缩放范围限制：0.01x ~ 100.0x
- 图片首次加载时自动 fit-to-window

### 2.5 MainWindow

**职责**：主窗口容器，协调各组件。

- 继承 `QWidget`（不用 `QMainWindow`，避免多余布局开销）
- 持有 `ImageView`、`ImageNavigator`、`ImageLoader`、`ImageCache`
- 处理文件打开（命令行参数 / 拖放）
- 连接导航信号与图片加载逻辑
- 窗口标题显示当前文件名和序号

**关键设计**：
- 无菜单栏、无工具栏、无状态栏，极简界面
- `ImageView` 填满整个窗口
- 支持文件拖放打开（`dragEnterEvent` / `dropEvent`）

---

## 3. 极速启动方案

### 3.1 启动流程优化

```
main() 启动流程（目标 < 200ms）：
1. QApplication 构造
2. MainWindow 构造（仅创建 ImageView，其余延迟）
3. MainWindow::show()
4. 解析命令行参数，若有文件则同步加载首图
5. QApplication::exec()

延迟到首图显示后（QTimer::singleShot(0, ...)）：
- ImageNavigator 扫描目录
- 启动预加载前后 N 张
```

### 3.2 策略

1. **最小构造**：`MainWindow` 构造函数只创建必要的子控件，不做 IO 操作
2. **同步加载首图**：用户指定的第一张图片同步加载，保证即刻可见
3. **延迟目录扫描**：目录扫描放在首图显示之后
4. **延迟预加载**：首图显示后再启动后台预加载

### 3.3 预加载策略

- 当前图片的前后各 2 张预加载到缓存
- 导航切换时，异步预加载新暴露的图片
- 预加载优先级：前进方向优先

---

## 4. 缩放实现方案

### 为什么选择 QWidget + QPainter 而非 QGraphicsView

| 维度 | QWidget + QPainter | QGraphicsView |
|------|-------------------|---------------|
| 启动开销 | 极小，单个 widget | 需创建 scene/view/item 三件套 |
| 内存占用 | 仅一个 QPixmap | 额外的 scene graph 数据结构 |
| 绘制控制 | 完全可控，直接操作 QPainter | 框架抽象层，间接控制 |
| 复杂度 | 简单直接，代码量少 | 对单图片场景过度设计 |
| 性能 | 单图片场景下更优 | 适合多图形对象场景 |

**结论**：SimplePicture 是单图片查看器，`QGraphicsView` 的 scene graph 架构完全多余。`QWidget` + `QPainter` 足够满足缩放/平移/绘制需求，启动更快，内存更小。

### 缩放实现细节

核心状态：
- `m_scale`: 当前缩放比例（double）
- `m_offset`: 图片左上角相对于 widget 左上角的偏移（QPointF）

滚轮缩放算法（以鼠标位置为中心）：

```cpp
void ImageView::wheelEvent(QWheelEvent* event) {
    const double oldScale = m_scale;
    const double factor = (event->angleDelta().y() > 0) ? 1.1 : (1.0 / 1.1);
    m_scale = qBound(kMinScale, m_scale * factor, kMaxScale);

    // 鼠标位置在 widget 坐标系中的点
    QPointF mousePos = event->position();
    // 调整 offset，使鼠标指向的图片内容点不变
    m_offset = mousePos - (m_scale / oldScale) * (mousePos - m_offset);

    update();
}
```

绘制：

```cpp
void ImageView::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    // 背景
    painter.fillRect(rect(), m_backgroundColor);

    if (m_pixmap.isNull()) return;

    // 计算目标矩形
    QRectF target(m_offset.x(), m_offset.y(),
                  m_pixmap.width() * m_scale,
                  m_pixmap.height() * m_scale);
    painter.drawPixmap(target, m_pixmap, m_pixmap.rect());
}
```

---

## 5. 缓存策略

### LRU 缓存设计

```
缓存容量: 20 张（可配置）
Key: 文件绝对路径 (QString)
Value: QPixmap

操作:
  get(path)     → 命中则返回 pixmap 并提升到 MRU 端；未命中返回 null
  put(path, px) → 插入/更新，超出容量时淘汰 LRU 端条目
  contains(path)→ 查询是否已缓存
  clear()       → 清空全部缓存
```

### 预加载策略

```
当前图片索引: i
预加载范围: [i-2, i+2]（共 5 张，含当前）

导航到 i+1 时:
  - 异步预加载 i+3（新暴露的前方图片）
  - i-2 自然在缓存中，若缓存未满则保留

预加载队列:
  - 前进方向的图片优先加载
  - 同一时刻最多 2 个并发预加载任务
```

### 内存估算

- 20 张 4K 图片（3840x2160 RGBA）≈ 20 × 32MB = 640MB
- 实际大部分图片远小于 4K，通常总内存 < 200MB
- 若需更严格控制，可按总像素数限制而非条目数

---

## 6. 跨平台注意事项

| 领域 | 注意事项 |
|------|----------|
| 文件路径 | 使用 `QDir::separator()` 或 `/`（Qt 自动转换） |
| 文件名排序 | 使用 `QCollator`，不同 locale 下行为一致 |
| 高 DPI | 使用 `QPixmap` 的 `devicePixelRatio`，确保绘制清晰 |
| 编码 | 文件路径统一 UTF-8，使用 `QString` 处理 |
| 构建 | CMake 统一构建系统，避免平台特定构建脚本 |
| WEBP 支持 | Qt 5.15.2 默认不含 WEBP 插件，需确保 `imageformats/qwebp.dll`（或 `.so`）在部署中 |
| TIFF 支持 | 同上，需 `imageformats/qtiff.dll`（或 `.so`） |
| ICO 支持 | Windows 原生支持，Linux 需 `imageformats/qico.so` |
| 窗口行为 | Linux 下窗口管理器差异大，避免依赖特定窗口状态 |

---

## 7. 头文件接口定义

### 7.1 `src/core/image_loader.h`

```cpp
#pragma once

#include <QObject>
#include <QPixmap>
#include <QString>
#include <functional>

namespace simplepic {

class ImageLoader : public QObject {
    Q_OBJECT

public:
    explicit ImageLoader(QObject* parent = nullptr);
    ~ImageLoader() override;

    // 同步加载图片，成功返回 QPixmap，失败返回 null QPixmap
    QPixmap loadSync(const QString& filePath);

    // 异步加载图片，完成后通过 imageLoaded 信号通知
    void loadAsync(const QString& filePath);

    // 取消所有正在进行的异步加载
    void cancelAll();

signals:
    // 异步加载完成信号
    void imageLoaded(const QString& filePath, const QPixmap& pixmap);

    // 异步加载失败信号
    void loadFailed(const QString& filePath, const QString& error);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace simplepic
```

### 7.2 `src/core/image_cache.h`

```cpp
#pragma once

#include <QMutex>
#include <QPixmap>
#include <QString>
#include <list>
#include <unordered_map>

namespace simplepic {

class ImageCache {
public:
    static constexpr int kDefaultCapacity = 20;

    explicit ImageCache(int capacity = kDefaultCapacity);
    ~ImageCache();

    // 禁止拷贝
    ImageCache(const ImageCache&) = delete;
    ImageCache& operator=(const ImageCache&) = delete;

    // 获取缓存的图片，未命中返回 null QPixmap
    QPixmap get(const QString& filePath);

    // 放入缓存，超出容量时淘汰最久未使用的条目
    void put(const QString& filePath, const QPixmap& pixmap);

    // 查询是否已缓存
    bool contains(const QString& filePath) const;

    // 清空缓存
    void clear();

    // 当前缓存条目数
    int size() const;

    // 缓存容量
    int capacity() const;

private:
    using CacheList = std::list<std::pair<QString, QPixmap>>;
    using CacheMap = std::unordered_map<size_t, CacheList::iterator>;

    void evict();
    size_t hashKey(const QString& filePath) const;

    int m_capacity;
    CacheList m_cacheList;   // front = MRU, back = LRU
    CacheMap m_cacheMap;
    mutable QMutex m_mutex;
};

} // namespace simplepic
```

### 7.3 `src/core/image_navigator.h`

```cpp
#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace simplepic {

class ImageNavigator : public QObject {
    Q_OBJECT

public:
    explicit ImageNavigator(QObject* parent = nullptr);
    ~ImageNavigator() override;

    // 扫描指定文件所在目录，定位到该文件
    void setCurrentFile(const QString& filePath);

    // 当前文件路径，无文件时返回空串
    QString currentFile() const;

    // 当前索引（从 0 开始），无文件时返回 -1
    int currentIndex() const;

    // 文件总数
    int totalCount() const;

    // 导航到下一张，到末尾时不动，返回是否成功
    bool goNext();

    // 导航到上一张，到开头时不动，返回是否成功
    bool goPrevious();

    // 跳转到指定索引
    bool jumpTo(int index);

    // 获取指定索引的文件路径
    QString fileAt(int index) const;

    // 获取文件列表
    QStringList fileList() const;

signals:
    // 当前文件变化信号
    void currentFileChanged(const QString& filePath, int index, int total);

private:
    void scanDirectory(const QString& dirPath);

    QStringList m_fileList;
    int m_currentIndex = -1;
};

} // namespace simplepic
```

### 7.4 `src/ui/image_view.h`

```cpp
#pragma once

#include <QPixmap>
#include <QPointF>
#include <QWidget>

namespace simplepic {

class ImageView : public QWidget {
    Q_OBJECT

public:
    static constexpr double kMinScale = 0.01;
    static constexpr double kMaxScale = 100.0;
    static constexpr double kScaleStep = 1.1;

    explicit ImageView(QWidget* parent = nullptr);
    ~ImageView() override;

    // 设置要显示的图片
    void setPixmap(const QPixmap& pixmap);

    // 获取当前图片
    const QPixmap& pixmap() const;

    // 获取当前缩放比例
    double scale() const;

    // 设置缩放比例（以 widget 中心为基准）
    void setScale(double scale);

    // 缩放适应窗口大小
    void fitToWindow();

    // 恢复原始大小 (1:1)
    void resetToOriginalSize();

signals:
    // 请求导航到下一张
    void nextImageRequested();

    // 请求导航到上一张
    void previousImageRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void clampOffset();

    QPixmap m_pixmap;
    double m_scale = 1.0;
    QPointF m_offset;           // 图片左上角在 widget 坐标系中的位置
    QPointF m_lastMousePos;     // 拖拽时的上一次鼠标位置
    bool m_dragging = false;
    QColor m_backgroundColor{30, 30, 30};  // 深灰背景
};

} // namespace simplepic
```

### 7.5 `src/ui/main_window.h`

```cpp
#pragma once

#include <QWidget>
#include <memory>

namespace simplepic {

class ImageView;
class ImageLoader;
class ImageCache;
class ImageNavigator;

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // 打开指定文件
    void openFile(const QString& filePath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onNextImage();
    void onPreviousImage();
    void onImageLoaded(const QString& filePath, const QPixmap& pixmap);
    void onCurrentFileChanged(const QString& filePath, int index, int total);

private:
    void setupUI();
    void connectSignals();
    void loadCurrentImage();
    void preloadNeighbors();
    void updateWindowTitle();

    ImageView* m_imageView = nullptr;
    std::unique_ptr<ImageLoader> m_loader;
    std::unique_ptr<ImageCache> m_cache;
    std::unique_ptr<ImageNavigator> m_navigator;

    QString m_currentFile;
};

} // namespace simplepic
```

### 7.6 `src/main.cpp` 入口框架

```cpp
#include "ui/main_window.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("SimplePicture");

    simplepic::MainWindow window;
    window.resize(1024, 768);
    window.show();

    // 命令行指定文件则打开
    const QStringList args = app.arguments();
    if (args.size() > 1) {
        window.openFile(args.at(1));
    }

    return app.exec();
}
```
