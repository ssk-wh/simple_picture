#include "ImageNavigator.h"
#include "ImageLoader.h"

#include <QCollator>
#include <QDir>
#include <QFileInfo>

namespace simplepic {

ImageNavigator::ImageNavigator(QObject* parent)
    : QObject(parent)
{
}

ImageNavigator::~ImageNavigator() = default;

void ImageNavigator::setCurrentFile(const QString& filePath)
{
    QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isFile()) {
        return;
    }

    QString absolutePath = fi.absoluteFilePath();
    QString dirPath = fi.absolutePath();

    scanDirectory(dirPath);

    // Locate the file in the sorted list
    int index = m_fileList.indexOf(absolutePath);
    if (index < 0) {
        return;
    }

    m_currentIndex = index;
    emit currentFileChanged(absolutePath, m_currentIndex, m_fileList.size());
}

QString ImageNavigator::currentFile() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_fileList.size()) {
        return QString();
    }
    return m_fileList.at(m_currentIndex);
}

int ImageNavigator::currentIndex() const
{
    return m_currentIndex;
}

int ImageNavigator::totalCount() const
{
    return m_fileList.size();
}

bool ImageNavigator::goNext()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_fileList.size() - 1) {
        return false;
    }
    m_currentIndex++;
    emit currentFileChanged(m_fileList.at(m_currentIndex), m_currentIndex, m_fileList.size());
    return true;
}

bool ImageNavigator::goPrevious()
{
    if (m_currentIndex <= 0) {
        return false;
    }
    m_currentIndex--;
    emit currentFileChanged(m_fileList.at(m_currentIndex), m_currentIndex, m_fileList.size());
    return true;
}

bool ImageNavigator::jumpTo(int index)
{
    if (index < 0 || index >= m_fileList.size()) {
        return false;
    }
    m_currentIndex = index;
    emit currentFileChanged(m_fileList.at(m_currentIndex), m_currentIndex, m_fileList.size());
    return true;
}

QString ImageNavigator::fileAt(int index) const
{
    if (index < 0 || index >= m_fileList.size()) {
        return QString();
    }
    return m_fileList.at(index);
}

QStringList ImageNavigator::fileList() const
{
    return m_fileList;
}

void ImageNavigator::scanDirectory(const QString& dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        m_fileList.clear();
        m_currentIndex = -1;
        return;
    }

    // Get supported image filters from ImageLoader
    QStringList filters = ImageLoader::supportedFilters();

    QStringList files = dir.entryList(filters, QDir::Files | QDir::Readable);

    // Natural sort using QCollator
    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(files.begin(), files.end(), collator);

    // Convert to absolute paths
    m_fileList.clear();
    m_fileList.reserve(files.size());
    for (const QString& file : files) {
        m_fileList.append(dir.absoluteFilePath(file));
    }

    m_currentIndex = -1;
}

} // namespace simplepic
