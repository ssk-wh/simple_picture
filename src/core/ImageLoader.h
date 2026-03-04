#pragma once

#include <QObject>
#include <QPixmap>
#include <QString>
#include <memory>

namespace easypic {

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

    // 支持的图片格式过滤器列表
    static QStringList supportedFilters();

signals:
    void imageLoaded(const QString& filePath, const QPixmap& pixmap);
    void loadFailed(const QString& filePath, const QString& error);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace easypic
