#include "MainWindow.h"
#include "ImageView.h"
#include "../core/ImageCache.h"
#include "../core/ImageLoader.h"
#include "../core/ImageNavigator.h"

#include "ChangelogDialog.h"

#include <QDateTime>
#include <QFileInfo>
#include <QImageReader>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>

namespace simplepic {

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
    , m_loader(std::make_unique<ImageLoader>())
    , m_cache(std::make_unique<ImageCache>())
    , m_navigator(std::make_unique<ImageNavigator>())
{
    setupUI();
    connectSignals();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    setWindowTitle(QStringLiteral("SimplePicture"));
    setMinimumSize(640, 480);
    resize(1024, 768);

    m_imageView = new ImageView(this);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_imageView);
}

void MainWindow::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(30, 30, 30));
}

void MainWindow::connectSignals()
{
    connect(m_imageView, &ImageView::nextImageRequested,
            this, &MainWindow::onNextImage);
    connect(m_imageView, &ImageView::previousImageRequested,
            this, &MainWindow::onPreviousImage);

    connect(m_loader.get(), &ImageLoader::imageLoaded,
            this, &MainWindow::onImageLoaded);

    connect(m_navigator.get(), &ImageNavigator::currentFileChanged,
            this, &MainWindow::onCurrentFileChanged);

    connect(m_imageView, &ImageView::fileDropped,
            this, &MainWindow::openFile);
}

static bool isSvgFile(const QString& path)
{
    return path.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive);
}

static ImageInfo buildImageInfo(const QString& filePath)
{
    ImageInfo info;
    QFileInfo fi(filePath);
    info.filePath = fi.absoluteFilePath();
    info.fileSize = fi.size();
    info.lastModified = fi.lastModified().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));

    QImageReader reader(filePath);
    reader.setDecideFormatFromContent(true);
    const QSize imgSize = reader.size();
    if (imgSize.isValid()) {
        info.pixelWidth = imgSize.width();
        info.pixelHeight = imgSize.height();
    }
    info.format = QString::fromLatin1(reader.format()).toUpper();
    if (info.format.isEmpty())
        info.format = fi.suffix().toUpper();

    // Try to get bit depth from image
    const QImage::Format fmt = reader.imageFormat();
    if (fmt != QImage::Format_Invalid)
        info.bitDepth = QImage::toPixelFormat(fmt).bitsPerPixel();

    return info;
}

void MainWindow::displayFile(const QString& filePath)
{
    if (isSvgFile(filePath)) {
        m_imageView->setSvg(filePath);
        if (m_imageView->pixmap().isNull() && !filePath.isEmpty()) {
            m_imageView->setError(QStringLiteral("无法打开图片: SVG"));
        }
    } else {
        QPixmap cached = m_cache->get(filePath);
        if (!cached.isNull()) {
            m_imageView->setPixmap(cached);
        } else {
            QPixmap pix = m_loader->loadSync(filePath);
            if (!pix.isNull()) {
                m_cache->put(filePath, pix);
                m_imageView->setPixmap(pix);
            } else {
                const QString suffix = QFileInfo(filePath).suffix().toUpper();
                m_imageView->setError(
                    QStringLiteral("无法打开图片: %1").arg(suffix.isEmpty() ? filePath : suffix));
            }
        }
    }

    m_imageView->setImageInfo(buildImageInfo(filePath));
}

void MainWindow::openFile(const QString& filePath)
{
    QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isFile())
        return;

    const QString absPath = fi.absoluteFilePath();

    displayFile(absPath);

    m_currentFile = absPath;

    // Scan directory and set navigator (deferred so the window appears fast)
    QTimer::singleShot(0, this, [this, absPath]() {
        m_navigator->setCurrentFile(absPath);
        updateWindowTitle();
        preloadNeighbors();
    });
}

void MainWindow::onNextImage()
{
    if (m_navigator->goNext()) {
        loadCurrentImage();
    }
}

void MainWindow::onPreviousImage()
{
    if (m_navigator->goPrevious()) {
        loadCurrentImage();
    }
}

void MainWindow::onImageLoaded(const QString& filePath, const QPixmap& pixmap)
{
    if (isSvgFile(filePath))
        return;

    m_cache->put(filePath, pixmap);

    if (filePath == m_currentFile) {
        m_imageView->setPixmap(pixmap);
    }
}

void MainWindow::onCurrentFileChanged(const QString& filePath, int /*index*/, int /*total*/)
{
    m_currentFile = filePath;
    updateWindowTitle();
}

void MainWindow::loadCurrentImage()
{
    const QString file = m_navigator->currentFile();
    if (file.isEmpty())
        return;

    m_currentFile = file;
    displayFile(file);
    updateWindowTitle();
    preloadNeighbors();
}

void MainWindow::preloadNeighbors()
{
    const int current = m_navigator->currentIndex();
    const int total = m_navigator->totalCount();
    if (current < 0 || total <= 0)
        return;

    // Preload 2 images ahead and 2 behind
    for (int delta : {-2, -1, 1, 2}) {
        const int idx = current + delta;
        if (idx < 0 || idx >= total)
            continue;

        const QString file = m_navigator->fileAt(idx);
        if (file.isEmpty() || m_cache->contains(file))
            continue;

        m_loader->loadAsync(file);
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F1) {
        showChangelog();
        return;
    }
    QWidget::keyPressEvent(event);
}

void MainWindow::showChangelog()
{
    ChangelogDialog dlg(this);
    dlg.exec();
}

void MainWindow::updateWindowTitle()
{
    const int index = m_navigator->currentIndex();
    const int total = m_navigator->totalCount();

    if (m_currentFile.isEmpty() || index < 0) {
        setWindowTitle(QStringLiteral("SimplePicture"));
        return;
    }

    const QString fileName = QFileInfo(m_currentFile).fileName();
    setWindowTitle(QStringLiteral("SimplePicture - %1 [%2/%3]")
                       .arg(fileName)
                       .arg(index + 1)
                       .arg(total));
}

} // namespace simplepic
