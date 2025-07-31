#include "mainwindow.h"

#include "bridge_simulation_service.h" // 包含头文件

#include "flow_data_reader.h"

#include <filesystem>
#include <algorithm> 

#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QThread>
#include <QDateTime>
#include <QDebug>
#include <QFormLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QButtonGroup>
#include <QMouseEvent>
#include <QWidget>

#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkQuad.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkProperty.h>
#include <vtkDoubleArray.h>
#include <vtkCellData.h>
#include <vtkContourFilter.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkTextProperty.h>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <WinUser.h>
#include <dwmapi.h>
#endif



MainWindow::MainWindow(const QString& projectName, const QString& projectPath, QWidget* parent)
:
    m_projectName(projectName), m_projectPath(projectPath), QMainWindow(parent), ui(new Ui::MainWindow)
{
    //setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->setupUi(this);
#if defined(Q_OS_WIN)
    // 获取窗口句柄
    HWND hwnd = (HWND)this->winId();
    // 扩展客户区以覆盖整个窗口
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
#endif 

    setupModel();

    setupUiFromModel();

    setupUiConnections();

    ui->sectionTypeComboBox->setCurrentIndex(0);
    onSectionTypeChanged(0);

    ui->projectNameLable->setText(projectName);


    setupTooglePairs();
    




}


// 步骤 3.1: 填充截面数据
void MainWindow::setupModel()
{
    m_sectionDefs.append({
        SectionTypes::Circle,
        "section_circle", "Circle",
        {
            {"param_diameters", "Diameter D(m)", 1.0}
        }
        });

    m_sectionDefs.append({
        SectionTypes::Rectangle,
        "section_rectangle", "Rectangle",
        {
            {"param_width", "Width W(m)", 8.0},
            {"param_height", "Height H(m)", 2.0}
        }
        });

    m_sectionDefs.append({
        SectionTypes::ChamferedRectangle,
        "section_chamfered_rectangle", "Chamfered Rectangle",
        {
            {"param_width", "Width W(m)", 4.0},
            {"param_height", "Height H(m)", 2.0},
            {"param_chamfered_radius", "Chamfered Radius R(m)", 0.1}
        }
        });
}

// 步骤 3.2: 根据数据模型动态创建UI
void MainWindow::setupUiFromModel()
{
    for (const auto& def : m_sectionDefs) {
        // A. 填充ComboBox，显示翻译后的文本，并绑定稳定的ID作为内部数据
        ui->sectionTypeComboBox->addItem(tr(def.nameComment), QVariant::fromValue(def.id));

        // B. 创建一个可滚动的区域，以防参数过多
        QScrollArea* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true); // 这一步至关重要！

        // C. 创建一个容器Widget，放置在滚动区内
        QWidget* contentWidget = new QWidget();

        // 使用QFormLayout可以自动对齐“标签:输入框”
        QVBoxLayout* vBoxLayout = new QVBoxLayout(contentWidget);
        
        // D. 遍历该截面的所有参数，并为每个参数创建标签和输入框
        for (const auto& param : def.parameters) {
            // 使用tr()翻译参数的键
            QLabel* label = new QLabel(tr(param.comment));
            QLineEdit* lineEdit = new QLineEdit(QString::number(param.defaultValue));
            auto font = label->font();
            font.setPointSize(10);


            label->setFont(font);
            lineEdit->setFont(font);


            // 设置验证器，只允许输入浮点数
            lineEdit->setValidator(new QDoubleValidator(0, 1e9, 4, this));
           
            vBoxLayout->addWidget(label);
            vBoxLayout->addWidget(lineEdit);
        }
        vBoxLayout->addStretch();
        // E. 将包含所有参数的contentWidget设置给滚动区

        
        scrollArea->setWidget(contentWidget);

        // F. 将配置好的整个滚动区作为一个页面，添加到StackedWidget中
        
        ui->parameterStack->addWidget(scrollArea);
    }
}


void MainWindow::setupUiConnections()
{
    // 连接 sectionTypeComboBox
    connect(ui->sectionTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onSectionTypeChanged);

    // --- 新增代码 ---
    // 1. 将关闭按钮的 clicked() 信号连接到窗口的 close() 槽
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::close);

    // 2. 将最小化按钮的 clicked() 信号连接到窗口的 showMinimized() 槽
    connect(ui->minimizeButton, &QPushButton::clicked, this, &MainWindow::showMinimized);

    // 3. 将最大化按钮的 clicked() 信号连接到我们自定义的 onMaximizeRestore() 槽
    connect(ui->maximizeButton, &QPushButton::clicked, this, &MainWindow::onMaximizeRestore);

    connect(ui->sectionTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onSectionTypeChanged);
    connect(ui->setGeometryButton, &QPushButton::clicked, this, &MainWindow::onSetGeometryButtonClicked);
    connect(ui->setBuiltInModeButton, &QPushButton::clicked, this, &MainWindow::onSetBuiltInModeButtonClicked);
    connect(ui->setImportDxfModeButton, &QPushButton::clicked, this, &MainWindow::onSetImportDxfModeButtonClicked);
    connect(ui->setParametersButton, &QPushButton::clicked, this, &MainWindow::onSetParametersButtonClicked);
}

