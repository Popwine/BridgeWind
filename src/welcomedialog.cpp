#include "welcomedialog.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>
#include <QStringListModel>
#include <QStringList>
#include <QStandardItemModel>
#include <QPainter>
#include <QFontMetrics>
#include <QMessageBox>
#include "settingsdialog.h"
WelcomeDialog::WelcomeDialog(QWidget* parent)
	:
	QDialog(parent),
	ui(new Ui::WelcomeDialog)
{

	ui->setupUi(this);
	//ui->newProjectButton->setText("1");

	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::reject);
	connect(ui->newProjectButton, &QPushButton::clicked, this, &WelcomeDialog::onNewProjectButtonClicked);

	QIcon newIcon(":/icons/res/icons/welcome_dialog_new_project.svg");
	QIcon openIcon(":/icons/res/icons/welcome_dialog_open_project.svg");
	QIcon settingsIcon(":/icons/res/icons/settings.svg");
	ui->newProjectButton->setIcon(newIcon);
	ui->openProjectButton->setIcon(openIcon);
	ui->settingsButton->setIcon(settingsIcon);

	QStandardItemModel* model = new QStandardItemModel(this);
	m_historyManager = new ProjectHistoryManager(this);
	// --- 添加第一项 ---
	const auto& recentProjects = m_historyManager->getProjects();
	for (const auto& projectInfo : recentProjects) {
		QStandardItem* item = new QStandardItem();
		item->setData(QIcon(":/icons/res/icons/default_project_icon.svg"), Qt::DecorationRole); // 推荐使用资源路径
		item->setData(projectInfo.name, ProjectNameRole);
		item->setData(projectInfo.path, ProjectPathRole);
		model->appendRow(item);
	}
	


	QListView* listView = ui->recentProjectsList;
	listView->setModel(model);

	// 2. 创建并设置我们自定义的委托！
	ProjectItemDelegate* delegate = new ProjectItemDelegate(this);
	listView->setItemDelegate(delegate);
	listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	connect(listView, &QListView::clicked, this, &WelcomeDialog::onRecentProjectClicked);

	connect(ui->settingsButton, &QPushButton::clicked, this, &WelcomeDialog::onSettingsButtonClicked);
	connect(ui->smallSettingsButton, &QPushButton::clicked, this, &WelcomeDialog::onSettingsButtonClicked);


}


WelcomeDialog::~WelcomeDialog() {

}
QString WelcomeDialog::finalProjectPath() const {
	return m_finalProjectPath;
}
QString WelcomeDialog::finalProjectName() const {
	return m_finalProjectName;
}
void WelcomeDialog::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {

		if (ui->titleBar->rect().contains(event->pos())
			|| ui->logoWidget->rect().contains(event->pos())
			|| ui->blankWidget->rect().contains(event->pos())
			) {
			//qDebug() << "mouse pressed in drag area. position: " << event->globalPos();
			m_dragPosition = event->globalPos() - this->frameGeometry().topLeft();
			event->accept();
		}
	}
}
void WelcomeDialog::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {

		//qDebug() << "mouse hold in drag area. position: " << event->globalPos();
		this->move(event->globalPos() - m_dragPosition);
		event->accept();


	}
}
void WelcomeDialog::onNewProjectButtonClicked() {
	qDebug() << "onNewProjectButtonClicked";

	NewProjectDialog newDialog(this); // 以 WelcomeDialog 为父窗口

	if (
		newDialog.exec() 
		== QDialog::Accepted) {
		// 1. 从对话框获取结果
		QString name = newDialog.projectName();
		QString path = newDialog.workPath();
		QString projectFullPath = path ; // 假设项目文件夹是 path/name

		// 2. 更新历史记录
		m_historyManager->addProject(name, projectFullPath);

		// 3. 【关键】设置结果并关闭自己，通知 main 函数

		m_finalProjectPath = projectFullPath; // 需要在 .h 中添加 m_finalProjectPath 成员
		m_finalProjectName = name;
		this->accept();
	}
}

void WelcomeDialog::onRecentProjectClicked(const QModelIndex& index)
{
	// 1. 从模型中获取数据
	// 使用您之前定义的 Role 来获取对应的项目名和路径
	QString name = index.data(ProjectNameRole).toString();
	QString path = index.data(ProjectPathRole).toString();

	qDebug() << "Clicked on recent project:" << name << "at path:" << path;

	// 2. 设置最终结果，这和 onNewProjectButtonClicked 里的逻辑一样
	m_finalProjectName = name;
	m_finalProjectPath = path;

	// 3. 调用 accept() 来关闭欢迎对话框，并通知主窗口操作成功
	this->accept();
}
void WelcomeDialog::onSettingsButtonClicked() {
	SettingsDialog dialog(this);
	dialog.exec();
}
ProjectItemDelegate::ProjectItemDelegate(QObject* parent) {

}

ProjectItemDelegate::~ProjectItemDelegate() {

}


void ProjectItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)const {





	painter->save(); // 保存 painter 当前状态
	painter->setRenderHint(QPainter::Antialiasing, true); // 开启抗锯齿

	QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
	QString name = index.data(ProjectNameRole).toString();
	QString path = index.data(ProjectPathRole).toString();

	// 关键：从 widget 获取实际的调色板
	const QWidget* widget = option.widget;
	QPalette palette = widget ? widget->palette() : option.palette;

	if (option.state & QStyle::State_Selected) {
		// 使用调色板中的高亮背景色（这个颜色会被 QSS 修改）
		painter->fillRect(option.rect, palette.highlight());
		// 使用高亮文本色
		painter->setPen(palette.highlightedText().color());
	}
	else if (option.state & QStyle::State_MouseOver) {
		painter->fillRect(option.rect, palette.light());
		painter->setPen(palette.windowText().color());
	}
	else {
		painter->setPen(palette.windowText().color());
	}

	// --- 计算布局 ---
	// 定义一些常量，方便调整
	const int padding = 8;
	//const int iconSize = 48; // SVG图标大小
	const int iconWidth = 100;
	const int iconHeight = 48;
	QRect contentRect = option.rect.adjusted(padding, padding, -padding, -padding);

	// 1. 图标区域
	QRect iconRect(contentRect.left(), contentRect.top(), iconWidth, iconHeight);

	// 2. 文字区域 (在图标右侧)
	QRect textRect(iconRect.right() + padding, contentRect.top(),
		contentRect.width() - iconWidth - padding, contentRect.height());

	// --- 绘制内容 ---
	// 1. 绘制图标
	icon.paint(painter, iconRect);

	// 2. 绘制项目名称 (第一行)
	// 设置画笔颜色为标准文字颜色
	painter->setPen(option.palette.windowText().color());
	QFont nameFont = painter->font();
	nameFont.setBold(false);
	nameFont.setFamily("Microsoft YaHei UI");
	nameFont.setPointSize(12);
	painter->setFont(nameFont);
	// 使用 ElideRight 处理过长的文字
	QString elidedName = painter->fontMetrics().elidedText(name, Qt::ElideRight, textRect.width());
	painter->drawText(textRect, Qt::AlignTop | Qt::AlignLeft, elidedName);

	// 3. 绘制项目路径 (第二行)
	// 设置画笔颜色为灰色
	painter->setPen(Qt::gray);
	QFont pathFont = painter->font();
	pathFont.setBold(false); // 不加粗
	pathFont.setPointSize(9);
	painter->setFont(pathFont);
	QString elidedPath = painter->fontMetrics().elidedText(path, Qt::ElideRight, textRect.width());
	// 在下半部分绘制
	painter->drawText(textRect, Qt::AlignBottom | Qt::AlignLeft, elidedPath);

	painter->restore(); // 恢复 painter 状态
}
QSize ProjectItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
	Q_UNUSED(option);
	Q_UNUSED(index);
	// 返回一个固定的尺寸，确保足够的高度来容纳图标和两行文字
	// 高度 = 上下内边距 + 图标高度
	const int padding = 8;
	const int iconHeight = 48;
	return QSize(200, iconHeight + 2 * padding); // 宽度会被视图自动拉伸，高度是关键
}

NewProjectDialog::NewProjectDialog(QWidget* parent)
	:
	QDialog(parent, Qt::Window | Qt::FramelessWindowHint),
	ui(new Ui::NewProjectDialog)
{
	qDebug() << "NewProjectDialog";
	ui->setupUi(this);
	//setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	qDebug() << "Dialog isVisible:" << isVisible(); // 此时应为 false
	qDebug() << "Dialog size:" << size();
	// 假设背景控件名为 backgroundWidget
	qDebug() << "Background widget pointer:" << ui->welcomeBackgroundWidget;
	if (ui->welcomeBackgroundWidget) {
		qDebug() << "Background widget size:" << ui->welcomeBackgroundWidget->size();
	}
	
	connect(ui->browseButton, &QPushButton::clicked, this, &NewProjectDialog::onBrowseButtonClicked);
	connect(ui->okButton, &QPushButton::clicked, this, &NewProjectDialog::onOkButtonClicked);
	connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
	connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::reject);
}
QString NewProjectDialog::projectName() const { return ui->projectNameEdit->text(); }
QString NewProjectDialog::workPath() const { return ui->workPathEdit->text(); }

// 槽函数实现
void NewProjectDialog::onBrowseButtonClicked() {
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose new project work path"));
	if (!dir.isEmpty()) {
		ui->workPathEdit->setText(dir);
	}
}

void NewProjectDialog::onOkButtonClicked() {
	// 1. 数据校验
	if (projectName().isEmpty() || workPath().isEmpty()) {
		QMessageBox::warning(this, tr("Input error"), "Project name and working path cannot be empty!");
		return;
	}

	// 2. 创建项目文件夹
	QDir dir(workPath());
	if (!dir.exists()) {
		dir.mkpath(".");
	}

	// 3. 一切成功，调用 accept() 关闭对话框并返回 Accepted 状态
	this->accept();
}

NewProjectDialog::~NewProjectDialog() {
	qDebug() << "~NewProjectDialog";
}

