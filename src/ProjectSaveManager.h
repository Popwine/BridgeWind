#pragma once
#ifndef PROJECT_SAVE_MANAGER_H
#define PROJECT_SAVE_MANAGER_H



#include <QObject>
#include <QString>
#include <QList>

class ProjectSaveManager : public QObject {
	Q_OBJECT
public:

	explicit ProjectSaveManager(QObject* parent = nullptr);
	
};
#endif // PROJECT_SAVE_MANAGER_H