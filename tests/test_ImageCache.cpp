#include <gtest/gtest.h>

#include <QPixmap>

#include "core/ImageCache.h"

using namespace simplepic;

class ImageCacheTest : public ::testing::Test {
protected:
    // Helper to create a dummy QPixmap with a specific color
    static QPixmap makePixmap(int w = 10, int h = 10)
    {
        QImage img(w, h, QImage::Format_RGB32);
        img.fill(Qt::red);
        return QPixmap::fromImage(img);
    }
};

TEST_F(ImageCacheTest, DefaultCapacity)
{
    ImageCache cache;
    EXPECT_EQ(cache.capacity(), ImageCache::kDefaultCapacity);
    EXPECT_EQ(cache.capacity(), 20);
}

TEST_F(ImageCacheTest, CustomCapacity)
{
    ImageCache cache(5);
    EXPECT_EQ(cache.capacity(), 5);
}

TEST_F(ImageCacheTest, PutAndGet)
{
    ImageCache cache;
    QPixmap px = makePixmap();
    cache.put("test.png", px);

    QPixmap result = cache.get("test.png");
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.width(), 10);
    EXPECT_EQ(result.height(), 10);
}

TEST_F(ImageCacheTest, GetMissReturnsNull)
{
    ImageCache cache;
    QPixmap result = cache.get("nonexistent.png");
    EXPECT_TRUE(result.isNull());
}

TEST_F(ImageCacheTest, ContainsCheck)
{
    ImageCache cache;
    EXPECT_FALSE(cache.contains("test.png"));

    cache.put("test.png", makePixmap());
    EXPECT_TRUE(cache.contains("test.png"));
    EXPECT_FALSE(cache.contains("other.png"));
}

TEST_F(ImageCacheTest, LRUEviction)
{
    ImageCache cache(3);

    cache.put("a.png", makePixmap());
    cache.put("b.png", makePixmap());
    cache.put("c.png", makePixmap());
    EXPECT_EQ(cache.size(), 3);

    // Adding a 4th item should evict "a.png" (oldest/LRU)
    cache.put("d.png", makePixmap());
    EXPECT_EQ(cache.size(), 3);
    EXPECT_FALSE(cache.contains("a.png"));
    EXPECT_TRUE(cache.contains("b.png"));
    EXPECT_TRUE(cache.contains("c.png"));
    EXPECT_TRUE(cache.contains("d.png"));
}

TEST_F(ImageCacheTest, AccessUpdatesOrder)
{
    ImageCache cache(3);

    cache.put("a.png", makePixmap());
    cache.put("b.png", makePixmap());
    cache.put("c.png", makePixmap());

    // Access "a.png" to move it to front (MRU)
    cache.get("a.png");

    // Now add "d.png" — should evict "b.png" (LRU), not "a.png"
    cache.put("d.png", makePixmap());
    EXPECT_TRUE(cache.contains("a.png"));
    EXPECT_FALSE(cache.contains("b.png"));
    EXPECT_TRUE(cache.contains("c.png"));
    EXPECT_TRUE(cache.contains("d.png"));
}

TEST_F(ImageCacheTest, UpdateExistingEntry)
{
    ImageCache cache;
    QPixmap px1 = makePixmap(10, 10);
    QPixmap px2 = makePixmap(20, 20);

    cache.put("test.png", px1);
    EXPECT_EQ(cache.size(), 1);
    EXPECT_EQ(cache.get("test.png").width(), 10);

    cache.put("test.png", px2);
    EXPECT_EQ(cache.size(), 1); // size should not increase
    EXPECT_EQ(cache.get("test.png").width(), 20);
}

TEST_F(ImageCacheTest, Clear)
{
    ImageCache cache;
    cache.put("a.png", makePixmap());
    cache.put("b.png", makePixmap());
    cache.put("c.png", makePixmap());
    EXPECT_EQ(cache.size(), 3);

    cache.clear();
    EXPECT_EQ(cache.size(), 0);
    EXPECT_FALSE(cache.contains("a.png"));
    EXPECT_FALSE(cache.contains("b.png"));
    EXPECT_FALSE(cache.contains("c.png"));
}

TEST_F(ImageCacheTest, SizeTracking)
{
    ImageCache cache;
    EXPECT_EQ(cache.size(), 0);

    cache.put("a.png", makePixmap());
    EXPECT_EQ(cache.size(), 1);

    cache.put("b.png", makePixmap());
    EXPECT_EQ(cache.size(), 2);

    cache.put("c.png", makePixmap());
    EXPECT_EQ(cache.size(), 3);

    // Updating existing entry should not change size
    cache.put("b.png", makePixmap());
    EXPECT_EQ(cache.size(), 3);

    cache.clear();
    EXPECT_EQ(cache.size(), 0);
}
