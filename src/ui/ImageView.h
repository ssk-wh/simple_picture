#pragma once

#include <QMimeData>
#include <QPixmap>
#include <QPointF>
#include <QTimer>
#include <QWidget>
#include <memory>

struct ImageInfo {
    QString filePath;
    qint64 fileSize = 0;       // bytes
    int pixelWidth = 0;
    int pixelHeight = 0;
    int bitDepth = 0;
    QString format;
    QString lastModified;
};

class QSvgRenderer;

namespace simplepic {

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
    void setError(const QString& message);
    void setImageInfo(const ImageInfo& info);
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
    void drawInfoPanel(QPainter& painter);

    ImageInfo m_imageInfo;
    bool m_showInfo = false;

    QPixmap m_pixmap;
    QPixmap m_svgOverview;       // small full-SVG pixmap for zoom animation
    QPixmap m_svgPixmapCache;    // full SVG rendered at current scale
    double m_svgCacheScale = 0.0;
    QTimer m_svgCacheTimer;
    std::unique_ptr<QSvgRenderer> m_svgRenderer;
    QSizeF m_svgDefaultSize;
    double m_scale = 1.0;
    QPointF m_offset;
    QPointF m_lastMousePos;
    bool m_dragging = false;
    bool m_fittedToWindow = true;  // Track whether we're in fit-to-window mode
    QString m_errorMessage;
    QColor m_backgroundColor{30, 30, 30};
};

} // namespace simplepic
