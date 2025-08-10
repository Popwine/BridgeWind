#include "aspectratiobutton.h"

#include <QStyle>
#include <QLayout>
// 构造函数，默认设置宽高比为 1:1 (正方形)
AspectRatioButton::AspectRatioButton(QWidget* parent)
    : QPushButton(parent), m_aspectRatio(1.0)
{
    m_parent = parent;
}

AspectRatioButton::AspectRatioButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent), m_aspectRatio(1.0)
{
}

void AspectRatioButton::setAspectRatio(qreal ratio)
{
    if (ratio > 0) {
        m_aspectRatio = ratio;
        // 强制更新一下尺寸
        updateGeometry();
    }
}

void AspectRatioButton::setAspectRatio(int w, int h)
{
    if (w > 0 && h > 0) {
        setAspectRatio(static_cast<qreal>(w) / h);
    }
}


// 这是所有魔法发生的地方
void AspectRatioButton::resizeEvent(QResizeEvent* event)
{
    // 首先，调用基类的实现
    QPushButton::resizeEvent(event);

    // 获取 resize 事件提供的新尺寸
    QSize newSize = event->size();

    // 计算基于当前宽度，理想的高度应该是多少
    int idealHeight = static_cast<int>(newSize.width() / m_aspectRatio);

    newSize.setHeight(idealHeight);
    
    setMinimumHeight(idealHeight);
    //if (newSize != this->size()) {
    //    QRect newGeometry(this->pos(), newSize);
    //    this->setGeometry(newGeometry);
    //}
    //int padding = static_cast<int>(qMin(this->width(), this->height()) * 0.1);
    //this->setIconSize(QSize(this->width() - padding, this->height() - padding));
    if (m_parent && parentWidget()->layout()) {
        // 1. 直接让布局失效，清除所有缓存
        m_parent->layout()->invalidate();

        // 2. 激活布局，强制它立即重新计算
        //   （在很多情况下，仅 invalidate() 然后等待下一个事件循环就足够了）
        m_parent->layout()->activate();
    }
}
bool AspectRatioButton::hasHeightForWidth() const
{
    return true;
}

// 实现核心计算逻辑
int AspectRatioButton::heightForWidth(int width) const
{
    // 根据给定的宽度和存储的宽高比，计算出理想的高度
    if (m_aspectRatio <= 0) {
        return QPushButton::heightForWidth(width); // 如果比例无效，使用基类行为
    }
    return static_cast<int>(width / m_aspectRatio);
}
QSize AspectRatioButton::sizeHint() const
{
    // 获取基类（QPushButton）的默认建议尺寸
    QSize hint = QPushButton::sizeHint();

    // 基于基类建议的宽度，来计算我们期望的高度
    int width = hint.width();
    int height = heightForWidth(width);

    return QSize(width, height);
}