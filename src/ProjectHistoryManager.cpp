#include "ProjectHistoryManager.h"
#include <QSettings>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
ProjectHistoryManager::ProjectHistoryManager(QObject* parent) : QObject(parent)
{
    // 确定配置文件的存储位置
    // 使用 QStandardPaths 获取一个更通用的、适合存放数据的目录
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath("."); // 如果目录不存在，则创建它
    }
    m_storagePath = configDir + "/recent_projects.json";
    qDebug() << m_storagePath;
    // 程序启动时，自动加载历史记录
    load();
}

void ProjectHistoryManager::addProject(const QString& name, const QString& path)
{
    // 检查项目是否已存在，如果存在则先移除
    for (int i = 0; i < m_projects.size(); ++i) {
        if (m_projects[i].path == path) {
            m_projects.removeAt(i);
            break;
        }
    }

    // 将新项目添加到列表的最前面
    m_projects.prepend({ name, path });

    // 如果列表超过了最大数量，则移除末尾的旧项目
    while (m_projects.size() > m_maxHistoryCount) {
        m_projects.removeLast();
    }

    // 保存到文件
    save();
}

const QList<ProjectInfo>& ProjectHistoryManager::getProjects() const
{
    return m_projects;
}

void ProjectHistoryManager::clearHistory()
{
    m_projects.clear();
    save();
}

void ProjectHistoryManager::load()
{
    QFile file(m_storagePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // 文件不存在或无法打开，直接返回
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return; // 文件格式不正确
    }

    m_projects.clear();
    QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        QJsonObject obj = value.toObject();
        if (obj.contains("name") && obj.contains("path")) {
            m_projects.append({ obj["name"].toString(), obj["path"].toString() });
        }
    }
}

void ProjectHistoryManager::save()
{
    QJsonArray array;
    for (const auto& project : m_projects) {
        QJsonObject obj;
        obj["name"] = project.name;
        obj["path"] = project.path;
        array.append(obj);
    }

    QJsonDocument doc(array);
    QFile file(m_storagePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson());
    }
}