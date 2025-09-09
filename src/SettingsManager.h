#pragma once
#include <QObject>


class SettingsManager : public QObject {
	Q_OBJECT
private:
	explicit SettingsManager(QObject* parent = nullptr);

private:
	QString m_language;
public:
	QString getLanguage() const { return m_language; }
	void setLanguage(const QString& lang) { m_language = lang; }
	void save();
	static SettingsManager& instance() {
		static SettingsManager instance;
		return instance;
	}
private:
	QString m_storagePath;
	bool load();
	void setAsSystemLanguage();
	
	
};
