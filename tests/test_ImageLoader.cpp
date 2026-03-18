#include <gtest/gtest.h>

#include <QImage>
#include <QPixmap>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "core/ImageLoader.h"

using namespace simplepic;

class ImageLoaderTest : public ::testing::Test {
protected:
    ImageLoader m_loader;
};

TEST_F(ImageLoaderTest, SupportedFilters)
{
    QStringList filters = ImageLoader::supportedFilters();
    EXPECT_FALSE(filters.isEmpty());
    EXPECT_TRUE(filters.contains("*.png"));
    EXPECT_TRUE(filters.contains("*.jpg"));
    EXPECT_TRUE(filters.contains("*.jpeg"));
    EXPECT_TRUE(filters.contains("*.bmp"));
    EXPECT_TRUE(filters.contains("*.gif"));
    EXPECT_TRUE(filters.contains("*.webp"));
    EXPECT_TRUE(filters.contains("*.tiff"));
    EXPECT_TRUE(filters.contains("*.tif"));
    EXPECT_TRUE(filters.contains("*.ico"));
}

TEST_F(ImageLoaderTest, LoadSyncInvalidPath)
{
    QPixmap result = m_loader.loadSync(QStringLiteral("C:/nonexistent/path/image.png"));
    EXPECT_TRUE(result.isNull());
}

TEST_F(ImageLoaderTest, LoadSyncEmptyPath)
{
    QPixmap result = m_loader.loadSync(QString());
    EXPECT_TRUE(result.isNull());
}

TEST_F(ImageLoaderTest, LoadSyncValidImage)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    QString filePath = tmpDir.path() + "/test_image.png";
    QImage testImage(10, 10, QImage::Format_RGB32);
    testImage.fill(Qt::red);
    ASSERT_TRUE(testImage.save(filePath, "PNG"));

    QPixmap result = m_loader.loadSync(filePath);
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.width(), 10);
    EXPECT_EQ(result.height(), 10);
}

TEST_F(ImageLoaderTest, LoadAsyncValidImage)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    QString filePath = tmpDir.path() + "/async_test.png";
    QImage testImage(10, 10, QImage::Format_RGB32);
    testImage.fill(Qt::blue);
    ASSERT_TRUE(testImage.save(filePath, "PNG"));

    QSignalSpy loadedSpy(&m_loader, &ImageLoader::imageLoaded);
    QSignalSpy failedSpy(&m_loader, &ImageLoader::loadFailed);

    m_loader.loadAsync(filePath);

    ASSERT_TRUE(loadedSpy.wait(5000));
    EXPECT_EQ(loadedSpy.count(), 1);
    EXPECT_EQ(failedSpy.count(), 0);

    QList<QVariant> args = loadedSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), filePath);
    QPixmap pixmap = args.at(1).value<QPixmap>();
    EXPECT_FALSE(pixmap.isNull());
}

TEST_F(ImageLoaderTest, LoadAsyncInvalidPath)
{
    QSignalSpy loadedSpy(&m_loader, &ImageLoader::imageLoaded);
    QSignalSpy failedSpy(&m_loader, &ImageLoader::loadFailed);

    m_loader.loadAsync(QStringLiteral("C:/nonexistent/path/image.png"));

    ASSERT_TRUE(failedSpy.wait(5000));
    EXPECT_EQ(failedSpy.count(), 1);
    EXPECT_EQ(loadedSpy.count(), 0);

    QList<QVariant> args = failedSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), QStringLiteral("C:/nonexistent/path/image.png"));
}

TEST_F(ImageLoaderTest, CancelAll)
{
    // cancelAll should not crash even with no pending tasks
    m_loader.cancelAll();

    // cancelAll should not crash after queuing async loads
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    QString filePath = tmpDir.path() + "/cancel_test.png";
    QImage testImage(10, 10, QImage::Format_RGB32);
    testImage.fill(Qt::green);
    ASSERT_TRUE(testImage.save(filePath, "PNG"));

    m_loader.loadAsync(filePath);
    m_loader.cancelAll();
    // Should not crash
}
