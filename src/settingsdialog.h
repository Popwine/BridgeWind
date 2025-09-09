#pragma once

#include <QDialog>
#include "ui_settingsdialog.h"
#include "SettingsManager.h"
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    // 构造函数接收当前的语言代码，用于初始化ComboBox
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();


private slots:
    void onApplyButtonClicked();
    void onOkButtonClicked();
    // onLanguageChanged 槽不再需要，因为我们只在点击按钮时应用更改
    
private:
    Ui::SettingsDialog* ui;
};