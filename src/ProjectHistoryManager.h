#pragma once
#ifndef PROJECT_HISTORY_MANAGER_H
#define PROJECT_HISTORY_MANAGER_H

#include <QObject>
#include <QString>
#include <QList>


struct ProjectInfo {
    QString name;
    QString path;
    // 未来可以扩展，如 QDateTime lastOpened;
};

class ProjectHistoryManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectHistoryManager(QObject* parent = nullptr);

    // 添加一个新项目到历史记录（如果已存在，则移到最前）
    void addProject(const QString& name, const QString& path);

    // 获取所有最近项目记录
    const QList<ProjectInfo>& getProjects() const;

    // (可选) 清除所有历史记录
    void clearHistory();

private:
    void load(); // 从文件中加载
    void save(); // 保存到文件

    QList<ProjectInfo> m_projects; // 内存中存储的项目列表
    QString m_storagePath;         // 配置文件的完整路径
    const int m_maxHistoryCount = 10; // 最多记录10条
};









#endif // PROJECT_HISTORY_MANAGER_H