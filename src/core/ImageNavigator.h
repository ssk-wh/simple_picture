#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace simplepic {

class ImageNavigator : public QObject {
    Q_OBJECT

public:
    explicit ImageNavigator(QObject* parent = nullptr);
    ~ImageNavigator() override;

    // 扫描指定文件所在目录，定位到该文件
    void setCurrentFile(const QString& filePath);

    // 当前文件路径
    QString currentFile() const;

    // 当前索引（从 0 开始），无文件时返回 -1
    int currentIndex() const;

    // 文件总数
    int totalCount() const;

    // 导航操作
    bool goNext();
    bool goPrevious();
    bool jumpTo(int index);

    // 获取指定索引的文件路径
    QString fileAt(int index) const;

    // 获取文件列表
    QStringList fileList() const;

signals:
    void currentFileChanged(const QString& filePath, int index, int total);

private:
    void scanDirectory(const QString& dirPath);

    QStringList m_fileList;
    int m_currentIndex = -1;
};

} // namespace simplepic
