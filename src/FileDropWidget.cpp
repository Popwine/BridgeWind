#include "FileDropWidget.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QPainter>
#include <QPen>

FileDropWidget::FileDropWidget(QWidget* parent)
    : QWidget(parent), m_isDragOver(false)
{
    // !!! 最关键的一步：开启此控件的拖放功能 !!!
    setAcceptDrops(true);


}

void FileDropWidget::dragEnterEvent(QDragEnterEvent* event)
{
    // 检查拖动的数据是否包含 URL (通常代表文件)
    if (event->mimeData()->hasUrls()) {
        // 如果是文件，就接受这个动作，准备接收
        event->acceptProposedAction();

        // 更新标志位并重绘界面，以提供视觉反馈
        m_isDragOver = true;
        update(); // 触发 paintEvent
    }
    else {
        // 如果不是文件，就忽略
        event->ignore();
    }
}

void FileDropWidget::dragMoveEvent(QDragMoveEvent* event)
{
    // 在大部分情况下，如果 dragEnterEvent 中接受了，这里也需要接受
    event->acceptProposedAction();
}

void FileDropWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    // 当拖动离开时，恢复标志位并重绘界面
    m_isDragOver = false;
    update();
    event->accept();
}

void FileDropWidget::dropEvent(QDropEvent* event)
{
    // 获取拖放的数据
    const QMimeData* mimeData = event->mimeData();

    // 再次检查是否包含文件
    if (mimeData->hasUrls()) {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        // 将 QUrl 列表转换为本地文件路径的 QStringList
        for (const QUrl& url : urlList) {
            pathList.append(url.toLocalFile());
        }

        // 发射信号，将文件路径列表传递出去
        if (!pathList.isEmpty()) {
            emit filesDropped(pathList);
        }

        event->acceptProposedAction();
    }
    else {
        event->ignore();
    }

    // 恢复标志位并重绘界面
    m_isDragOver = false;
    update();
}

void FileDropWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event); // 避免编译器警告

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    
    if (m_isDragOver) {
        
        painter.setBrush(QColor("#dcdce1"));
        QPen pen(QColor("#6c84cb")); 
        pen.setStyle(Qt::DashLine);
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 20, 20);

        painter.setPen(QColor(0, 120, 215));

    }
    else {
        
        painter.setBrush(QColor("#dcdce1"));
        QPen pen(QColor("#CBD0DC"));
        pen.setStyle(Qt::DashLine);
        pen.setWidth(4);
        painter.setPen(pen);
        painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 20, 20);

        painter.setPen(Qt::gray);

    }
}