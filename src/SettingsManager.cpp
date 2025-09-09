#include "SettingsManager.h"


#include <QSettings>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QLocale>
#include <QStringList>
SettingsManager::SettingsManager(QObject* parent) : QObject(parent)
{
    // 确定配置文件的存储位置
    // 使用 QStandardPaths 获取一个更通用的、适合存放数据的目录
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath("."); // 如果目录不存在，则创建它
    }
    m_storagePath = configDir + "/settings.json";
    qDebug() << m_storagePath;
    // 程序启动时，自动加载历史记录
    bool isLoaded = load();
	if (isLoaded)
	{
		qDebug() << "Settings loaded successfully.";
	}
	else
	{
		qDebug() << "Failed to load settings. Using default settings.";
		setAsSystemLanguage();
	}
}
void SettingsManager::save() {
	QJsonObject rootObj;
	rootObj["language"] = m_language;
	QJsonDocument jsonDoc(rootObj);
	QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);
	QFile file(m_storagePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qWarning() << "无法打开文件进行写入:" << m_storagePath;
		return;
	}
	file.write(jsonData);
	file.close();
}
bool SettingsManager::load() {
    QFileInfo fileInfo(m_storagePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qDebug() << "File doesn't exit or invalid:" << m_storagePath;
        
        return false;
    }
    QFile file(m_storagePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Couldn't open file:" << m_storagePath;
        
        return false;
    }
    QByteArray fileContent = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileContent, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析错误:" << parseError.errorString() << "在偏移量:" << parseError.offset;
        
        return false;
    }

    if (jsonDoc.isNull() || (jsonDoc.isEmpty() && !jsonDoc.isArray() && !jsonDoc.isObject())) {
        qWarning() << "JSON文档为空或无效。";
        return false;
    }
    if (!jsonDoc.isObject()) {
        qWarning("JSON document is not an object.");
        return false;
    }
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("language") && rootObj["language"].isString()) {
        QString language = rootObj["language"].toString();
        qDebug() << "language in settings.json:" << language; // 输出: Username: "johndoe"
		m_language = language;
	}
	else {
		qWarning() << "JSON中缺少'language'字段或其类型不正确。";
        setAsSystemLanguage();
	}


	return true;
}

void SettingsManager::setAsSystemLanguage() {
    QStringList uiLanguages = QLocale::system().uiLanguages();
    qDebug() << "UI Languages (ordered by preferences):" << uiLanguages;

    if (!uiLanguages.isEmpty()) {
        qDebug() << "Firt UI Language:" << uiLanguages.first();
    }
    else {
		m_language = "en_US";
        return;
    }

    if (uiLanguages.first() == "en-US") {
		m_language = "en_US";
	}
	else if (uiLanguages.first().startsWith("zh")) {
		m_language = "zh_CN";
	}
    else if (uiLanguages.first().startsWith("de")) {
        m_language = "de_DE";
    }

    else {
        m_language = "en_US";
    }

	qDebug() << "Language set to \"" << m_language << "\" in SettingsManager";
}