void MainWindow::setupTooglePairs() {
    QButtonGroup* toggleGroupGeoOrSim = new QButtonGroup(this);
    toggleGroupGeoOrSim->addButton(ui->setGeometryButton);
    toggleGroupGeoOrSim->addButton(ui->setParametersButton);
    // 设置为互斥模式
    toggleGroupGeoOrSim->setExclusive(true);
    ui->setGeometryButton->setChecked(true);

    QButtonGroup* toggleGroupBuiltinOrDxf = new QButtonGroup(this);
    toggleGroupBuiltinOrDxf->addButton(ui->setBuiltInModeButton);
    toggleGroupBuiltinOrDxf->addButton(ui->setImportDxfModeButton);
    // 设置为互斥模式
    toggleGroupBuiltinOrDxf->setExclusive(true);
    ui->setBuiltInModeButton->setChecked(true);

    QButtonGroup* toggleGroupBuiltinOrDxfLine = new QButtonGroup(this);
    toggleGroupBuiltinOrDxfLine->addButton(ui->setImportDxfModeButtonLine);
    toggleGroupBuiltinOrDxfLine->addButton(ui->setBuiltInModeButtonLine);
    // 设置为互斥模式
    toggleGroupBuiltinOrDxfLine->setExclusive(true);
    ui->setBuiltInModeButtonLine->setChecked(true);

}
void MainWindow::onMaximizeRestore()
{
    if (isMaximized()) {
        showNormal(); // 如果当前是最大化状态，则恢复正常大小
    }
    else {
        showMaximized(); // 否则，最大化窗口
    }
}
MainWindow::~MainWindow()
{

}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
#if defined(Q_OS_WIN)
    if (eventType == "windows_generic_MSG")
    {
        MSG* msg = static_cast<MSG*>(message);

        // 1. 拦截窗口尺寸计算消息，欺骗系统客户区占满了整个窗口
        if (msg->message == WM_NCCALCSIZE)
        {
            // 当 wParam 为 TRUE 时，lParam 指向一个 NCCALCSIZE_PARAMS 结构体。
            // 通过返回 0，我们告诉 Windows 不要在窗口的非客户区（标题栏和边框）上进行任何绘制。
            // 这会有效地移除标准的窗口框架，但保留 DWM 提供的阴影和动画效果。
            if (msg->wParam == TRUE)
            {
                *result = 0;
                return true; // 我们已经处理了这个消息，不要再传递了
            }
        }

        // 2. 拦截鼠标命中测试消息，告诉系统鼠标在哪个区域
        if (msg->message == WM_NCHITTEST)
        {
            *result = 0;
            const LONG borderWidth = 8; // 窗口边框宽度，用于缩放
            RECT winrect;
            GetWindowRect(msg->hwnd, &winrect);

            long x = LOWORD(msg->lParam);
            long y = HIWORD(msg->lParam);

            // --- 关键改动：让按钮区域响应为普通客户区 ---
            QPoint pos = mapFromGlobal(QPoint(x, y));
            if (ui->minimizeButton->geometry().contains(pos) ||
                ui->maximizeButton->geometry().contains(pos) ||
                ui->closeButton->geometry().contains(pos))
            {
                // 如果在按钮上，让基类处理，这样按钮才能收到点击事件
                return QMainWindow::nativeEvent(eventType, message, result);
            }

            // --- 拖动区域 ---
            if (ui->titleBar->geometry().contains(pos)) {
                *result = HTCAPTION;
                return true;
            }

            // --- 边框缩放区域 (可选，但强烈推荐) ---
            if (x >= winrect.left && x < winrect.left + borderWidth) {
                *result = HTLEFT;
            }
            if (x < winrect.right && x >= winrect.right - borderWidth) {
                *result = HTRIGHT;
            }
            if (y >= winrect.top && y < winrect.top + borderWidth) {
                *result = HTTOP;
            }
            if (y < winrect.bottom && y >= winrect.bottom - borderWidth) {
                *result = HTBOTTOM;
            }
            if (x >= winrect.left && x < winrect.left + borderWidth && y >= winrect.top && y < winrect.top + borderWidth) {
                *result = HTTOPLEFT;
            }
            if (x < winrect.right && x >= winrect.right - borderWidth && y >= winrect.top && y < winrect.top + borderWidth) {
                *result = HTTOPRIGHT;
            }
            if (x >= winrect.left && x < winrect.left + borderWidth && y < winrect.bottom && y >= winrect.bottom - borderWidth) {
                *result = HTBOTTOMLEFT;
            }
            if (x < winrect.right && x >= winrect.right - borderWidth && y < winrect.bottom && y >= winrect.bottom - borderWidth) {
                *result = HTBOTTOMRIGHT;
            }

            if (*result != 0) {
                return true;
            }
        }
    }
#endif
    // 对于我们不处理的所有其他消息，调用基类实现
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::onSectionTypeChanged(int index)
{

    ui->parameterStack->setCurrentIndex(index);
    qDebug() << "setCurrentIndex: " << index;
}

void MainWindow::onSetGeometryButtonClicked() {
    ui->geoOrSimStack->setCurrentIndex(0);
}
void MainWindow::onSetParametersButtonClicked() {
    ui->geoOrSimStack->setCurrentIndex(1);
}
void MainWindow::onSetBuiltInModeButtonClicked() {
    ui->geometryModeStack->setCurrentIndex(0);
    ui->setBuiltInModeButtonLine->setChecked(true);
}
void MainWindow::onSetImportDxfModeButtonClicked() {
    ui->geometryModeStack->setCurrentIndex(1);
    ui->setImportDxfModeButtonLine->setChecked(true);
}

