#include "ImageCache.h"

#include <QMutexLocker>

namespace easypic {

ImageCache::ImageCache(int capacity)
    : m_capacity(capacity)
{
}

ImageCache::~ImageCache() = default;

QPixmap ImageCache::get(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);

    auto it = m_cacheMap.find(filePath);
    if (it == m_cacheMap.end()) {
        return QPixmap();
    }

    // Move accessed item to front (MRU)
    m_cacheList.splice(m_cacheList.begin(), m_cacheList, it->second);
    return it->second->second;
}

void ImageCache::put(const QString& filePath, const QPixmap& pixmap)
{
    QMutexLocker locker(&m_mutex);

    auto it = m_cacheMap.find(filePath);

    if (it != m_cacheMap.end()) {
        // Update existing entry and move to front
        it->second->second = pixmap;
        m_cacheList.splice(m_cacheList.begin(), m_cacheList, it->second);
        return;
    }

    // Evict if at capacity
    while (static_cast<int>(m_cacheMap.size()) >= m_capacity) {
        evict();
    }

    // Insert new entry at front
    m_cacheList.emplace_front(filePath, pixmap);
    m_cacheMap[filePath] = m_cacheList.begin();
}

bool ImageCache::contains(const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);
    return m_cacheMap.find(filePath) != m_cacheMap.end();
}

void ImageCache::clear()
{
    QMutexLocker locker(&m_mutex);
    m_cacheList.clear();
    m_cacheMap.clear();
}

int ImageCache::size() const
{
    QMutexLocker locker(&m_mutex);
    return static_cast<int>(m_cacheMap.size());
}

int ImageCache::capacity() const
{
    return m_capacity;
}

void ImageCache::evict()
{
    // Remove LRU item (back of list)
    if (m_cacheList.empty()) {
        return;
    }
    auto& back = m_cacheList.back();
    m_cacheMap.erase(back.first);
    m_cacheList.pop_back();
}

} // namespace easypic
