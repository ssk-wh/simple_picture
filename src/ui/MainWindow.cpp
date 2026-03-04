#include "MainWindow.h"
#include "ImageView.h"
#include "../core/ImageCache.h"
#include "../core/ImageLoader.h"
#include "../core/ImageNavigator.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QPainter>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace easypic {

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
    , m_loader(std::make_unique<ImageLoader>(this))
    , m_cache(std::make_unique<ImageCache>())
    , m_navigator(std::make_unique<ImageNavigator>(this))
{
    setupUI();
    connectSignals();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    setWindowTitle(QStringLiteral("EasyPicture"));
    setMinimumSize(640, 480);
    resize(1024, 768);

    setAcceptDrops(true);

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
}

void MainWindow::openFile(const QString& filePath)
{
    QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isFile())
        return;

    const QString absPath = fi.absoluteFilePath();

    // Synchronously load the first image for immediate display
    QPixmap pix = m_loader->loadSync(absPath);
    if (!pix.isNull()) {
        m_cache->put(absPath, pix);
        m_imageView->setPixmap(pix);
    }

    m_currentFile = absPath;

    // Scan directory and set navigator (deferred so the window appears fast)
    QTimer::singleShot(0, this, [this, absPath]() {
        m_navigator->setCurrentFile(absPath);
        updateWindowTitle();
        preloadNeighbors();
    });
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        const auto urls = event->mimeData()->urls();
        for (const auto& url : urls) {
            if (url.isLocalFile()) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const auto urls = event->mimeData()->urls();
    for (const auto& url : urls) {
        if (url.isLocalFile()) {
            openFile(url.toLocalFile());
            event->acceptProposedAction();
            return;
        }
    }
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
    m_cache->put(filePath, pixmap);

    // If this is the currently displayed file, show it
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

    // Try cache first
    QPixmap cached = m_cache->get(file);
    if (!cached.isNull()) {
        m_imageView->setPixmap(cached);
        updateWindowTitle();
        preloadNeighbors();
        return;
    }

    // Cache miss: load synchronously for immediate display
    QPixmap pix = m_loader->loadSync(file);
    if (!pix.isNull()) {
        m_cache->put(file, pix);
        m_imageView->setPixmap(pix);
    }

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

void MainWindow::updateWindowTitle()
{
    const int index = m_navigator->currentIndex();
    const int total = m_navigator->totalCount();

    if (m_currentFile.isEmpty() || index < 0) {
        setWindowTitle(QStringLiteral("EasyPicture"));
        return;
    }

    const QString fileName = QFileInfo(m_currentFile).fileName();
    setWindowTitle(QStringLiteral("EasyPicture - %1 [%2/%3]")
                       .arg(fileName)
                       .arg(index + 1)
                       .arg(total));
}

} // namespace easypic
