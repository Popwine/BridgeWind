#include "settingsdialog.h"
#include <QSettings> // 包含 QSettings
#include <QVariant>  // 包含 QVariant
#include <QMessageBox>
SettingsDialog::SettingsDialog(QWidget* parent)
    :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    auto& settingsManager = SettingsManager::instance();
	QString languageCode = settingsManager.getLanguage();
	if (languageCode == "en_US") {
		ui->languageComboBox->setCurrentIndex(0);
	}
	else if (languageCode == "zh_CN") {
		ui->languageComboBox->setCurrentIndex(1);
	}
	else if (languageCode == "de_DE") {
		ui->languageComboBox->setCurrentIndex(2);
	}
	else {
		ui->languageComboBox->setCurrentIndex(0); // 默认回到英语
	}

    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyButtonClicked);
    connect(ui->okButton, &QPushButton::clicked, this, &SettingsDialog::onOkButtonClicked);
    // 不再需要连接 onLanguageChanged
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::onApplyButtonClicked() {
    // 1. 获取当前 ComboBox 选中的语言代码 (我们存在 userData 里)
    int languageIndex = ui->languageComboBox->currentIndex();

    QString languageCode;

	if (languageIndex == -1) {
		// 没有选择任何语言，可能是个错误
		return;
	}
    else if (languageIndex == 0) {
        languageCode = "en_US";

	}
	else if (languageIndex == 1) {
		languageCode = "zh_CN";

	}
	else if (languageIndex == 2) {
		languageCode = "de_DE";
	}
	auto& settingsManager = SettingsManager::instance();
	if (settingsManager.getLanguage() != languageCode) {
		settingsManager.setLanguage(languageCode);
		settingsManager.save();
		if (languageCode == "en_US") {
			QMessageBox::information(this, "Info", "Language changed to English. Please restart the application to apply the changes.");
		}
		else if (languageCode == "zh_CN") {
			QMessageBox::information(this, "信息", "语言已更改为中文。请重启应用程序以应用更改。");
		}
		else if (languageCode == "de_DE") {
			QMessageBox::information(this, "Info", "Die Sprache wurde auf Deutsch geändert. Bitte starten Sie die Anwendung neu, um die Änderungen zu übernehmen.");
		}

	}
    //ui->applyButton->setEnabled(false);
}

void SettingsDialog::onOkButtonClicked() {
    // OK = Apply + Close
    onApplyButtonClicked();
    this->accept();
}