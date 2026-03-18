#include "ImageView.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSettings>
#include <QSvgRenderer>
#include <QUrl>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace simplepic {

ImageView::ImageView(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);

    m_svgCacheTimer.setSingleShot(true);
    connect(&m_svgCacheTimer, &QTimer::timeout, this, &ImageView::updateSvgCache);

    QSettings settings(QStringLiteral("SimplePicture"), QStringLiteral("SimplePicture"));
    m_showInfo = settings.value(QStringLiteral("showImageInfo"), false).toBool();
}

ImageView::~ImageView() = default;

void ImageView::setPixmap(const QPixmap& pixmap)
{
    m_svgCacheTimer.stop();
    m_svgRenderer.reset();
    m_svgOverview = QPixmap();
    m_svgPixmapCache = QPixmap();
    m_errorMessage.clear();
    m_pixmap = pixmap;
    m_dragging = false;
    m_fittedToWindow = true;
    fitToWindow();
    update();
}

void ImageView::setError(const QString& message)
{
    m_svgCacheTimer.stop();
    m_svgRenderer.reset();
    m_svgOverview = QPixmap();
    m_svgPixmapCache = QPixmap();
    m_pixmap = QPixmap();
    m_dragging = false;
    m_errorMessage = message;
    update();
}

void ImageView::setSvg(const QString& filePath)
{
    m_svgCacheTimer.stop();
    auto renderer = std::make_unique<QSvgRenderer>(filePath);
    if (!renderer->isValid())
        return;

    m_errorMessage.clear();
    m_svgRenderer = std::move(renderer);
    m_svgDefaultSize = m_svgRenderer->defaultSize();
    if (m_svgDefaultSize.isEmpty())
        m_svgDefaultSize = QSizeF(1024, 1024);

    m_pixmap = QPixmap(m_svgDefaultSize.toSize());
    m_pixmap.fill(Qt::transparent);

    m_svgPixmapCache = QPixmap();
    m_svgCacheScale = 0.0;

    // Pre-render a small overview for smooth zoom animation
    {
        constexpr int kOverviewMax = 2048;
        QSize overviewSize = m_svgDefaultSize.toSize().scaled(
            kOverviewMax, kOverviewMax, Qt::KeepAspectRatio);
        m_svgOverview = QPixmap(overviewSize);
        m_svgOverview.fill(Qt::transparent);
        QPainter painter(&m_svgOverview);
        painter.setRenderHint(QPainter::Antialiasing);
        m_svgRenderer->render(&painter);
    }

    m_dragging = false;
    m_fittedToWindow = true;
    fitToWindow();
    updateSvgCache();
    update();
}

void ImageView::setImageInfo(const ImageInfo& info)
{
    m_imageInfo = info;
    if (m_showInfo)
        update();
}

const QPixmap& ImageView::pixmap() const
{
    return m_pixmap;
}

double ImageView::scale() const
{
    return m_scale;
}

void ImageView::setScale(double scale)
{
    m_scale = std::clamp(scale, kMinScale, kMaxScale);
    m_fittedToWindow = false;
    clampOffset();
    update();
}

void ImageView::fitToWindow()
{
    if (m_pixmap.isNull()) {
        m_scale = 1.0;
        m_offset = QPointF(0, 0);
        return;
    }

    const double widgetW = width();
    const double widgetH = height();
    const double pixmapW = m_pixmap.width();
    const double pixmapH = m_pixmap.height();

    if (pixmapW <= 0 || pixmapH <= 0) {
        m_scale = 1.0;
        m_offset = QPointF(0, 0);
        return;
    }

    const double scaleX = widgetW / pixmapW;
    const double scaleY = widgetH / pixmapH;
    // Use original size (1:1), only shrink if image exceeds window
    m_scale = std::min({1.0, scaleX, scaleY});
    m_scale = std::clamp(m_scale, kMinScale, kMaxScale);

    // Center the image
    const double scaledW = pixmapW * m_scale;
    const double scaledH = pixmapH * m_scale;
    m_offset.setX((widgetW - scaledW) / 2.0);
    m_offset.setY((widgetH - scaledH) / 2.0);
}

void ImageView::resetToOriginalSize()
{
    if (m_pixmap.isNull())
        return;

    m_scale = 1.0;
    m_fittedToWindow = false;

    // Center the image at 1:1
    const double widgetW = width();
    const double widgetH = height();
    const double pixmapW = m_pixmap.width();
    const double pixmapH = m_pixmap.height();
    m_offset.setX((widgetW - pixmapW) / 2.0);
    m_offset.setY((widgetH - pixmapH) / 2.0);

    clampOffset();
    update();
}

