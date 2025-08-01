#include "mainwindow.h"

#include "bridge_simulation_service.h" 

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
#include <QMap> 
#include <QMessageBox>

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
#include <vtkTextActor.h>
#include <vtkLineSource.h>
#include <vtkArcSource.h>
#include <vtkAppendPolyData.h>

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
    m_currentParams.workingDirectory = projectPath.toStdString();




    setupTooglePairs();
    
    setupVtkRenderWindow();



}


// 步骤 3.1: 填充截面数据
void MainWindow::setupModel()
{
    m_sectionDefs.append({
        SectionTypes::Undefined, //id
        "section_undefined", "Select a section",
        {
            
        }
        });
    m_sectionDefs.append({
        SectionTypes::Circle, //id
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
            {"param_chamfered_radius", "Chamfered Radius R(m)", 0.2}
        }
        });

    m_sectionDefs.append({
        SectionTypes::StreamlinedBoxGirder,
        "section_streamlined_box_girder", "Streamlined Box Girder",
        {
            {"param_total_width", "Total Width B(m)", 12},
            {"param_total_height", "Total Height H(m)", 1.5},
            {"param_bottom_width", "Bottom Width B1(m)", 8},
            {"param_slope", "Top Slope i(%)", 2},
            {"param_angle_1", "Angle 1 a1(°)", 30},
            {"param_angle_2", "Angle 2 a2(°)", 20},
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
            connect(lineEdit, &QLineEdit::editingFinished, this, &MainWindow::onGeoParameterEditingFinished);

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
void MainWindow::setupVtkRenderWindow() {
    


    vtkRenderWindow* renderWindow = ui->vtkRenderWidget->renderWindow();

    // --- 3. 创建 Mapper 和 Actor ---

    vtkNew<vtkTextActor> actor;
    
    actor->SetInput("BridgeWind v0.0\nChoose a section to start.");
    vtkTextProperty* textProperty = actor->GetTextProperty();
    textProperty->SetFontSize(24);
    textProperty->SetFontFamilyToCourier();
    textProperty->SetColor(0.7, 0.7, 0.7); // 黄色

    textProperty->SetJustificationToCentered(); // 水平居中对齐
    textProperty->SetVerticalJustificationToTop(); // 垂直顶部对齐

    // --- 3. 设置文本在窗口中的位置 ---
    // (x, y) 像素坐标，原点在左下角
    actor->SetPosition(250, 350);
    // --- 4. 将 Actor 添加到 Renderer ---
    // 使用我们之前在 .h 文件中声明的成员变量 m_renderer
    m_renderer->AddActor(actor);
    m_renderer->SetBackground(0.949, 0.949, 0.969); // 设置背景色

    // --- 5. 将 Renderer 添加到 RenderWindow ---
    // 这是将 VTK 管线连接到 Qt 控件的关键一步
    renderWindow->AddRenderer(m_renderer);

    // --- 6. (可选但推荐) 设置 2D 交互方式 ---
    vtkNew<vtkInteractorStyleImage> style;
    renderWindow->GetInteractor()->SetInteractorStyle(style);

    // --- 7. 重置相机视角并渲染 ---
    m_renderer->ResetCamera();
    renderWindow->Render();
}
void MainWindow::renderVtkWindowWithGeometry(const BridgeWind::Geometry& geometry) {

    vtkNew<vtkAppendPolyData> appendFilter;



    for (const auto& line : geometry.lines) {
        vtkNew<vtkLineSource> lineSource;
        lineSource->SetPoint1(line.begin.x, line.begin.y, 0);
        lineSource->SetPoint2(line.end.x, line.end.y, 0);
        appendFilter->AddInputConnection(lineSource->GetOutputPort());
    }
    auto vtkArcs = geometry.getVtkFormatArcs();
    for (const auto& vArc : vtkArcs) {
        vtkNew<vtkArcSource> arcSource;
        arcSource->SetCenter(vArc.center.x, vArc.center.y, 0);
        arcSource->SetPoint1(vArc.p1.x, vArc.p1.y, 0);
        arcSource->SetPoint2(vArc.p2.x, vArc.p2.y, 0);
        arcSource->SetResolution(vArc.resolution);
        appendFilter->AddInputConnection(arcSource->GetOutputPort());
    }

    vtkRenderWindow* renderWindow = ui->vtkRenderWidget->renderWindow();



    vtkNew<vtkPolyDataMapper> combinedMapper;
    combinedMapper->SetInputConnection(appendFilter->GetOutputPort());

    vtkNew<vtkActor> combinedActor;
    combinedActor->SetMapper(combinedMapper);
    combinedActor->GetProperty()->SetColor(0, 0, 0);
    combinedActor->GetProperty()->SetLineWidth(2);

    // --- 4. 渲染 ---

    m_renderer->SetBackground(0.949, 0.949, 0.969);
    m_renderer->RemoveAllViewProps();
    m_renderer->AddActor(combinedActor); // 只需添加这一个 Actor

    renderWindow->AddRenderer(m_renderer);
    m_renderer->ResetCamera();
    renderWindow->Render();

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

    connect(ui->fileDropWidget, &FileDropWidget::filesDropped, this, &MainWindow::onDxfFileDropped);
    connect(ui->dxfBrowseButton, &QPushButton::clicked, this, &MainWindow::onDxfFileBrowseButtonClicked);
}

void MainWindow::setupTooglePairs() {
    QButtonGroup* toggleGroupGeoOrSim = new QButtonGroup(this);
    toggleGroupGeoOrSim->addButton(ui->setGeometryButton);
    toggleGroupGeoOrSim->addButton(ui->setMeshingButton);
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
    onGeoParameterEditingFinished();
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


void MainWindow::onGeoParameterEditingFinished() {
    try {


        qDebug() << "Parameter editing finished. Updating values...";

        // 1. 获取当前选中的截面索引
        int currentIndex = ui->sectionTypeComboBox->currentIndex();
        if (currentIndex < 0) {
            qDebug() << "No section selected.";
            return; // 如果没有有效的选项，则直接返回
        }

        // 2. 从模型中获取当前截面的定义
        const SectionDef& currentDef = m_sectionDefs[currentIndex];
        qDebug() << "Current Section:" << currentDef.nameComment;

        // 3. 从 QStackedWidget 中获取当前的页面 (它是一个 QScrollArea)
        QScrollArea* scrollArea = qobject_cast<QScrollArea*>(ui->parameterStack->widget(currentIndex));
        if (!scrollArea) {
            qDebug() << "Error: Could not find QScrollArea at index" << currentIndex;
            return;
        }

        // 4. 从 QScrollArea 中获取其内容 QWidget
        QWidget* contentWidget = scrollArea->widget();
        if (!contentWidget) {
            qDebug() << "Error: ScrollArea has no content widget";
            return;
        }

        // 5. 在内容 QWidget 中查找所有的 QLineEdit 子控件
        // findChildren 会返回一个列表，其顺序与添加时的顺序一致
        QList<QLineEdit*> lineEdits = contentWidget->findChildren<QLineEdit*>();

        // 安全检查：确保找到的 LineEdit 数量与模型中的参数数量一致
        if (lineEdits.count() != currentDef.parameters.count()) {
            qDebug() << "Error: Mismatch between found LineEdits and parameter definitions!";
            return;
        }

        // 6. 遍历 QLineEdit 列表，提取数据并与参数键关联
        // 使用 QMap 来存储 "参数键 -> 值" 的映射
        QMap<QString, double> currentParameters;
        for (int i = 0; i < lineEdits.count(); ++i) {
            const ParameterDef& paramDef = currentDef.parameters[i]; // 获取对应的参数定义
            QLineEdit* lineEdit = lineEdits[i];                      // 获取对应的输入框

            QString key = QString::fromUtf8(paramDef.key); // 从模型获取key
            bool ok;
            double value = lineEdit->text().toDouble(&ok); // 从UI获取text并转为double

            if (ok) {
                currentParameters.insert(key, value);
            }
        }

        // 7. (演示) 打印所有获取到的参数和值
        qDebug() << "--- Collected Parameters ---";
        for (auto it = currentParameters.constBegin(); it != currentParameters.constEnd(); ++it) {
            qDebug() << it.key() << ":" << it.value();
        }
        qDebug() << "--------------------------";





        m_geometry.clear();
        if (currentDef.id == SectionTypes::Undefined) {
            return;
        }
        else if (currentDef.id == SectionTypes::Circle) {

            m_geometry.resetAsCircle(currentParameters["param_diameters"]);

        }
        else if (currentDef.id == SectionTypes::Rectangle) {
            m_geometry.resetAsRectangle(
                currentParameters["param_width"],
                currentParameters["param_height"]
            );
        }
        else if (currentDef.id == SectionTypes::ChamferedRectangle) {
            m_geometry.resetAsChamferedRectangle(
                currentParameters["param_width"],
                currentParameters["param_height"],
                currentParameters["param_chamfered_radius"]
            );
        }
        else if (currentDef.id == SectionTypes::StreamlinedBoxGirder) {
            m_geometry.resetAsStreamlinedBoxGirder(
                currentParameters["param_total_width"],
                currentParameters["param_total_height"],
                currentParameters["param_bottom_width"],
                currentParameters["param_slope"],
                currentParameters["param_angle_1"],
                currentParameters["param_angle_2"]
            );
        }
        renderVtkWindowWithGeometry(m_geometry);
    }
    catch (const std::invalid_argument& e) {
        QMessageBox::critical(this, tr("Error"), QString("An invalid parameter error occurred: %1").arg(e.what()));
    }
    catch (const std::exception& e) {
        // 其他标准异常
        QMessageBox::critical(this, tr("Error"), QString("Exception occurred: %1").arg(e.what()));
    }
    catch (...) {
        // 未知异常
        QMessageBox::critical(this, tr("Error"), tr("Unknown Error"));
    }

}


void MainWindow::onDxfFileDropped(const QStringList& filePaths) {
    try {

        if (filePaths.size() == 1) {
            QString filePath = filePaths[0];
            m_geometry.clear();
            m_geometry.loadFromDXF(filePath.toStdString());
            renderVtkWindowWithGeometry(m_geometry);
        }
        else if (filePaths.size() > 1){
            throw std::runtime_error("Please don't drop more than one file.");
        }
        else if (filePaths.size() == 0) {
            throw std::runtime_error("No file was dropped.");
        }

    }
    catch (const std::invalid_argument& e) {
        QMessageBox::critical(this, tr("Error"), QString("An invalid parameter error occurred: %1").arg(e.what()));
    }
    catch (const std::exception& e) {
        // 其他标准异常
        QMessageBox::critical(this, tr("Error"), QString("Exception occurred: %1").arg(e.what()));
    }
    catch (...) {
        // 未知异常
        QMessageBox::critical(this, tr("Error"), tr("Unknown Error"));
    }
}
void MainWindow::onDxfFileBrowseButtonClicked() {



        
    try {

        QString dxfFilePath = QFileDialog::getOpenFileName(
            this,
            tr("Choose a DXF file"),
            m_projectPath,
            tr("DXF Files (*.dxf);;All Files (*)")
        );
        m_geometry.clear();
        m_geometry.loadFromDXF(dxfFilePath.toStdString());
        renderVtkWindowWithGeometry(m_geometry);

    }
    catch (const std::invalid_argument& e) {
        QMessageBox::critical(this, tr("Error"), QString("An invalid parameter error occurred: %1").arg(e.what()));
    }
    catch (const std::exception& e) {
        // 其他标准异常
        QMessageBox::critical(this, tr("Error"), QString("Exception occurred: %1").arg(e.what()));
    }
    catch (...) {
        // 未知异常
        QMessageBox::critical(this, tr("Error"), tr("Unknown Error"));
    }

}
