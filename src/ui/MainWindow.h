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

    void openFile(const QString& filePath);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onNextImage();
    void onPreviousImage();
    void onImageLoaded(const QString& filePath, const QPixmap& pixmap);
    void onCurrentFileChanged(const QString& filePath, int index, int total);

private:
    void setupUI();
    void connectSignals();
    void displayFile(const QString& filePath);
    void loadCurrentImage();
    void preloadNeighbors();
    void updateWindowTitle();
    void showChangelog();

    ImageView* m_imageView = nullptr;
    std::unique_ptr<ImageLoader> m_loader;
    std::unique_ptr<ImageCache> m_cache;
    std::unique_ptr<ImageNavigator> m_navigator;

    QString m_currentFile;
};

} // namespace simplepic