void ImageView::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Background
    painter.fillRect(rect(), m_backgroundColor);

    if (m_pixmap.isNull()) {
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.setPen(QColor(140, 140, 140));
        if (m_errorMessage.isEmpty()) {
            painter.drawText(rect(), Qt::AlignCenter,
                             QStringLiteral("Drag an image here or open via command line"));
        } else {
            painter.drawText(rect(), Qt::AlignCenter, m_errorMessage);
        }
        return;
    }

    // Draw the image
    const double scaledW = m_pixmap.width() * m_scale;
    const double scaledH = m_pixmap.height() * m_scale;
    const QRectF targetRect(m_offset.x(), m_offset.y(), scaledW, scaledH);

    if (m_svgRenderer && !m_svgPixmapCache.isNull()
        && m_svgCacheScale == m_scale) {
        // Full SVG cache at current scale — draw at offset (instant drag)
        painter.drawPixmap(m_offset, m_svgPixmapCache);
    } else if (m_svgRenderer && !m_svgPixmapCache.isNull()
               && m_svgCacheScale > 0) {
        // Scale changed — stretch old cache as placeholder
        const double ratio = m_scale / m_svgCacheScale;
        const double w = m_svgPixmapCache.width() * ratio;
        const double h = m_svgPixmapCache.height() * ratio;
        painter.drawPixmap(QRectF(m_offset.x(), m_offset.y(), w, h),
                           m_svgPixmapCache,
                           QRectF(m_svgPixmapCache.rect()));
    } else if (m_svgRenderer && !m_svgOverview.isNull()) {
        // No cache yet — use overview
        painter.drawPixmap(targetRect, m_svgOverview, QRectF(m_svgOverview.rect()));
    } else if (m_svgRenderer) {
        painter.setRenderHint(QPainter::Antialiasing);
        m_svgRenderer->render(&painter, targetRect);
    } else {
        painter.drawPixmap(targetRect, m_pixmap, QRectF(m_pixmap.rect()));
    }

    if (m_showInfo)
        drawInfoPanel(painter);
}

