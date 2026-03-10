#include "MainWindow.h"
#include "ImageView.h"
#include "../core/ImageCache.h"
#include "../core/ImageLoader.h"
#include "../core/ImageNavigator.h"

#include "ChangelogDialog.h"

#include <QFileInfo>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>

namespace easypic {

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
    setWindowTitle(QStringLiteral("EasyPicture"));
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

void MainWindow::displayFile(const QString& filePath)
{
    if (isSvgFile(filePath)) {
        m_imageView->setSvg(filePath);
    } else {
        QPixmap cached = m_cache->get(filePath);
        if (!cached.isNull()) {
            m_imageView->setPixmap(cached);
            return;
        }
        QPixmap pix = m_loader->loadSync(filePath);
        if (!pix.isNull()) {
            m_cache->put(filePath, pix);
            m_imageView->setPixmap(pix);
        }
    }
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
