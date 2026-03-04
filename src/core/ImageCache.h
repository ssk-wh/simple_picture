#pragma once

#include <QMutex>
#include <QPixmap>
#include <QString>
#include <list>
#include <unordered_map>

namespace easypic {

class ImageCache {
public:
    static constexpr int kDefaultCapacity = 20;

    explicit ImageCache(int capacity = kDefaultCapacity);
    ~ImageCache();

    ImageCache(const ImageCache&) = delete;
    ImageCache& operator=(const ImageCache&) = delete;

    // 获取缓存的图片，未命中返回 null QPixmap
    QPixmap get(const QString& filePath);

    // 放入缓存，超出容量时淘汰最久未使用的条目
    void put(const QString& filePath, const QPixmap& pixmap);

    // 查询是否已缓存
    bool contains(const QString& filePath) const;

    // 清空缓存
    void clear();

    // 当前缓存条目数
    int size() const;

    // 缓存容量
    int capacity() const;

private:
    struct QStringHash {
        size_t operator()(const QString& s) const { return qHash(s); }
    };

    using CacheList = std::list<std::pair<QString, QPixmap>>;
    using CacheMap = std::unordered_map<QString, CacheList::iterator, QStringHash>;

    void evict();

    int m_capacity;
    CacheList m_cacheList;
    CacheMap m_cacheMap;
    mutable QMutex m_mutex;
};

} // namespace easypic