void ImageView::wheelEvent(QWheelEvent* event)
{
    if (m_pixmap.isNull()) {
        event->ignore();
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const QPointF mousePos = event->position();
#else
    const QPointF mousePos = event->posF();
#endif
    const double oldScale = m_scale;

    // Zoom in or out
    if (event->angleDelta().y() > 0) {
        m_scale *= kScaleStep;
    } else {
        m_scale /= kScaleStep;
    }
    m_scale = std::clamp(m_scale, kMinScale, kMaxScale);
    m_fittedToWindow = false;

    // Zoom centered on mouse position
    const double ratio = m_scale / oldScale;
    m_offset.setX(mousePos.x() - (mousePos.x() - m_offset.x()) * ratio);
    m_offset.setY(mousePos.y() - (mousePos.y() - m_offset.y()) * ratio);

    clampOffset();

    if (m_svgRenderer)
        m_svgCacheTimer.start(150);

    update();
    event->accept();
}

void ImageView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !m_pixmap.isNull()) {
        m_dragging = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else if (event->button() == Qt::RightButton) {
        m_showInfo = !m_showInfo;
        QSettings settings(QStringLiteral("SimplePicture"), QStringLiteral("SimplePicture"));
        settings.setValue(QStringLiteral("showImageInfo"), m_showInfo);
        update();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        const QPointF delta = event->pos() - m_lastMousePos;
        m_offset += delta;
        m_lastMousePos = event->pos();
        clampOffset();

        update();
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void ImageView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void ImageView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
        resetToOriginalSize();
        event->accept();
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
}

void ImageView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        window()->close();
        event->accept();
        break;
    case Qt::Key_Right:
        emit nextImageRequested();
        event->accept();
        break;
    case Qt::Key_Left:
        emit previousImageRequested();
        event->accept();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void ImageView::resizeEvent(QResizeEvent* /*event*/)
{
    if (!m_pixmap.isNull()) {
        if (m_fittedToWindow) {
            fitToWindow();
        } else {
            clampOffset();
        }
        if (m_svgRenderer && m_svgCacheScale != m_scale)
            m_svgCacheTimer.start(150);
        update();
    }
}

void ImageView::clampOffset()
{
    if (m_pixmap.isNull())
        return;

    const double scaledW = m_pixmap.width() * m_scale;
    const double scaledH = m_pixmap.height() * m_scale;
    const double widgetW = width();
    const double widgetH = height();

    // Ensure at least some portion of the image stays visible
    const double visibleMin = 50.0;

    if (scaledW <= widgetW) {
        // Image fits horizontally - center it
        m_offset.setX((widgetW - scaledW) / 2.0);
    } else {
        const double minX = widgetW - scaledW - visibleMin;
        const double maxX = visibleMin;
        m_offset.setX(std::clamp(m_offset.x(), minX, maxX));
    }

    if (scaledH <= widgetH) {
        // Image fits vertically - center it
        m_offset.setY((widgetH - scaledH) / 2.0);
    } else {
        const double minY = widgetH - scaledH - visibleMin;
        const double maxY = visibleMin;
        m_offset.setY(std::clamp(m_offset.y(), minY, maxY));
    }
}

void ImageView::updateSvgCache()
{
    if (!m_svgRenderer)
        return;

    // Skip if scale unchanged (offset changes don't need re-render)
    if (m_svgCacheScale == m_scale)
        return;

    // Render entire SVG at current scale
    const int cacheW = static_cast<int>(std::ceil(m_svgDefaultSize.width() * m_scale));
    const int cacheH = static_cast<int>(std::ceil(m_svgDefaultSize.height() * m_scale));
    if (cacheW <= 0 || cacheH <= 0)
        return;

    // Cap at 16384x16384 (~1GB) to prevent OOM at extreme zoom
    constexpr int kMaxCacheDim = 16384;
    if (cacheW > kMaxCacheDim || cacheH > kMaxCacheDim)
        return;

    m_svgPixmapCache = QPixmap(cacheW, cacheH);
    m_svgPixmapCache.fill(Qt::transparent);
    {
        QPainter painter(&m_svgPixmapCache);
        painter.setRenderHint(QPainter::Antialiasing);
        m_svgRenderer->render(&painter);
    }

    m_svgCacheScale = m_scale;
    update();
}

static QString formatFileSize(qint64 bytes)
{
    if (bytes < 1024)
        return QString::number(bytes) + QStringLiteral(" B");
    if (bytes < 1024 * 1024)
        return QString::number(bytes / 1024.0, 'f', 1) + QStringLiteral(" KB");
    if (bytes < 1024LL * 1024 * 1024)
        return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + QStringLiteral(" MB");
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + QStringLiteral(" GB");
}

void ImageView::drawInfoPanel(QPainter& painter)
{
    if (m_imageInfo.filePath.isEmpty())
        return;

    QStringList lines;
    lines << m_imageInfo.filePath;
    if (m_imageInfo.fileSize > 0)
        lines << QStringLiteral("大小: %1").arg(formatFileSize(m_imageInfo.fileSize));
    if (m_imageInfo.pixelWidth > 0 && m_imageInfo.pixelHeight > 0)
        lines << QStringLiteral("像素: %1 × %2").arg(m_imageInfo.pixelWidth).arg(m_imageInfo.pixelHeight);
    if (!m_imageInfo.format.isEmpty())
        lines << QStringLiteral("格式: %1").arg(m_imageInfo.format);
    if (m_imageInfo.bitDepth > 0)
        lines << QStringLiteral("色深: %1 bit").arg(m_imageInfo.bitDepth);
    if (!m_imageInfo.lastModified.isEmpty())
        lines << QStringLiteral("修改: %1").arg(m_imageInfo.lastModified);
    lines << QStringLiteral("缩放: %1%").arg(static_cast<int>(m_scale * 100));

    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

    const QFontMetrics fm(font);
    const int lineHeight = fm.height() + 2;
    const int padding = 8;

    int maxWidth = 0;
    for (const auto& line : lines)
        maxWidth = std::max(maxWidth, fm.horizontalAdvance(line));

    const int panelW = maxWidth + padding * 2;
    const int panelH = lineHeight * lines.size() + padding * 2;
    const int panelX = padding;
    const int panelY = height() - panelH - padding;

    // Semi-transparent background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.drawRoundedRect(panelX, panelY, panelW, panelH, 4, 4);

    // Text
    painter.setPen(QColor(220, 220, 220));
    int y = panelY + padding + fm.ascent();
    for (const auto& line : lines) {
        painter.drawText(panelX + padding, y, line);
        y += lineHeight;
    }
}

void ImageView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        for (const auto& url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void ImageView::dropEvent(QDropEvent* event)
{
    for (const auto& url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            emit fileDropped(url.toLocalFile());
            event->acceptProposedAction();
            return;
        }
    }
}

} // namespace simplepic
