#include "ImageLoader.h"

#include <QFileInfo>
#include <QFutureWatcher>
#include <QImage>
#include <QtConcurrent>

namespace easypic {

struct ImageLoader::Impl {
    QList<QFutureWatcher<QImage>*> watchers;
};

ImageLoader::ImageLoader(QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>())
{
}

ImageLoader::~ImageLoader()
{
    cancelAll();
}

QPixmap ImageLoader::loadSync(const QString& filePath)
{
    QImage image(filePath);
    if (image.isNull()) {
        return QPixmap();
    }
    return QPixmap::fromImage(std::move(image));
}

void ImageLoader::loadAsync(const QString& filePath)
{
    auto* watcher = new QFutureWatcher<QImage>(this);
    m_impl->watchers.append(watcher);

    connect(watcher, &QFutureWatcher<QImage>::finished, this, [this, watcher, filePath]() {
        m_impl->watchers.removeOne(watcher);
        watcher->deleteLater();

        QImage image = watcher->result();
        if (image.isNull()) {
            emit loadFailed(filePath, QStringLiteral("Failed to load image: %1").arg(filePath));
            return;
        }
        // QPixmap::fromImage must be called on the GUI thread
        QPixmap pixmap = QPixmap::fromImage(std::move(image));
        emit imageLoaded(filePath, pixmap);
    });

    QFuture<QImage> future = QtConcurrent::run([filePath]() -> QImage {
        return QImage(filePath);
    });

    watcher->setFuture(future);
}

void ImageLoader::cancelAll()
{
    for (auto* watcher : m_impl->watchers) {
        watcher->cancel();
        watcher->waitForFinished();
        delete watcher;
    }
    m_impl->watchers.clear();
}

QStringList ImageLoader::supportedFilters()
{
    return {
        QStringLiteral("*.png"),
        QStringLiteral("*.jpg"),
        QStringLiteral("*.jpeg"),
        QStringLiteral("*.bmp"),
        QStringLiteral("*.gif"),
        QStringLiteral("*.webp"),
        QStringLiteral("*.tiff"),
        QStringLiteral("*.tif"),
        QStringLiteral("*.ico"),
    };
}

} // namespace easypic
