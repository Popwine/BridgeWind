#pragma once
#ifndef FILEDROPWIDGET_H
#define FILEDROPWIDGET_H

#include <QWidget>
#include <QStringList>

// 向前声明，避免不必要的 #include
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

class FileDropWidget : public QWidget
{
    Q_OBJECT // 必须包含，以便使用信号和槽

public:
    explicit FileDropWidget(QWidget* parent = nullptr);

signals:
    // 当文件成功拖入时，发射此信号，并携带所有文件的路径列表
    void filesDropped(const QStringList& filePaths);

protected:
    // 重写 QWidget 的这四个事件处理函数
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    // 我们将重写 paintEvent 来提供视觉反馈
    void paintEvent(QPaintEvent* event) override;

private:
    // 一个标志位，用于判断当前是否有文件正悬停在控件上
    bool m_isDragOver;
};

#endif // FILEDROPWIDGET_H