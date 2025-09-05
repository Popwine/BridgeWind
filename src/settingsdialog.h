#pragma once
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }

QT_END_NAMESPACE


class SettingsDialog : public QDialog {
	Q_OBJECT
public:
	explicit SettingsDialog(QWidget* parent = nullptr);
	~SettingsDialog();

private:
	Ui::SettingsDialog* ui;
};
#endif // SETTINGSDIALOG_H