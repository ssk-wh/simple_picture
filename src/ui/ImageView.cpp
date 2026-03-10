#include "ImageView.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSvgRenderer>
#include <QUrl>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace easypic {

ImageView::ImageView(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);
}

ImageView::~ImageView() = default;

void ImageView::setPixmap(const QPixmap& pixmap)
{
    m_svgRenderer.reset();
    m_pixmap = pixmap;
    m_dragging = false;
    m_fittedToWindow = true;
    fitToWindow();
    update();
}

void ImageView::setSvg(const QString& filePath)
{
    auto renderer = std::make_unique<QSvgRenderer>(filePath);
    if (!renderer->isValid())
        return;

    m_svgRenderer = std::move(renderer);
    m_svgDefaultSize = m_svgRenderer->defaultSize();
    if (m_svgDefaultSize.isEmpty())
        m_svgDefaultSize = QSizeF(1024, 1024);

    // Set a dummy pixmap with the SVG's default size for scale/offset calculations
    m_pixmap = QPixmap(m_svgDefaultSize.toSize());
    m_pixmap.fill(Qt::transparent);

    m_dragging = false;
    m_fittedToWindow = true;
    fitToWindow();
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
        // Draw placeholder text
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.setPen(QColor(140, 140, 140));
        painter.drawText(rect(), Qt::AlignCenter,
                         QStringLiteral("Drag an image here or open via command line"));
        return;
    }

    // Draw the image
    const double scaledW = m_pixmap.width() * m_scale;
    const double scaledH = m_pixmap.height() * m_scale;
    const QRectF targetRect(m_offset.x(), m_offset.y(), scaledW, scaledH);

    if (m_svgRenderer) {
        painter.setRenderHint(QPainter::Antialiasing);
        m_svgRenderer->render(&painter, targetRect);
    } else {
        painter.drawPixmap(targetRect, m_pixmap, QRectF(m_pixmap.rect()));
    }
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
        // Image wider than viewport - clamp so at least visibleMin pixels remain visible
        const double minX = widgetW - scaledW - visibleMin;  // rightmost edge
        const double maxX = visibleMin;                       // leftmost edge
        m_offset.setX(std::clamp(m_offset.x(), minX, maxX));
    }

    if (scaledH <= widgetH) {
        // Image fits vertically - center it
        m_offset.setY((widgetH - scaledH) / 2.0);
    } else {
        // Image taller than viewport - clamp so at least visibleMin pixels remain visible
        const double minY = widgetH - scaledH - visibleMin;
        const double maxY = visibleMin;
        m_offset.setY(std::clamp(m_offset.y(), minY, maxY));
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

} // namespace easypic
