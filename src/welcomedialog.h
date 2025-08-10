#pragma once

#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include "ui_welcomedialog.h"
#include "ui_newprojectdialog.h"
#include "ProjectHistoryManager.h"
#include <QFileDialog>
#include <QDialog>
#include <QPoint>
class QMouseEvent;

QT_BEGIN_NAMESPACE
namespace Ui { class WelcomeDialog; }
namespace Ui { class NewProjectDialog; }// 1. 这是Qt的模板化声明
QT_END_NAMESPACE


class WelcomeDialog : public QDialog {
	Q_OBJECT


public:
	explicit WelcomeDialog(QWidget* parent = nullptr);
	~WelcomeDialog();
public:
	QString finalProjectPath() const;
	QString finalProjectName() const;
protected:
	
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
private:
	Ui::WelcomeDialog* ui;
	QPoint m_dragPosition;
	QString m_finalProjectPath;
	QString m_finalProjectName;
	ProjectHistoryManager* m_historyManager;
private slots:
	void onNewProjectButtonClicked();
	void onRecentProjectClicked(const QModelIndex& index);
};

#include <QStyledItemDelegate>
const int ProjectNameRole = Qt::UserRole + 1;
const int ProjectPathRole = Qt::UserRole + 2;

class ProjectItemDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	explicit ProjectItemDelegate(QObject* parent = nullptr);
	~ProjectItemDelegate();

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)const override;
	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;


};

class NewProjectDialog : public QDialog {
	Q_OBJECT
public:
	explicit NewProjectDialog(QWidget* parent = nullptr);
	~NewProjectDialog();
private:
	Ui::NewProjectDialog* ui;

public:
	QString projectName() const;
	QString workPath() const;

private slots:
	void onBrowseButtonClicked();
	void onOkButtonClicked();

};

#endif // ! WELCOMEDIALOG_H
