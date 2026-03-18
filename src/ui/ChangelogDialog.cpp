#include "ChangelogDialog.h"
#include "version.h"

#include <QFile>
#include <QFont>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTextEdit>
#include <QVBoxLayout>

namespace simplepic {

// ============== CloseButton ==============

CloseButton::CloseButton(const QString& text, QWidget* parent)
    : QWidget(parent)
    , m_text(text)
{
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(36);
    setMinimumWidth(88);
}

QSize CloseButton::sizeHint() const
{
    return {88, 36};
}

void CloseButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QColor kNormal(70, 130, 230);
    const QColor kHover(90, 150, 250);
    const QColor kPress(55, 110, 200);

    QColor bg = m_pressed ? kPress : (m_hovered ? kHover : kNormal);

    QPainterPath path;
    path.addRoundedRect(QRectF(rect()), 6, 6);
    p.fillPath(path, bg);

    p.setPen(Qt::white);
    QFont f = font();
    f.setPixelSize(14);
    p.setFont(f);
    p.drawText(rect(), Qt::AlignCenter, m_text);
}

void CloseButton::enterEvent(QEvent* /*event*/)
{
    m_hovered = true;
    update();
}

void CloseButton::leaveEvent(QEvent* /*event*/)
{
    m_hovered = false;
    m_pressed = false;
    update();
}

void CloseButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
    }
}

void CloseButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        update();
        if (rect().contains(event->pos())) {
            emit clicked();
        }
    }
}

// ============== ChangelogDialog ==============

ChangelogDialog::ChangelogDialog(QWidget* parent)
    : QDialog(parent)
{
    applyDarkPalette();
    setupUI();
}

void ChangelogDialog::applyDarkPalette()
{
    QPalette pal;
    const QColor kBg(38, 38, 42);
    const QColor kText(220, 220, 225);
    const QColor kBase(30, 30, 34);
    const QColor kHighlight(70, 130, 230);

    pal.setColor(QPalette::Window, kBg);
    pal.setColor(QPalette::WindowText, kText);
    pal.setColor(QPalette::Base, kBase);
    pal.setColor(QPalette::Text, kText);
    pal.setColor(QPalette::Highlight, kHighlight);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    setPalette(pal);
}

void ChangelogDialog::setupUI()
{
    setWindowTitle(QStringLiteral("SimplePicture - 更新日志"));
    resize(560, 440);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);

    // Title
    m_titleLabel = new QLabel(this);
    m_titleLabel->setText(QStringLiteral("SimplePicture v%1").arg(EASYPIC_VERSION));
    QFont titleFont = m_titleLabel->font();
    titleFont.setPixelSize(22);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    QPalette titlePal = m_titleLabel->palette();
    titlePal.setColor(QPalette::WindowText, QColor(235, 235, 240));
    m_titleLabel->setPalette(titlePal);
    layout->addWidget(m_titleLabel);

    // Separator line (drawn via a thin widget)
    auto* separator = new QWidget(this);
    separator->setFixedHeight(1);
    separator->setAutoFillBackground(true);
    QPalette sepPal = separator->palette();
    sepPal.setColor(QPalette::Window, QColor(60, 60, 65));
    separator->setPalette(sepPal);
    layout->addWidget(separator);

    // Changelog text
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFrameShape(QFrame::NoFrame);
    m_textEdit->setAutoFillBackground(true);

    QPalette textPal = m_textEdit->palette();
    textPal.setColor(QPalette::Base, QColor(30, 30, 34));
    textPal.setColor(QPalette::Text, QColor(200, 200, 208));
    m_textEdit->setPalette(textPal);

    QFont textFont = m_textEdit->font();
    textFont.setPixelSize(14);
    m_textEdit->setFont(textFont);

    QFile file(QStringLiteral(":/CHANGELOG.md"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        m_textEdit->setMarkdown(QString::fromUtf8(file.readAll()));
#else
        m_textEdit->setPlainText(QString::fromUtf8(file.readAll()));
#endif
    }

    layout->addWidget(m_textEdit, 1);

    // Close button
    auto* closeBtn = new CloseButton(QStringLiteral("关闭"), this);
    connect(closeBtn, &CloseButton::clicked, this, &QDialog::accept);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);
}

void ChangelogDialog::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(38, 38, 42));
}

} // namespace simplepic
