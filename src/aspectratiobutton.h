#pragma once
#ifndef ASPECTRATIOBUTTON_H
#define ASPECTRATIOBUTTON_H

#include <QPushButton>
#include <QResizeEvent>

class AspectRatioButton : public QPushButton
{
    Q_OBJECT

public:
    // 构造函数，可以接受一个宽高比作为参数
    explicit AspectRatioButton(QWidget* parent = nullptr);
    explicit AspectRatioButton(const QString& text, QWidget* parent = nullptr);

    // 设置宽高比 (宽度 / 高度)
    void setAspectRatio(qreal ratio);
    void setAspectRatio(int w, int h);
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    QSize sizeHint() const override;
protected:
    // 核心：重写 resizeEvent
    void resizeEvent(QResizeEvent* event) override;

private:
    qreal m_aspectRatio; // 存储目标的宽高比
    QWidget* m_parent;
};

#endif // ASPECTRATIOBUTTON_H