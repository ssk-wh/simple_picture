#pragma once

#include <QMimeData>
#include <QPixmap>
#include <QPointF>
#include <QTimer>
#include <QWidget>
#include <memory>

class QSvgRenderer;

namespace easypic {

class ImageView : public QWidget {
    Q_OBJECT

public:
    static constexpr double kMinScale = 0.01;
    static constexpr double kMaxScale = 100.0;
    static constexpr double kScaleStep = 1.1;

    explicit ImageView(QWidget* parent = nullptr);
    ~ImageView() override;

    void setPixmap(const QPixmap& pixmap);
    void setSvg(const QString& filePath);
    const QPixmap& pixmap() const;

    double scale() const;
    void setScale(double scale);
    void fitToWindow();
    void resetToOriginalSize();

signals:
    void nextImageRequested();
    void previousImageRequested();
    void fileDropped(const QString& filePath);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void clampOffset();
    void updateSvgCache();

    QPixmap m_pixmap;
    QPixmap m_svgOverview;       // small full-SVG pixmap for zoom animation
    QPixmap m_svgPixmapCache;    // viewport-sized cache for crisp display
    double m_svgCacheScale = 0.0;
    QPointF m_svgCacheOffset;
    QSize m_svgCacheViewport;
    QTimer m_svgCacheTimer;
    std::unique_ptr<QSvgRenderer> m_svgRenderer;
    QSizeF m_svgDefaultSize;
    double m_scale = 1.0;
    QPointF m_offset;
    QPointF m_lastMousePos;
    bool m_dragging = false;
    bool m_fittedToWindow = true;  // Track whether we're in fit-to-window mode
    QColor m_backgroundColor{30, 30, 30};
};

} // namespace easypic
