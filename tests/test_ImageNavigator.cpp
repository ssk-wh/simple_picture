#include <gtest/gtest.h>

#include <QImage>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "core/ImageNavigator.h"

using namespace easypic;

class ImageNavigatorTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_navigator = std::make_unique<ImageNavigator>();
    }

    // Create a temporary directory with N test PNG images named a.png, b.png, c.png, ...
    void createTestImages(QTemporaryDir& tmpDir, int count)
    {
        QImage img(10, 10, QImage::Format_RGB32);
        img.fill(Qt::red);
        for (int i = 0; i < count; ++i) {
            QString name = QString(QChar('a' + i)) + ".png";
            QString path = tmpDir.path() + "/" + name;
            img.save(path, "PNG");
        }
    }

    std::unique_ptr<ImageNavigator> m_navigator;
};

TEST_F(ImageNavigatorTest, InitialState)
{
    EXPECT_EQ(m_navigator->currentIndex(), -1);
    EXPECT_EQ(m_navigator->totalCount(), 0);
    EXPECT_TRUE(m_navigator->currentFile().isEmpty());
}

TEST_F(ImageNavigatorTest, SetCurrentFile)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    createTestImages(tmpDir, 3);

    QString filePath = tmpDir.path() + "/b.png";
    m_navigator->setCurrentFile(filePath);

    EXPECT_EQ(m_navigator->totalCount(), 3);
    EXPECT_GE(m_navigator->currentIndex(), 0);
    EXPECT_FALSE(m_navigator->currentFile().isEmpty());
}

TEST_F(ImageNavigatorTest, SetInvalidFile)
{
    // Setting a nonexistent file should not crash
    m_navigator->setCurrentFile(QStringLiteral("C:/nonexistent/path/image.png"));
    EXPECT_EQ(m_navigator->currentIndex(), -1);
    EXPECT_EQ(m_navigator->totalCount(), 0);
}

TEST_F(ImageNavigatorTest, GoNext)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    createTestImages(tmpDir, 3);

    m_navigator->setCurrentFile(tmpDir.path() + "/a.png");
    int initialIndex = m_navigator->currentIndex();

    bool result = m_navigator->goNext();
    EXPECT_TRUE(result);
    EXPECT_EQ(m_navigator->currentIndex(), initialIndex + 1);
}

TEST_F(ImageNavigatorTest, GoPrevious)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    createTestImages(tmpDir, 3);

    m_navigator->setCurrentFile(tmpDir.path() + "/c.png");
    int initialIndex = m_navigator->currentIndex();

    bool result = m_navigator->goPrevious();
    EXPECT_TRUE(result);
    EXPECT_EQ(m_navigator->currentIndex(), initialIndex - 1);
}

TEST_F(ImageNavigatorTest, BoundaryNext)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    createTestImages(tmpDir, 3);

    // Navigate to last image
    m_navigator->setCurrentFile(tmpDir.path() + "/c.png");
    int lastIndex = m_navigator->currentIndex();

    bool result = m_navigator->goNext();
    EXPECT_FALSE(result);
    EXPECT_EQ(m_navigator->currentIndex(), lastIndex);
}

TEST_F(ImageNavigatorTest, BoundaryPrevious)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    createTestImages(tmpDir, 3);

    // Navigate to first image
    m_navigator->setCurrentFile(tmpDir.path() + "/a.png");
    EXPECT_EQ(m_navigator->currentIndex(), 0);

    bool result = m_navigator->goPrevious();
    EXPECT_FALSE(result);
    EXPECT_EQ(m_navigator->currentIndex(), 0);
}

TEST_F(ImageNavigatorTest, JumpTo)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    createTestImages(tmpDir, 5);

    m_navigator->setCurrentFile(tmpDir.path() + "/a.png");

    bool result = m_navigator->jumpTo(3);
    EXPECT_TRUE(result);
    EXPECT_EQ(m_navigator->currentIndex(), 3);
}

TEST_F(ImageNavigatorTest, JumpToInvalid)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    createTestImages(tmpDir, 3);

    m_navigator->setCurrentFile(tmpDir.path() + "/a.png");

    EXPECT_FALSE(m_navigator->jumpTo(-1));
    EXPECT_FALSE(m_navigator->jumpTo(100));
    EXPECT_EQ(m_navigator->currentIndex(), 0); // unchanged
}

TEST_F(ImageNavigatorTest, FileAt)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    createTestImages(tmpDir, 3);

    m_navigator->setCurrentFile(tmpDir.path() + "/a.png");

    // fileAt should return valid paths for valid indices
    for (int i = 0; i < m_navigator->totalCount(); ++i) {
        EXPECT_FALSE(m_navigator->fileAt(i).isEmpty());
    }

    // fileAt should return empty for invalid indices
    EXPECT_TRUE(m_navigator->fileAt(-1).isEmpty());
    EXPECT_TRUE(m_navigator->fileAt(100).isEmpty());
}

TEST_F(ImageNavigatorTest, CurrentFileChanged)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    createTestImages(tmpDir, 3);

    QSignalSpy spy(m_navigator.get(), &ImageNavigator::currentFileChanged);

    m_navigator->setCurrentFile(tmpDir.path() + "/a.png");
    EXPECT_EQ(spy.count(), 1);

    // Verify signal arguments: filePath, index, total
    QList<QVariant> args = spy.takeFirst();
    EXPECT_FALSE(args.at(0).toString().isEmpty());
    EXPECT_EQ(args.at(1).toInt(), m_navigator->currentIndex());
    EXPECT_EQ(args.at(2).toInt(), m_navigator->totalCount());

    // goNext should also emit the signal
    m_navigator->goNext();
    EXPECT_EQ(spy.count(), 1);

    // goPrevious should also emit the signal
    m_navigator->goPrevious();
    EXPECT_EQ(spy.count(), 2);
}
