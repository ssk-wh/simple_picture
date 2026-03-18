#pragma once

#include <QDialog>

class QLabel;
class QTextEdit;

namespace simplepic {

class CloseButton : public QWidget {
    Q_OBJECT

public:
    explicit CloseButton(const QString& text, QWidget* parent = nullptr);
    QSize sizeHint() const override;

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QString m_text;
    bool m_hovered = false;
    bool m_pressed = false;
};

class ChangelogDialog : public QDialog {
    Q_OBJECT

public:
    explicit ChangelogDialog(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
    void applyDarkPalette();

    QLabel* m_titleLabel = nullptr;
    QTextEdit* m_textEdit = nullptr;
};

} // namespace simplepic
