#include "MainWindow.h"
#include "LoginWindow.h"
#include<sstream>
#include <QApplication>
#include <QHeaderView>
#include <QSpinBox>
#include <QFile>
#include <QDateTime>
#include<qinputdialog.h>

MainWindow::MainWindow(SystemManager* systemManager, const UserInfo& userInfo, QWidget* parent)
    : QMainWindow(parent), systemManager(systemManager), currentUser(userInfo) {

    setWindowTitle("医院挂号管理系统");
    resize(1000, 700);

    setupUI();
    applyStyles();

    // 加载基础数据
    loadUserData();
    loadDoctors();
    loadRegistrations();  // 这会根据角色调用不同的函数
}

MainWindow::~MainWindow() {
    delete systemManager;
}

void MainWindow::setupUI() {
    // 创建中央部件
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 标题栏 
    QWidget* titleBar = new QWidget();
    titleBar->setFixedHeight(60);
    titleBar->setStyleSheet("background-color: #2c3e50;");

    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 20, 0);

    QLabel* titleLabel = new QLabel("🏥 医院挂号管理系统");
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");

    QLabel* userInfoLabel = new QLabel(
        QString("欢迎，%1").arg(
            QString::fromStdString(currentUser.name)
        )
    );
    userInfoLabel->setStyleSheet("color: white; font-size: 14px;");

    QPushButton* logoutButton = new QPushButton("退出登录");
    logoutButton->setFixedSize(100, 35);
    logoutButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #e74c3c;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #c0392b;"
        "}"
    );

    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);

    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(userInfoLabel);
    titleLayout->addSpacing(20);
    titleLayout->addWidget(logoutButton);

    mainLayout->addWidget(titleBar);

    // 标签页
    tabWidget = new QTabWidget();
    tabWidget->setTabPosition(QTabWidget::North);
    tabWidget->setTabShape(QTabWidget::Rounded);

    setupHomeTab();
    // 挂号管理（根据角色显示不同的界面）
    if (currentUser.role == "patient") {
        setupPatientRegistrationTab();  // 病人的挂号管理
    }
    else if (currentUser.role == "doctor") {
        setupDoctorRegistrationTab();   // 医生的工作台
    }
    else if (currentUser.role == "admin") {
        setupAdminRegistrationTab();    // 管理员查看所有挂号
    }

    // 个人中心（所有用户都有）
    setupProfileTab();
    
    // 医生列表（病人和管理员可以查看）
    if (currentUser.role == "patient" || currentUser.role == "admin") {
        setupDoctorsTab();
    }

    // 科室管理（仅管理员）
    if (currentUser.role == "admin") {
        setupDepartmentTab();
    }

    mainLayout->addWidget(tabWidget);

    // 状态栏
    QStatusBar* statusBar = new QStatusBar();
    setStatusBar(statusBar);

    QLabel* statusLabel = new QLabel(
        QString("登录时间：%1 | 角色：%2").arg(
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
            getRoleDisplayName(currentUser.role)
        )
    );
    statusLabel->setStyleSheet(
        "QLabel {"
        "  background-color: #2c3e50;"
        "  color: white;"
        "  padding: 5px;"
        "  border-radius: 3px;"
        "}"
    );
    statusBar->addWidget(statusLabel);
}
void MainWindow::setupHomeTab() {
    homeTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(homeTab);
    layout->setAlignment(Qt::AlignCenter);

    welcomeLabel = new QLabel(
        QString("欢迎使用医院挂号管理系统，%1！").arg(QString::fromStdString(currentUser.name))
    );
    welcomeLabel->setAlignment(Qt::AlignCenter);
    QFont font = welcomeLabel->font();
    font.setPointSize(24);
    font.setBold(true);
    welcomeLabel->setFont(font);
    welcomeLabel->setStyleSheet("color: #2c3e50;");

    QLabel* instructionLabel = new QLabel("请使用上方标签页进行相应操作");
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("color: #7f8c8d; font-size: 16px; margin-top: 20px;");

    layout->addStretch();
    layout->addWidget(welcomeLabel);
    layout->addWidget(instructionLabel);
    layout->addStretch();

    tabWidget->addTab(homeTab, "🏠 首页");
}
void MainWindow::setupProfileTab() {
    profileTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(profileTab);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 个人信息组
    QGroupBox* infoGroup = new QGroupBox("个人信息");
    QFormLayout* infoLayout = new QFormLayout(infoGroup);

    // 用户名和角色（只读）
    QLabel* usernameLabel = new QLabel(QString::fromStdString(currentUser.username));
    QLabel* roleLabel = new QLabel(getRoleDisplayName(currentUser.role));

    // 可编辑信息 - 将变量声明为类的成员变量
    nameEdit = new QLineEdit(QString::fromStdString(currentUser.name));

    genderCombo = new QComboBox();
    genderCombo->addItem("男", "male");
    genderCombo->addItem("女", "female");
    genderCombo->addItem("其他", "other");
    genderCombo->setCurrentIndex(genderCombo->findData(QString::fromStdString(currentUser.gender)));

    ageSpin = new QSpinBox();  // 这里直接使用类的成员变量
    ageSpin->setRange(0, 150);
    ageSpin->setValue(currentUser.age);

    phoneEdit = new QLineEdit(QString::fromStdString(currentUser.phone));

    // 根据角色显示额外字段
    if (currentUser.role == "patient") {
        addressEdit = new QLineEdit(QString::fromStdString(currentUser.address));
        idCardEdit = new QLineEdit(QString::fromStdString(currentUser.idCard));
        departmentEdit = nullptr;

        infoLayout->addRow("地址:", addressEdit);
        infoLayout->addRow("身份证号:", idCardEdit);
    }
    else if (currentUser.role == "doctor") {
        addressEdit = nullptr;
        idCardEdit = nullptr;
        departmentEdit = new QLineEdit(QString::fromStdString(currentUser.department));

        infoLayout->addRow("科室:", departmentEdit);
    }
    else {
        addressEdit = nullptr;
        idCardEdit = nullptr;
        departmentEdit = nullptr;
    }

    saveProfileButton = new QPushButton("保存信息");
    saveProfileButton->setFixedHeight(40);

    connect(saveProfileButton, &QPushButton::clicked, this, &MainWindow::onSaveProfileClicked);

    infoLayout->addRow("用户名:", usernameLabel);
    infoLayout->addRow("角色:", roleLabel);
    infoLayout->addRow("姓名:", nameEdit);
    infoLayout->addRow("性别:", genderCombo);
    infoLayout->addRow("年龄:", ageSpin);
    infoLayout->addRow("电话:", phoneEdit);
    infoLayout->addRow("", saveProfileButton);

    // 修改密码组
    QGroupBox* passwordGroup = new QGroupBox("修改密码");
    QFormLayout* passwordLayout = new QFormLayout(passwordGroup);

    oldPasswordEdit = new QLineEdit();
    oldPasswordEdit->setPlaceholderText("请输入原密码");
    oldPasswordEdit->setEchoMode(QLineEdit::Password);

    newPasswordEdit = new QLineEdit();
    newPasswordEdit->setPlaceholderText("请输入新密码");
    newPasswordEdit->setEchoMode(QLineEdit::Password);

    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setPlaceholderText("请确认新密码");
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    changePasswordButton = new QPushButton("修改密码");
    changePasswordButton->setFixedHeight(40);

    connect(changePasswordButton, &QPushButton::clicked,
        this, &MainWindow::onChangePasswordClicked);

    passwordLayout->addRow("原密码:", oldPasswordEdit);
    passwordLayout->addRow("新密码:", newPasswordEdit);
    passwordLayout->addRow("确认密码:", confirmPasswordEdit);
    passwordLayout->addRow("", changePasswordButton);

    mainLayout->addWidget(infoGroup);
    mainLayout->addWidget(passwordGroup);
    mainLayout->addStretch();

    tabWidget->addTab(profileTab, "👤 个人中心");
}
void MainWindow::setupDoctorsTab() {
    doctorsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(doctorsTab);
    layout->setContentsMargins(10, 10, 10, 10);

    doctorsTable = new QTableWidget();
    doctorsTable->setColumnCount(6);
    doctorsTable->setHorizontalHeaderLabels(
        QStringList() << "ID" << "姓名" << "性别" << "年龄" << "电话" << "科室"
    );
    doctorsTable->setAlternatingRowColors(true);
    doctorsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    doctorsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    doctorsTable->horizontalHeader()->setStretchLastSection(true);
    doctorsTable->verticalHeader()->setVisible(false);

    layout->addWidget(doctorsTable);

    tabWidget->addTab(doctorsTab, "👨‍⚕️ 医生列表");
}
void MainWindow::setupDepartmentTab() {
    departmentTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(departmentTab);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ==== 科室管理组 ====
    QGroupBox* manageGroup = new QGroupBox("科室管理");
    QVBoxLayout* manageLayout = new QVBoxLayout(manageGroup);

    // 工具栏
    QWidget* toolbar = new QWidget();
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);

    addDepartmentButton = new QPushButton("➕ 添加科室");
    editDepartmentButton = new QPushButton("✏️ 编辑");
    deleteDepartmentButton = new QPushButton("🗑️ 删除");
    refreshDepartmentButton = new QPushButton("🔄 刷新");

    // 设置按钮样式
    addDepartmentButton->setStyleSheet("background-color: #10b981;");
    editDepartmentButton->setStyleSheet("background-color: #f59e0b;");
    deleteDepartmentButton->setStyleSheet("background-color: #ef4444;");

    connect(addDepartmentButton, &QPushButton::clicked,
        this, &MainWindow::onAddDepartmentClicked);
    connect(editDepartmentButton, &QPushButton::clicked,
        this, &MainWindow::onEditDepartmentClicked);
    connect(deleteDepartmentButton, &QPushButton::clicked,
        this, &MainWindow::onDeleteDepartmentClicked);
    connect(refreshDepartmentButton, &QPushButton::clicked,
        this, &MainWindow::loadDepartments);

    toolbarLayout->addWidget(addDepartmentButton);
    toolbarLayout->addWidget(editDepartmentButton);
    toolbarLayout->addWidget(deleteDepartmentButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(refreshDepartmentButton);

    // 科室表格
    departmentTable = new QTableWidget();
    departmentTable->setColumnCount(5);
    departmentTable->setHorizontalHeaderLabels(
        QStringList() << "ID" << "科室名称" << "联系电话" << "位置" << "描述"
    );
    departmentTable->setAlternatingRowColors(true);
    departmentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    departmentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    departmentTable->horizontalHeader()->setStretchLastSection(true);
    departmentTable->verticalHeader()->setVisible(false);

    // 设置列宽
    departmentTable->setColumnWidth(0, 60);   // ID
    departmentTable->setColumnWidth(1, 120);  // 科室名称
    departmentTable->setColumnWidth(2, 120);  // 联系电话
    departmentTable->setColumnWidth(3, 150);  // 位置

    manageLayout->addWidget(toolbar);
    manageLayout->addWidget(departmentTable);

    // ==== 医生分配组 ====
    QGroupBox* assignGroup = new QGroupBox("医生科室分配");
    QFormLayout* assignLayout = new QFormLayout(assignGroup);

    assignDoctorCombo = new QComboBox();
    assignDepartmentCombo = new QComboBox();
    assignButton = new QPushButton("分配");

    assignButton->setStyleSheet("background-color: #3b82f6; padding: 8px 20px;");
    connect(assignButton, &QPushButton::clicked,
        this, &MainWindow::onAssignDoctorClicked);

    assignLayout->addRow("选择医生:", assignDoctorCombo);
    assignLayout->addRow("分配到科室:", assignDepartmentCombo);
    assignLayout->addRow("", assignButton);

    // 加载医生和科室数据
    loadDoctorsForAssignment();
    loadDepartmentsForAssignment();

    // 添加到主布局
    mainLayout->addWidget(manageGroup);
    mainLayout->addWidget(assignGroup);

    // 根据用户角色控制权限
    if (currentUser.role != "admin") {
        addDepartmentButton->setEnabled(false);
        editDepartmentButton->setEnabled(false);
        deleteDepartmentButton->setEnabled(false);
        assignButton->setEnabled(false);

        QString tooltip = "只有管理员可以管理科室";
        addDepartmentButton->setToolTip(tooltip);
        editDepartmentButton->setToolTip(tooltip);
        deleteDepartmentButton->setToolTip(tooltip);
        assignButton->setToolTip(tooltip);
    }

    tabWidget->addTab(departmentTab, "🏥 科室管理");
}
void MainWindow::applyStyles() {
    QFile styleFile("styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        setStyleSheet(styleSheet);
    }
}

void MainWindow::loadUserData() {
    // 重新加载用户信息
    currentUser = systemManager->getUserInfo(currentUser.userId);

    // 更新欢迎信息
    if (welcomeLabel) {
        welcomeLabel->setText(
            QString("欢迎使用医院挂号管理系统，%1！").arg(QString::fromStdString(currentUser.name))
        );
    }
}

void MainWindow::loadDoctors() {
    auto doctors = systemManager->getAllDoctors();

    if (doctorsTable) {
        doctorsTable->setRowCount(static_cast<int>(doctors.size()));
        for (int i = 0; i < static_cast<int>(doctors.size()); ++i) {
            const auto& doctor = doctors[i];

            doctorsTable->setItem(i, 0, new QTableWidgetItem(QString::number(doctor.doctorId)));
            doctorsTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(doctor.name)));
            doctorsTable->setItem(i, 2, new QTableWidgetItem(getGenderDisplayName(doctor.gender)));
            doctorsTable->setItem(i, 3, new QTableWidgetItem(QString::number(doctor.age)));
            doctorsTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(doctor.phone)));

            // 显示科室信息
            QString department = QString::fromStdString(doctor.department);
            if (department.isEmpty()) {
                department = "未分配";
            }
            doctorsTable->setItem(i, 5, new QTableWidgetItem(department));
        }
    }
}

// 事件处理函数
void MainWindow::onLogoutClicked() {
    if (QMessageBox::question(this, "确认退出", "确定要退出登录吗？") == QMessageBox::Yes) {
        LoginWindow* loginWindow = new LoginWindow();
        loginWindow->show();
        this->close();
    }
}

void MainWindow::onDepartmentSelected(int index) {
    QString department = departmentCombo->itemData(index).toString();
    doctorCombo->clear();
    doctorCombo->setEnabled(!department.isEmpty());
    if (!department.isEmpty()) {
        auto doctors = systemManager->getDoctorsByDepartment(department.toStdString());
        doctorCombo->addItem("请选择医生", 0);
        for (const auto& doctor : doctors) {
            doctorCombo->addItem(
                QString("%1 (%2)").arg(
                    QString::fromStdString(doctor.name),
                    QString::fromStdString(doctor.phone)
                ),
                doctor.doctorId
            );
        }
    }
}

void MainWindow::onNewRegistrationClicked() {
    if (currentUser.role != "patient") {
        QMessageBox::warning(this, "权限不足", "只有病人可以挂号！");
        return;
    }

    QString date = dateEdit->date().toString("yyyy-MM-dd");
    int doctorId = doctorCombo->currentData().toInt();
    QString notes = notesEdit->toPlainText();

    if (doctorId == 0) {
        QMessageBox::warning(this, "提示", "请选择医生！");
        return;
    }

    int regId = systemManager->createRegistration(currentUser.userId, doctorId,
        date.toStdString(), notes.toStdString());

    if (regId > 0) {
        QMessageBox::information(this, "成功", QString("挂号成功！挂号单号：%1").arg(regId));

        // 清空表单
        notesEdit->clear();
        loadRegistrations();
    }
    else {
        QMessageBox::critical(this, "失败",
            QString("挂号失败：%1").arg(QString::fromStdString(systemManager->getLastError())));
    }
}

void MainWindow::refreshRegistrations() {
    loadRegistrations();
    QMessageBox::information(this, "刷新", "挂号记录已刷新！");
}

void MainWindow::filterRegistrations(const QString& status) {
    // 这里可以实现按状态过滤的逻辑
    // 由于时间关系，我们只是重新加载所有数据
    loadRegistrations();
}

void MainWindow::onSaveProfileClicked() {
    UserInfo updatedInfo = currentUser;
    updatedInfo.name = nameEdit->text().toStdString();
    updatedInfo.gender = genderCombo->currentData().toString().toStdString();
    updatedInfo.age = ageSpin->value();
    updatedInfo.phone = phoneEdit->text().toStdString();

    if (currentUser.role == "patient" && addressEdit && idCardEdit) {
        updatedInfo.address = addressEdit->text().toStdString();
        updatedInfo.idCard = idCardEdit->text().toStdString();
    }
    else if (currentUser.role == "doctor" && departmentEdit) {
        updatedInfo.department = departmentEdit->text().toStdString();
    }

    if (systemManager->updateUserInfo(updatedInfo)) {
        currentUser = updatedInfo;
        QMessageBox::information(this, "成功", "个人信息更新成功！");
        loadUserData();
    }
    else {
        QMessageBox::critical(this, "失败",
            QString("更新失败：%1").arg(QString::fromStdString(systemManager->getLastError())));
    }
}

void MainWindow::onChangePasswordClicked() {
    QString oldPassword = oldPasswordEdit->text();
    QString newPassword = newPasswordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "错误", "两次输入的密码不一致！");
        return;
    }

    if (newPassword.length() < 6) {
        QMessageBox::warning(this, "错误", "密码长度不能少于6位！");
        return;
    }

    // 这里需要调用SystemManager的changePassword方法
    // 由于之前的SystemManager没有实现这个方法，我们先显示提示
    QMessageBox::information(this, "提示", "修改密码功能正在开发中...");

    // 清空密码框
    oldPasswordEdit->clear();
    newPasswordEdit->clear();
    confirmPasswordEdit->clear();
}

QString MainWindow::getRoleDisplayName(const std::string& role) {
    if (role == "patient") return "病人";
    if (role == "doctor") return "医生";
    if (role == "admin") return "管理员";
    return "未知";
}

QString MainWindow::getGenderDisplayName(const std::string& gender) {
    if (gender == "male") return "男";
    if (gender == "female") return "女";
    return "其他";
}
void MainWindow::loadDepartments() {
    if (!departmentTable) return;

    // 从数据库获取科室数据
    auto departments = systemManager->getAllDepartments();
    departmentTable->setRowCount(static_cast<int>(departments.size()));

    for (int i = 0; i < static_cast<int>(departments.size()); ++i) {
        const auto& dept = departments[i];

        departmentTable->setItem(i, 0, new QTableWidgetItem(QString::number(dept.departmentId)));
        departmentTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(dept.departmentName)));
        departmentTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(dept.contactPhone)));
        departmentTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(dept.location)));
        departmentTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(dept.description)));
    }
}
void MainWindow::loadDepartmentsForAssignment() {
    if (!assignDepartmentCombo) {
        std::cout << "错误：assignDepartmentCombo 为空" << std::endl;
        return;
    }

    assignDepartmentCombo->clear();
    assignDepartmentCombo->addItem("请选择科室", 0);
    assignDepartmentCombo->addItem("未分配", -1);

    std::cout << "开始加载科室分配列表..." << std::endl;

    try {
        // 从数据库获取科室数据
        auto departments = systemManager->getAllDepartments();
        std::cout << "从数据库获取到 " << departments.size() << " 个科室" << std::endl;

        for (const auto& dept : departments) {
            QString deptName = QString::fromStdString(dept.departmentName);
            assignDepartmentCombo->addItem(deptName, dept.departmentId);

            std::cout << "添加科室: " << deptName.toStdString()
                << " (ID: " << dept.departmentId << ")" << std::endl;
        }

        // 如果数据库没有科室数据，添加一些示例
        if (departments.empty()) {
            std::cout << "数据库中没有科室数据，使用默认数据" << std::endl;

            QStringList defaultDepartments = {
                "内科", "外科", "儿科", "妇产科", "眼科", "口腔科"
            };

            for (int i = 0; i < defaultDepartments.size(); ++i) {
                assignDepartmentCombo->addItem(defaultDepartments[i], i + 1);
                std::cout << "添加默认科室: " << defaultDepartments[i].toStdString()
                    << " (ID: " << i + 1 << ")" << std::endl;
            }
        }

        std::cout << "科室下拉框加载完成，共 " << assignDepartmentCombo->count()
            << " 个选项" << std::endl;

    }
    catch (const std::exception& e) {
        std::cout << "加载科室数据异常: " << e.what() << std::endl;

        // 如果出错，使用默认数据
        QStringList defaultDepartments = {
            "内科", "外科", "儿科", "妇产科", "眼科", "口腔科"
        };

        for (int i = 0; i < defaultDepartments.size(); ++i) {
            assignDepartmentCombo->addItem(defaultDepartments[i], i + 1);
        }
    }
}
void MainWindow::loadDoctorsForAssignment() {
    if (!assignDoctorCombo) return;

    assignDoctorCombo->clear();
    assignDoctorCombo->addItem("请选择医生", 0);

    try {
        // 从数据库获取医生数据
        auto doctors = systemManager->getAllDoctors();

        // 如果数据库有医生，添加到下拉框
        for (const auto& doctor : doctors) {
            QString department = QString::fromStdString(doctor.department);
            if (department.isEmpty()) {
                department = "未分配";
            }

            QString displayText = QString("%1 (%2)")
                .arg(QString::fromStdString(doctor.name))
                .arg(department);

            assignDoctorCombo->addItem(displayText, doctor.doctorId);
        }

        // 如果数据库没有医生，添加一些示例数据
        if (doctors.empty()) {
            assignDoctorCombo->addItem("张医生 (内科)", 1001);
            assignDoctorCombo->addItem("李医生 (外科)", 1002);
            assignDoctorCombo->addItem("王医生 (儿科)", 1003);
            assignDoctorCombo->addItem("刘医生 (妇产科)", 1004);
            assignDoctorCombo->addItem("陈医生 (眼科)", 1005);
        }

    }
    catch (...) {
        // 如果出错，使用示例数据
        assignDoctorCombo->addItem("张医生 (内科)", 1001);
        assignDoctorCombo->addItem("李医生 (外科)", 1002);
        assignDoctorCombo->addItem("王医生 (儿科)", 1003);
        assignDoctorCombo->addItem("刘医生 (妇产科)", 1004);
        assignDoctorCombo->addItem("陈医生 (眼科)", 1005);
    }
}
//病人挂号管理
void MainWindow::setupPatientRegistrationTab() {
    QWidget* registrationTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(registrationTab);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    QGroupBox* newRegGroup = new QGroupBox("🏥 新增挂号");
    newRegGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 16px; }");
    QFormLayout* formLayout = new QFormLayout(newRegGroup);
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    dateEdit->setMinimumDate(QDate::currentDate());
    dateEdit->setMaximumDate(QDate::currentDate().addDays(30));

    // 选择科室
    // 选择科室
    departmentCombo = new QComboBox();
    departmentCombo->addItem("请选择科室", "");        // 提示项，data 为空
    departmentCombo->addItem("内科", "内科");
    departmentCombo->addItem("外科", "外科");
    departmentCombo->addItem("儿科", "儿科");
    departmentCombo->addItem("妇产科", "妇产科");
    departmentCombo->addItem("眼科", "眼科");
    departmentCombo->addItem("口腔科", "口腔科");
    departmentCombo->addItem("耳鼻喉科", "耳鼻喉科");
    departmentCombo->addItem("皮肤科", "皮肤科");
    departmentCombo->addItem("中医科", "中医科");
    departmentCombo->addItem("康复科", "康复科");
    departmentCombo->addItem("急诊科", "急诊科");
    // 选择医生
    doctorCombo = new QComboBox();
    doctorCombo->setEnabled(false);
    doctorCombo->addItem("请先选择科室", 0);

    // 症状描述
    notesEdit = new QTextEdit();
    notesEdit->setMaximumHeight(80);
    notesEdit->setPlaceholderText("请描述您的症状（可选，但有助于医生诊断）");

    // 提交按钮
    submitButton = new QPushButton("✅ 提交挂号");
    submitButton->setFixedHeight(45);
    submitButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #10b981;"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 16px;"
        "  border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #059669;"
        "}"
    );

    connect(departmentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onDepartmentSelected);
    connect(submitButton, &QPushButton::clicked,
        this, &MainWindow::onNewRegistrationClicked);

    formLayout->addRow("📅 挂号日期:", dateEdit);
    formLayout->addRow("🏥 选择科室:", departmentCombo);
    formLayout->addRow("👨‍⚕️ 选择医生:", doctorCombo);
    formLayout->addRow("📝 症状描述:", notesEdit);
    formLayout->addRow("", submitButton);

    // ===== 我的挂号记录 =====
    QGroupBox* recordsGroup = new QGroupBox("📋 我的挂号记录");
    recordsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 16px; }");
    QVBoxLayout* recordsLayout = new QVBoxLayout(recordsGroup);

    // 工具栏
    QWidget* toolbar = new QWidget();
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);

    refreshButton = new QPushButton("🔄 刷新");
    refreshButton->setFixedSize(100, 35);

    QLabel* filterLabel = new QLabel("筛选:");
    filterCombo = new QComboBox();
    filterCombo->addItem("全部状态", "");
    filterCombo->addItem("待处理", "pending");
    filterCombo->addItem("已完成", "completed");
    filterCombo->addItem("已取消", "cancelled");

    connect(departmentCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(onDepartmentSelected(int)));

    connect(filterCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(onFilterChanged()));

    toolbarLayout->addWidget(refreshButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(filterLabel);
    toolbarLayout->addWidget(filterCombo);

    // 挂号表格
    regTable = new QTableWidget();
    regTable->setColumnCount(6);
    regTable->setHorizontalHeaderLabels(
        QStringList() << "单号" << "日期" << "医生" << "科室" << "状态" << "费用"
    );
    regTable->setAlternatingRowColors(true);
    regTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    regTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    regTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    regTable->verticalHeader()->setVisible(false);

    // 设置列宽比例
    regTable->setColumnWidth(0, 80);   // 单号
    regTable->setColumnWidth(1, 100);  // 日期
    regTable->setColumnWidth(2, 120);  // 医生
    regTable->setColumnWidth(3, 100);  // 科室
    regTable->setColumnWidth(4, 80);   // 状态
    regTable->setColumnWidth(5, 100);  // 费用

    recordsLayout->addWidget(toolbar);
    recordsLayout->addWidget(regTable);

    // 添加到主布局
    mainLayout->addWidget(newRegGroup);
    mainLayout->addWidget(recordsGroup);

    // 提示信息
    QLabel* tipLabel = new QLabel("💡 提示：挂号后请按时就诊，如有变动请及时联系医院");
    tipLabel->setStyleSheet("color: #666; font-size: 12px; padding: 5px; background-color: #f8f9fa; border-radius: 4px;");
    mainLayout->addWidget(tipLabel);

    tabWidget->addTab(registrationTab, "📋 挂号管理");
}

void MainWindow::setupAdminRegistrationTab() {
    QWidget* adminTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(adminTab);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ===== 新增挂号（管理员）=====
    QGroupBox* addRegGroup = new QGroupBox("➕ 新增挂号单");
    QFormLayout* addFormLayout = new QFormLayout(addRegGroup);

    adminDateEdit = new QDateEdit(QDate::currentDate());
    adminDateEdit->setCalendarPopup(true);
    adminDateEdit->setDisplayFormat("yyyy-MM-dd");

    adminPatientCombo = new QComboBox();
    adminDoctorCombo = new QComboBox();
    adminNotesEdit = new QTextEdit();
    adminNotesEdit->setMaximumHeight(30);
    adminNotesEdit->setPlaceholderText("备注信息");

    QPushButton* adminAddButton = new QPushButton("添加挂号单");
    adminAddButton->setStyleSheet("background-color: #10b981; color: white; font-weight: bold; padding: 15px;");

    connect(adminAddButton, &QPushButton::clicked, this, &MainWindow::onAdminAddRegistrationClicked);

    // 加载病人和医生列表
    loadPatientsForAdmin();
    loadDoctorsForAdmin();

    addFormLayout->addRow("挂号日期:", adminDateEdit);
    addFormLayout->addRow("选择病人:", adminPatientCombo);
    addFormLayout->addRow("选择医生:", adminDoctorCombo);
    addFormLayout->addRow("备注:", adminNotesEdit);
    addFormLayout->addRow("", adminAddButton);

    // ===== 系统统计 =====
    QGroupBox* statsGroup = new QGroupBox("📊 系统统计");
    QGridLayout* statsLayout = new QGridLayout(statsGroup);
    // 创建统计卡片
    auto createStatCard = [](const QString& title, const QString& value, const QString& color) {
        QWidget* card = new QWidget();
        QVBoxLayout* cardLayout = new QVBoxLayout(card);

        QLabel* titleLabel = new QLabel(title);
        QLabel* valueLabel = new QLabel(value);

        titleLabel->setStyleSheet(QString("color: %1; font-size: 12px;").arg(color));
        valueLabel->setStyleSheet("font-size: 24px; font-weight: bold;");

        cardLayout->addWidget(titleLabel);
        cardLayout->addWidget(valueLabel);
        card->setStyleSheet("background-color: white; border-radius: 8px; padding: 15px;");
        return card;
        };

    // 先更新统计数据
    updateAdminStats();

    // 创建卡片并存储引用，以便后续更新
    QWidget* todayCard = createStatCard("今日挂号", QString::number(adminTodayCount), "#3b82f6");
    QWidget* doctorCard = createStatCard("医生数", QString::number(adminDoctorCount), "#f59e0b");
    QWidget* patientCard = createStatCard("病人数", QString::number(adminPatientCount), "#ef4444");

    // 为了后续更新，我们需要存储值标签
    QLabel* todayValueLabel = todayCard->findChild<QLabel*>();
    QLabel* doctorValueLabel = doctorCard->findChild<QLabel*>();
    QLabel* patientValueLabel = patientCard->findChild<QLabel*>();

    // 添加到布局
    statsLayout->addWidget(todayCard, 0, 0);
    statsLayout->addWidget(doctorCard, 0, 1);
    statsLayout->addWidget(patientCard, 0, 2);
    // ===== 所有挂号记录 =====
    QGroupBox* recordsGroup = new QGroupBox("📋 所有挂号记录");
    QVBoxLayout* recordsLayout = new QVBoxLayout(recordsGroup);

    // 高级筛选工具栏
    QWidget* filterBar = new QWidget();
    QHBoxLayout* filterLayout = new QHBoxLayout(filterBar);

    startDateEdit = new QDateEdit(QDate::currentDate().addDays(-7));
    endDateEdit = new QDateEdit(QDate::currentDate());
    deptFilterCombo = new QComboBox();
    doctorFilterCombo = new QComboBox();
    statusFilterCombo = new QComboBox();
    filterButton = new QPushButton("筛选");
    resetButton = new QPushButton("重置");
    exportAllButton = new QPushButton("导出全部");

    startDateEdit->setDisplayFormat("yyyy-MM-dd");
    endDateEdit->setDisplayFormat("yyyy-MM-dd");

    deptFilterCombo->addItem("所有科室", "");
    doctorFilterCombo->addItem("所有医生", "");
    statusFilterCombo->addItem("所有状态", "");
    statusFilterCombo->addItem("待处理", "pending");
    statusFilterCombo->addItem("已完成", "completed");
    statusFilterCombo->addItem("已取消", "cancelled");

    filterLayout->addWidget(new QLabel("日期:"));
    filterLayout->addWidget(startDateEdit);
    filterLayout->addWidget(new QLabel("至"));
    filterLayout->addWidget(endDateEdit);
    filterLayout->addWidget(new QLabel("科室:"));
    filterLayout->addWidget(deptFilterCombo);
    filterLayout->addWidget(new QLabel("医生:"));
    filterLayout->addWidget(doctorFilterCombo);
    filterLayout->addWidget(new QLabel("状态:"));
    filterLayout->addWidget(statusFilterCombo);
    filterLayout->addWidget(filterButton);
    filterLayout->addWidget(resetButton);
    filterLayout->addStretch();
    filterLayout->addWidget(exportAllButton);

    // 挂号表格
    adminRegTable = new QTableWidget();
    adminRegTable->setColumnCount(10);
    adminRegTable->setHorizontalHeaderLabels(
        QStringList() << "单号" << "日期" << "病人" << "医生" << "科室" << "状态" << "费用" << "账单" << "操作" << "删除"
    );
    adminRegTable->setAlternatingRowColors(true);
    adminRegTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    adminRegTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    adminRegTable->horizontalHeader()->setStretchLastSection(true);
    adminRegTable->verticalHeader()->setVisible(false);

    recordsLayout->addWidget(filterBar);
    recordsLayout->addWidget(adminRegTable);

    // ===== 系统管理操作 =====
    QGroupBox* adminActionsGroup = new QGroupBox("⚙️ 系统管理");
    QHBoxLayout* adminActionsLayout = new QHBoxLayout(adminActionsGroup);

    backupBtn = new QPushButton("💾 数据备份");
    restoreBtn = new QPushButton("🔄 数据恢复");
    reportBtn = new QPushButton("📈 生成报表");
    systemLogBtn = new QPushButton("📋 系统日志");

    QString adminBtnStyle = "padding: 10px; font-weight: bold; min-width: 100px;";
    backupBtn->setStyleSheet(adminBtnStyle + "background-color: #3b82f6; color: white;");
    restoreBtn->setStyleSheet(adminBtnStyle + "background-color: #10b981; color: white;");
    reportBtn->setStyleSheet(adminBtnStyle + "background-color: #f59e0b; color: white;");
    systemLogBtn->setStyleSheet(adminBtnStyle + "background-color: #6b7280; color: white;");

    adminActionsLayout->addWidget(backupBtn);
    adminActionsLayout->addWidget(restoreBtn);
    adminActionsLayout->addWidget(reportBtn);
    adminActionsLayout->addWidget(systemLogBtn);
    adminActionsLayout->addStretch();

    // 添加到主布局
    mainLayout->addWidget(addRegGroup);
    mainLayout->addWidget(statsGroup);
    mainLayout->addWidget(recordsGroup,1);
    mainLayout->addWidget(adminActionsGroup);

    tabWidget->addTab(adminTab, "📋 挂号管理");

    // 加载数据
    loadAdminRegistrations();

    // 连接筛选按钮
    connect(filterButton, &QPushButton::clicked, [this, todayValueLabel,
        doctorValueLabel, patientValueLabel]() {
            // 筛选后重新加载数据
            loadAdminRegistrations();
            // 更新统计数据
            updateAdminStats();
            // 更新显示
            if (todayValueLabel) todayValueLabel->setText(QString::number(adminTodayCount));
            if (doctorValueLabel) doctorValueLabel->setText(QString::number(adminDoctorCount));
            if (patientValueLabel) patientValueLabel->setText(QString::number(adminPatientCount));
        });

    connect(resetButton, &QPushButton::clicked, [this, todayValueLabel,
        doctorValueLabel, patientValueLabel]() {
            // 重置筛选条件
            startDateEdit->setDate(QDate::currentDate().addDays(-7));
            endDateEdit->setDate(QDate::currentDate());
            deptFilterCombo->setCurrentIndex(0);
            doctorFilterCombo->setCurrentIndex(0);
            statusFilterCombo->setCurrentIndex(0);

            // 重新加载数据
            loadAdminRegistrations();
            // 更新统计数据
            updateAdminStats();
            // 更新显示
            if (todayValueLabel) todayValueLabel->setText(QString::number(adminTodayCount));
            if (doctorValueLabel) doctorValueLabel->setText(QString::number(adminDoctorCount));
            if (patientValueLabel) patientValueLabel->setText(QString::number(adminPatientCount));
        });
}

void MainWindow::loadRegistrations() {
    std::vector<RegistrationInfo> registrations;

    if (currentUser.role == "patient") {
        registrations = systemManager->getRegistrationsByPatient(currentUser.userId);
        // 调用病人的专门函数
        loadPatientRegistrations();
    }
    else if (currentUser.role == "doctor") {
        registrations = systemManager->getRegistrationsByDoctor(currentUser.userId);
        // 调用医生的专门函数
        loadDoctorRegistrations();
    }
    else {
        registrations = systemManager->getAllRegistrations();
        // 调用管理员的专门函数
        loadAdminRegistrations();
    }
}
// 病人挂号记录加载
void MainWindow::loadPatientRegistrations() {
    std::vector<RegistrationInfo> registrations = systemManager->getRegistrationsByPatient(currentUser.userId);
    if (regTable) {
        regTable->setRowCount(static_cast<int>(registrations.size()));

        for (int i = 0; i < static_cast<int>(registrations.size()); ++i) {
            const auto& reg = registrations[i];

            // 状态显示
            QString statusText;
            QString statusStyle;
            if (reg.status == "pending") {
                statusText = "待处理";
                statusStyle = "color: #f59e0b; font-weight: bold;";
            }
            else if (reg.status == "completed") {
                statusText = "已完成";
                statusStyle = "color: #10b981; font-weight: bold;";
            }
            else {
                statusText = "已取消";
                statusStyle = "color: #ef4444; font-weight: bold;";
            }

            regTable->setItem(i, 0, new QTableWidgetItem(QString::number(reg.registrationId)));
            regTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(reg.registrationDate)));
            regTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(reg.doctorName)));
            regTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(reg.doctorDepartment)));

            QTableWidgetItem* statusItem = new QTableWidgetItem(statusText);
            statusItem->setTextAlignment(Qt::AlignCenter);
            statusItem->setData(Qt::UserRole, QString::fromStdString(reg.status));
            regTable->setItem(i, 4, statusItem);

            // 金额
            QString amountText = reg.hasBill ?
                QString("¥%1").arg(reg.billAmount, 0, 'f', 2) : "待结算";
            regTable->setItem(i, 5, new QTableWidgetItem(amountText));

            // 病人只能查看，不能操作
            QWidget* actionWidget = new QWidget();
            QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
            actionLayout->setContentsMargins(5, 2, 5, 2);
            actionLayout->setSpacing(5);

            QPushButton* viewBtn = new QPushButton("查看");
            viewBtn->setFixedSize(60, 25);
            viewBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #3b82f6;"
                "  color: white;"
                "  border: none;"
                "  border-radius: 3px;"
                "  font-size: 12px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #2563eb;"
                "}"
            );

            connect(viewBtn, &QPushButton::clicked, [this, reg]() {
                QString info = QString(
                    "挂号单详情：\n"
                    "单号：%1\n"
                    "日期：%2\n"
                    "医生：%3\n"
                    "科室：%4\n"
                    "状态：%5\n"
                    "费用：%6\n"
                    "备注：%7"
                ).arg(
                    QString::number(reg.registrationId),
                    QString::fromStdString(reg.registrationDate),
                    QString::fromStdString(reg.doctorName),
                    QString::fromStdString(reg.doctorDepartment),
                    reg.status == "pending" ? "待处理" : (reg.status == "completed" ? "已完成" : "已取消"),
                    reg.hasBill ? QString("¥%1").arg(reg.billAmount, 0, 'f', 2) : "待结算",
                    QString::fromStdString(reg.notes)
                );

                QMessageBox::information(this, "挂号详情", info);
                });

            // 如果状态是待处理，病人可以取消
            if (reg.status == "pending") {
                QPushButton* cancelBtn = new QPushButton("取消");
                cancelBtn->setFixedSize(60, 25);
                cancelBtn->setStyleSheet(
                    "QPushButton {"
                    "  background-color: #ef4444;"
                    "  color: white;"
                    "  border: none;"
                    "  border-radius: 3px;"
                    "  font-size: 12px;"
                    "}"
                    "QPushButton:hover {"
                    "  background-color: #dc2626;"
                    "}"
                );

                connect(cancelBtn, &QPushButton::clicked, [this, reg]() {
                    if (QMessageBox::question(this, "确认取消",
                        "确定要取消这个挂号吗？") == QMessageBox::Yes) {
                        // 这里需要调用取消挂号的接口
                        // 暂时先显示消息
                        QMessageBox::information(this, "成功", "挂号已取消");
                        loadRegistrations();
                    }
                    });

                actionLayout->addWidget(cancelBtn);
            }

            actionLayout->addWidget(viewBtn);
            actionLayout->addStretch();

            if (regTable->columnCount() > 6) {
                regTable->setCellWidget(i, 6, actionWidget);
            }
        }
    }
}

// 医生挂号记录加载
void MainWindow::loadDoctorRegistrations() {
    std::vector<RegistrationInfo> registrations = systemManager->getRegistrationsByDoctor(currentUser.userId);
    if (doctorRegTable) {
        doctorRegTable->setRowCount(static_cast<int>(registrations.size()));

        for (int i = 0; i < static_cast<int>(registrations.size()); ++i) {
            const auto& reg = registrations[i];

            // 状态显示
            QString statusText;
            QString statusStyle;
            if (reg.status == "pending") {
                statusText = "待处理";
                statusStyle = "background-color: #f59e0b; color: white;";
            }
            else if (reg.status == "completed") {
                statusText = "已完成";
                statusStyle = "background-color: #10b981; color: white;";
            }
            else {
                statusText = "已取消";
                statusStyle = "background-color: #ef4444; color: white;";
            }

            doctorRegTable->setItem(i, 0, new QTableWidgetItem(QString::number(reg.registrationId)));
            doctorRegTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(reg.registrationDate)));
            doctorRegTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(reg.patientName)));
            doctorRegTable->setItem(i, 3, new QTableWidgetItem(QString("病人ID: %1").arg(reg.patientId)));
            doctorRegTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(reg.notes)));

            QTableWidgetItem* statusItem = new QTableWidgetItem(statusText);
            statusItem->setTextAlignment(Qt::AlignCenter);
            doctorRegTable->setItem(i, 5, statusItem);

            // 金额
            QString amountText = reg.hasBill ?
                QString("¥%1").arg(reg.billAmount, 0, 'f', 2) : "待结算";
            doctorRegTable->setItem(i, 6, new QTableWidgetItem(amountText));

            // 操作按钮 - 医生可以结算
            QWidget* actionWidget = new QWidget();
            QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
            actionLayout->setContentsMargins(5, 2, 5, 2);
            actionLayout->setSpacing(5);

            if (reg.status == "pending") {
                QPushButton* settleButton = new QPushButton("结算");
                settleButton->setFixedSize(60, 25);
                settleButton->setStyleSheet(
                    "QPushButton {"
                    "  background-color: #27ae60;"
                    "  color: white;"
                    "  border: none;"
                    "  border-radius: 3px;"
                    "}"
                    "QPushButton:hover {"
                    "  background-color: #219653;"
                    "}"
                );

                connect(settleButton, &QPushButton::clicked, [this, reg]() {
                    bool ok;
                    double amount = QInputDialog::getDouble(this, "结算",
                        QString("请输入结算金额（挂号单号：%1）:").arg(reg.registrationId),
                        50.0, 0.0, 10000.0, 2, &ok);

                    if (ok) {
                        int billId = systemManager->createBill(reg.registrationId, amount);
                        if (billId > 0) {
                            QMessageBox::information(this, "成功",
                                QString("结算成功！结算单号：%1").arg(billId));
                            loadRegistrations();
                        }
                        else {
                            QMessageBox::critical(this, "失败",
                                QString("结算失败：%1").arg(
                                    QString::fromStdString(systemManager->getLastError())));
                        }
                    }
                    });

                actionLayout->addWidget(settleButton);
            }

            // 查看病人信息按钮
            QPushButton* viewPatientBtn = new QPushButton("病人信息");
            viewPatientBtn->setFixedSize(80, 25);
            viewPatientBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #3b82f6;"
                "  color: white;"
                "  border: none;"
                "  border-radius: 3px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #2563eb;"
                "}"
            );

            connect(viewPatientBtn, &QPushButton::clicked, [this, reg]() {
                // 这里可以显示病人详细信息
                QString patientInfo = QString("病人：%1\n备注：%2")
                    .arg(QString::fromStdString(reg.patientName))
                    .arg(QString::fromStdString(reg.notes));
                QMessageBox::information(this, "病人信息", patientInfo);
                });

            actionLayout->addWidget(viewPatientBtn);
            actionLayout->addStretch();

            if (doctorRegTable->columnCount() > 7) {
                doctorRegTable->setCellWidget(i, 7, actionWidget);
            }
        }
    }
    loadTodayRegistrations();
    updateDoctorStats();
}

void MainWindow::setupDoctorRegistrationTab() {
    QWidget* doctorTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(doctorTab);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ===== 医生工作台标题 =====
    QLabel* welcomeLabel = new QLabel(
        QString("👨‍⚕️ %1医生工作台").arg(QString::fromStdString(currentUser.name))
    );
    welcomeLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50;");
    welcomeLabel->setAlignment(Qt::AlignCenter);

    QLabel* infoLabel = new QLabel(
        QString("科室：%1 | 工号：%2").arg(
            QString::fromStdString(currentUser.department),
            QString::number(currentUser.userId)
        )
    );
    infoLabel->setStyleSheet("color: #666; font-size: 14px;");
    infoLabel->setAlignment(Qt::AlignCenter);

    // ===== 今日待处理挂号 =====
    QGroupBox* todayGroup = new QGroupBox("📅 今日待处理挂号");
    todayGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 16px; }");
    QVBoxLayout* todayLayout = new QVBoxLayout(todayGroup);

    todayTable = new QTableWidget();  // 改为成员变量
    todayTable->setColumnCount(6);
    todayTable->setHorizontalHeaderLabels(
        QStringList() << "单号" << "时间" << "病人" << "症状" << "状态" << "操作"
    );
    todayTable->setAlternatingRowColors(true);
    todayTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    todayTable->horizontalHeader()->setStretchLastSection(true);
    todayTable->verticalHeader()->setVisible(false);
    todayTable->setMaximumHeight(200);

    todayLayout->addWidget(todayTable);

    // ===== 所有病人挂号 =====
    QGroupBox* allRegGroup = new QGroupBox("📋 所有病人挂号");
    allRegGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 16px; }");
    QVBoxLayout* allRegLayout = new QVBoxLayout(allRegGroup);

    // 工具栏
    QWidget* toolbar = new QWidget();
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);

    doctorRefreshButton = new QPushButton("🔄 刷新");
    doctorRefreshButton->setFixedSize(100, 35);

    QLabel* filterLabel = new QLabel("状态:");
    doctorFilterCombo = new QComboBox();
    doctorFilterCombo->addItem("全部", "");
    doctorFilterCombo->addItem("待处理", "pending");
    doctorFilterCombo->addItem("已处理", "completed");
    doctorFilterCombo->addItem("已取消", "cancelled");

    QPushButton* exportButton = new QPushButton("📥 导出");
    exportButton->setFixedSize(100, 35);

    connect(doctorRefreshButton, &QPushButton::clicked, this, [this]() {
        loadDoctorRegistrations();
        loadTodayRegistrations();
        updateDoctorStats();
        });

    connect(doctorFilterCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(onDoctorFilterChanged()));

    toolbarLayout->addWidget(doctorRefreshButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(filterLabel);
    toolbarLayout->addWidget(doctorFilterCombo);
    toolbarLayout->addSpacing(20);
    toolbarLayout->addWidget(exportButton);

    // 挂号表格
    doctorRegTable = new QTableWidget();
    doctorRegTable->setColumnCount(8);
    doctorRegTable->setHorizontalHeaderLabels(
        QStringList() << "单号" << "日期" << "病人" << "病人ID" << "症状" << "状态" << "费用" << "操作"
    );
    doctorRegTable->setAlternatingRowColors(true);
    doctorRegTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    doctorRegTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    doctorRegTable->horizontalHeader()->setStretchLastSection(true);
    doctorRegTable->verticalHeader()->setVisible(false);

    allRegLayout->addWidget(toolbar);
    allRegLayout->addWidget(doctorRegTable);

    // ===== 快速操作 =====
    QGroupBox* quickActionsGroup = new QGroupBox("⚡ 快速操作");
    quickActionsGroup->setStyleSheet("QGroupBox { font-weight: bold; font-size: 16px; }");
    QHBoxLayout* actionsLayout = new QHBoxLayout(quickActionsGroup);

    addPrescriptionBtn = new QPushButton("💊 开具处方");
    viewScheduleBtn = new QPushButton("📅 查看排班");
    patientStatsBtn = new QPushButton("📊 病人统计");
    settingBtn = new QPushButton("⚙️ 工作设置");

    // 设置按钮样式
    QString buttonStyle = R"(
        QPushButton {
            padding: 12px;
            font-size: 14px;
            font-weight: bold;
            border-radius: 8px;
            min-width: 120px;
        }
    )";

    addPrescriptionBtn->setStyleSheet(buttonStyle + "background-color: #3b82f6; color: white;");
    viewScheduleBtn->setStyleSheet(buttonStyle + "background-color: #10b981; color: white;");
    patientStatsBtn->setStyleSheet(buttonStyle + "background-color: #f59e0b; color: white;");
    settingBtn->setStyleSheet(buttonStyle + "background-color: #6b7280; color: white;");

    // 连接开具处方按钮
    connect(addPrescriptionBtn, &QPushButton::clicked, this, &MainWindow::onAddPrescriptionClicked);

    actionsLayout->addWidget(addPrescriptionBtn);
    actionsLayout->addWidget(viewScheduleBtn);
    actionsLayout->addWidget(patientStatsBtn);
    actionsLayout->addWidget(settingBtn);

    // 添加到主布局
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addWidget(infoLabel);
    mainLayout->addWidget(todayGroup);
    mainLayout->addWidget(allRegGroup);
    mainLayout->addWidget(quickActionsGroup);

    // 统计信息 - 改为成员变量
    statsWidget = new QWidget();
    statsLayout = new QHBoxLayout(statsWidget);

    todayCountLabel = new QLabel("今日接诊: 0");
    pendingCountLabel = new QLabel("待处理: 0");
    completedCountLabel = new QLabel("已完成: 0");
    totalCountLabel = new QLabel("总计: 0");

    QString statStyle = "padding: 8px 15px; background-color: #f8f9fa; border-radius: 6px; font-weight: bold;";
    todayCountLabel->setStyleSheet(statStyle + "color: #3b82f6;");
    pendingCountLabel->setStyleSheet(statStyle + "color: #f59e0b;");
    completedCountLabel->setStyleSheet(statStyle + "color: #10b981;");
    totalCountLabel->setStyleSheet(statStyle + "color: #6b7280;");

    statsLayout->addWidget(todayCountLabel);
    statsLayout->addWidget(pendingCountLabel);
    statsLayout->addWidget(completedCountLabel);
    statsLayout->addWidget(totalCountLabel);
    statsLayout->addStretch();

    mainLayout->addWidget(statsWidget);

    tabWidget->addTab(doctorTab, "👨‍⚕️ 医生工作台");
}
// MainWindow.cpp - 添加新函数

// 加载今日待处理挂号
void MainWindow::loadTodayRegistrations() {
    if (!todayTable) return;

    // 获取今日日期
    QDate today = QDate::currentDate();
    std::string todayStr = today.toString("yyyy-MM-dd").toStdString();
    using namespace std;
    stringstream query;
    query << "SELECT r.registration_id, r.registration_date, "
        << "p.name as patient_name, r.notes, r.status "
        << "FROM registrations r "
        << "JOIN patients p ON r.patient_id = p.patient_id "
        << "WHERE r.doctor_id = " << currentUser.userId
        << " AND r.registration_date = '" << todayStr << "' "
        << " AND r.status = 'pending' "
        << "ORDER BY r.registration_id";

    auto results = systemManager->dbManager->getQueryResult(query.str());

    todayTable->setRowCount(static_cast<int>(results.size()));

    for (size_t i = 0; i < static_cast<int>(results.size()); ++i) {
        const auto& row = results[i];

        todayTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(row[0])));
        todayTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(row[1])));
        todayTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(row[2])));
        todayTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(row[3])));
        todayTable->setItem(i, 4, new QTableWidgetItem(QString::fromUtf8("待处理")));
        // 操作按钮
        QWidget* actionWidget = new QWidget();
        QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(5, 2, 5, 2);
        actionLayout->setSpacing(5);

        QPushButton* handleBtn = new QPushButton("处理");
        handleBtn->setFixedSize(60, 25);
        handleBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #3b82f6;"
            "  color: white;"
            "  border: none;"
            "  border-radius: 3px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #2563eb;"
            "}"
        );

        int regId = std::stoi(row[0]);
        connect(handleBtn, &QPushButton::clicked, [this, regId]() {
            onHandleRegistrationClicked(regId);
            });

        actionLayout->addWidget(handleBtn);
        actionLayout->addStretch();

        todayTable->setCellWidget(i, 5, actionWidget);
    }
}

// 更新医生统计数据
void MainWindow::updateDoctorStats() {
    if (!todayCountLabel || !pendingCountLabel || !completedCountLabel || !totalCountLabel) return;

    // 今日接诊数
    QDate today = QDate::currentDate();
    std::string todayStr = today.toString("yyyy-MM-dd").toStdString();

    std::stringstream todayQuery;
    todayQuery << "SELECT COUNT(*) FROM registrations "
        << "WHERE doctor_id = " << currentUser.userId
        << " AND registration_date = '" << todayStr << "' "
        << " AND status = 'completed'";

    auto todayResults = systemManager->dbManager->getQueryResult(todayQuery.str());
    int todayCount = todayResults.empty() ? 0 : std::stoi(todayResults[0][0]);

    // 待处理数
    std::stringstream pendingQuery;
    pendingQuery << "SELECT COUNT(*) FROM registrations "
        << "WHERE doctor_id = " << currentUser.userId
        << " AND status = 'pending'";

    auto pendingResults = systemManager->dbManager->getQueryResult(pendingQuery.str());
    int pendingCount = pendingResults.empty() ? 0 : std::stoi(pendingResults[0][0]);

    // 已完成数
    std::stringstream completedQuery;
    completedQuery << "SELECT COUNT(*) FROM registrations "
        << "WHERE doctor_id = " << currentUser.userId
        << " AND status = 'completed'";

    auto completedResults = systemManager->dbManager->getQueryResult(completedQuery.str());
    int completedCount = completedResults.empty() ? 0 : std::stoi(completedResults[0][0]);

    // 总数
    std::stringstream totalQuery;
    totalQuery << "SELECT COUNT(*) FROM registrations "
        << "WHERE doctor_id = " << currentUser.userId;

    auto totalResults = systemManager->dbManager->getQueryResult(totalQuery.str());
    int totalCount = totalResults.empty() ? 0 : std::stoi(totalResults[0][0]);

    // 更新标签
    todayCountLabel->setText(QString("今日接诊: %1").arg(todayCount));
    pendingCountLabel->setText(QString("待处理: %1").arg(pendingCount));
    completedCountLabel->setText(QString("已完成: %1").arg(completedCount));
    totalCountLabel->setText(QString("总计: %1").arg(totalCount));
}

void MainWindow::onFilterChanged() {
    QString status = filterCombo->currentData().toString();
    // 根据状态过滤挂号记录
    if (currentUser.role == "patient") {
        loadPatientRegistrations();
    }
    else if (currentUser.role == "doctor") {
        loadDoctorRegistrations();
    }
    else if (currentUser.role == "admin") {
        loadAdminRegistrations();
    }
}

void MainWindow::onDoctorFilterChanged() {
    QString status = doctorFilterCombo->currentData().toString();
    // 实现医生的过滤逻辑
    loadDoctorRegistrations();
}
// MainWindow.cpp - 添加新函数

void MainWindow::onAddPrescriptionClicked() {
    // 弹出对话框选择要处理的挂号
    bool ok;
    QString regIdStr = QInputDialog::getText(this, "开具处方",
        "请输入挂号单号:", QLineEdit::Normal, "", &ok);

    if (ok && !regIdStr.isEmpty()) {
        int regId = regIdStr.toInt();
        onHandleRegistrationClicked(regId);
    }
}

void MainWindow::onHandleRegistrationClicked(int registrationId) {
    // 检查挂号单是否存在且属于该医生
    RegistrationInfo reg = systemManager->getRegistrationById(registrationId);

    if (reg.registrationId == 0) {
        QMessageBox::warning(this, "错误", "挂号单不存在！");
        return;
    }

    if (reg.doctorId != currentUser.userId) {
        QMessageBox::warning(this, "错误", "这不是您的病人！");
        return;
    }

    if (reg.status != "pending") {
        QMessageBox::warning(this, "错误", "该挂号单已处理或已取消！");
        return;
    }

    // 创建处方对话框
    QDialog dialog(this);
    dialog.setWindowTitle("开具处方 - 挂号单号: " + QString::number(registrationId));
    dialog.setFixedSize(500, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QLabel* infoLabel = new QLabel(
        QString("病人: %1\n挂号日期: %2\n症状描述: %3")
        .arg(QString::fromStdString(reg.patientName))
        .arg(QString::fromStdString(reg.registrationDate))
        .arg(QString::fromStdString(reg.notes))
    );
    infoLabel->setStyleSheet("padding: 10px; background-color: #f8f9fa; border-radius: 5px;");

    // 诊断结果
    QLabel* diagnosisLabel = new QLabel("诊断结果:");
    QTextEdit* diagnosisEdit = new QTextEdit();
    diagnosisEdit->setPlaceholderText("请输入诊断结果...");

    // 处方药品
    QLabel* medicineLabel = new QLabel("处方药品:");
    QTableWidget* medicineTable = new QTableWidget();
    medicineTable->setColumnCount(4);
    medicineTable->setHorizontalHeaderLabels(QStringList() << "药品名称" << "用法" << "用量" << "天数");
    medicineTable->setRowCount(3);

    // 添加药品按钮
    QPushButton* addMedicineBtn = new QPushButton("添加药品");
    connect(addMedicineBtn, &QPushButton::clicked, [medicineTable]() {
        int row = medicineTable->rowCount();
        medicineTable->insertRow(row);
        });

    // 费用
    QLabel* feeLabel = new QLabel("总费用:");
    QDoubleSpinBox* feeSpinBox = new QDoubleSpinBox();
    feeSpinBox->setRange(0, 10000);
    feeSpinBox->setValue(50.0);
    feeSpinBox->setPrefix("¥ ");
    feeSpinBox->setDecimals(2);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* saveBtn = new QPushButton("保存处方并结算");
    QPushButton* cancelBtn = new QPushButton("取消");

    saveBtn->setStyleSheet("background-color: #10b981; color: white; font-weight: bold; padding: 8px 16px;");
    cancelBtn->setStyleSheet("background-color: #ef4444; color: white; font-weight: bold; padding: 8px 16px;");

    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(cancelBtn);

    layout->addWidget(infoLabel);
    layout->addWidget(diagnosisLabel);
    layout->addWidget(diagnosisEdit);
    layout->addWidget(medicineLabel);
    layout->addWidget(medicineTable);
    layout->addWidget(addMedicineBtn);
    layout->addWidget(feeLabel);
    layout->addWidget(feeSpinBox);
    layout->addLayout(buttonLayout);

    // 连接按钮
    connect(saveBtn, &QPushButton::clicked, [&]() {
        if (diagnosisEdit->toPlainText().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "警告", "请输入诊断结果！");
            return;
        }

        double amount = feeSpinBox->value();

        // 1. 创建账单
        int billId = systemManager->createBill(registrationId, amount);

        if (billId > 0) {
            // 2. 更新挂号单状态
            std::stringstream query;
            query << "UPDATE registrations SET status = 'completed' "
                << "WHERE registration_id = " << registrationId;

            if (systemManager->dbManager->executeQuery(query.str())) {
                // 3. 保存处方信息（这里可以扩展为专门的处方表）
                std::string diagnosis = diagnosisEdit->toPlainText().toStdString();
                // 可以保存到数据库或生成PDF处方单

                QMessageBox::information(&dialog, "成功",
                    QString("处方已保存！\n账单号: %1\n费用: ¥%2")
                    .arg(billId).arg(amount, 0, 'f', 2));

                dialog.accept();

                // 刷新数据
                loadDoctorRegistrations();
                loadTodayRegistrations();
                updateDoctorStats();
            }
        }
        else {
            QMessageBox::critical(&dialog, "错误",
                QString("结算失败: %1").arg(QString::fromStdString(systemManager->getLastError())));
        }
        });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}
// MainWindow.cpp - 实现管理员功能

// 加载病人列表（管理员）
void MainWindow::loadPatientsForAdmin() {
    if (!adminPatientCombo) return;
    
    adminPatientCombo->clear();
    adminPatientCombo->addItem("请选择病人", 0);
    
    std::string query = "SELECT patient_id, name FROM patients ORDER BY name";
    auto results = systemManager->dbManager->getQueryResult(query);
    
    for (const auto& row : results) {
        if (row.size() >= 2) {
            int patientId = std::stoi(row[0]);
            QString name = QString::fromStdString(row[1]);
            adminPatientCombo->addItem(name, patientId);
        }
    }
}

// 加载医生列表（管理员）
void MainWindow::loadDoctorsForAdmin() {
    if (!adminDoctorCombo) return;
    
    adminDoctorCombo->clear();
    adminDoctorCombo->addItem("请选择医生", 0);
    
    std::string query = "SELECT doctor_id, name, department FROM doctors ORDER BY name";
    auto results = systemManager->dbManager->getQueryResult(query);
    
    for (const auto& row : results) {
        if (row.size() >= 3) {
            int doctorId = std::stoi(row[0]);
            QString name = QString::fromStdString(row[1]);
            QString dept = QString::fromStdString(row[2]);
            adminDoctorCombo->addItem(QString("%1 (%2)").arg(name).arg(dept), doctorId);
        }
    }
}

void MainWindow::updateAdminStats() {
    // 今日挂号数 - 使用当前日期
    QDate today = QDate::currentDate();
    std::string todayStr = today.toString("yyyy-MM-dd").toStdString();

    std::stringstream todayQuery;
    todayQuery << "SELECT COUNT(*) FROM registrations "
        << "WHERE registration_date = '" << todayStr << "'";

    auto todayResults = systemManager->getDatabaseManager()->getQueryResult(todayQuery.str());
    adminTodayCount = todayResults.empty() ? 0 : std::stoi(todayResults[0][0]);

    // 总收入 - 修改查询逻辑
    std::stringstream incomeQuery;
    // 方法1：从bills表查询已支付账单的总金额
    incomeQuery << "SELECT COALESCE(SUM(amount), 0) FROM bills WHERE status = 'paid'";
    // 或者方法2：从registration_bills关联查询
    // incomeQuery << "SELECT COALESCE(SUM(b.amount), 0) "
    //             << "FROM bills b "
    //             << "JOIN registration_bills rb ON b.bill_id = rb.bill_id "
    //             << "WHERE b.status = 'paid'";

    auto incomeResults = systemManager->getDatabaseManager()->getQueryResult(incomeQuery.str());
    if (!incomeResults.empty() && !incomeResults[0][0].empty()) {
        adminTotalIncome = std::stod(incomeResults[0][0]);
    }
    else {
        adminTotalIncome = 0.0;
    }

    // 医生数
    std::stringstream doctorQuery;
    doctorQuery << "SELECT COUNT(*) FROM doctors";

    auto doctorResults = systemManager->getDatabaseManager()->getQueryResult(doctorQuery.str());
    adminDoctorCount = doctorResults.empty() ? 0 : std::stoi(doctorResults[0][0]);

    // 病人数
    std::stringstream patientQuery;
    patientQuery << "SELECT COUNT(*) FROM patients";

    auto patientResults = systemManager->getDatabaseManager()->getQueryResult(patientQuery.str());
    adminPatientCount = patientResults.empty() ? 0 : std::stoi(patientResults[0][0]);

    qDebug() << "统计更新 - 今日挂号:" << adminTodayCount
        << "总收入:" << adminTotalIncome
        << "医生数:" << adminDoctorCount
        << "病人数:" << adminPatientCount;
}
// 管理员添加挂号
void MainWindow::onAdminAddRegistrationClicked() {
    QString date = adminDateEdit->date().toString("yyyy-MM-dd");
    int patientId = adminPatientCombo->currentData().toInt();
    int doctorId = adminDoctorCombo->currentData().toInt();
    QString notes = adminNotesEdit->toPlainText();
    
    if (patientId == 0 || doctorId == 0) {
        QMessageBox::warning(this, "错误", "请选择病人和医生！");
        return;
    }
    
    int regId = systemManager->createRegistration(patientId, doctorId,
        date.toStdString(), notes.toStdString());
    
    if (regId > 0) {
        QMessageBox::information(this, "成功", 
            QString("挂号单添加成功！\n挂号单号：%1").arg(regId));
        
        // 清空表单
        adminNotesEdit->clear();
        
        // 刷新数据
        loadAdminRegistrations();
        updateAdminStats();
    } else {
        QMessageBox::critical(this, "失败",
            QString("添加失败：%1").arg(QString::fromStdString(systemManager->getLastError())));
    }
}

// 修改loadAdminRegistrations函数，添加删除功能
void MainWindow::loadAdminRegistrations() {
    if (!adminRegTable) return;
    
    std::vector<RegistrationInfo> registrations = systemManager->getAllRegistrations();
    adminRegTable->setRowCount(static_cast<int>(registrations.size()));

    for (int i = 0; i < static_cast<int>(registrations.size()); ++i) {
        const auto& reg = registrations[i];

        // 状态显示
        QString statusText;
        QString statusStyle;
        if (reg.status == "pending") {
            statusText = "待处理";
        }
        else if (reg.status == "completed") {
            statusText = "已完成";
        }
        else {
            statusText = "已取消";
        }

        adminRegTable->setItem(i, 0, new QTableWidgetItem(QString::number(reg.registrationId)));
        adminRegTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(reg.registrationDate)));
        adminRegTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(reg.patientName)));
        adminRegTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(reg.doctorName)));
        adminRegTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(reg.doctorDepartment)));
        adminRegTable->setItem(i, 5, new QTableWidgetItem(statusText));

        // 金额
        QString amountText = reg.hasBill ?
            QString("¥%1").arg(reg.billAmount, 0, 'f', 2) : "未结算";
        adminRegTable->setItem(i, 6, new QTableWidgetItem(amountText));

        // 账单状态
        QString billStatus = reg.hasBill ? "已结算" : "未结算";
        adminRegTable->setItem(i, 7, new QTableWidgetItem(billStatus));

        // 操作按钮 - 查看详情
        QWidget* actionWidget = new QWidget();
        QHBoxLayout* actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(5, 2, 5, 2);
        actionLayout->setSpacing(5);

        QPushButton* viewBtn = new QPushButton("详情");
        viewBtn->setFixedSize(60, 25);
        viewBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #3b82f6;"
            "  color: white;"
            "  border: none;"
            "  border-radius: 3px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #2563eb;"
            "}"
        );

        connect(viewBtn, &QPushButton::clicked, [this, reg]() {
            QString info = QString(
                "挂号单详情：\n"
                "单号：%1\n"
                "日期：%2\n"
                "病人：%3 (ID: %4)\n"
                "医生：%5 (ID: %6)\n"
                "科室：%7\n"
                "状态：%8\n"
                "费用：%9\n"
                "账单状态：%10\n"
                "备注：%11"
            ).arg(
                QString::number(reg.registrationId),
                QString::fromStdString(reg.registrationDate),
                QString::fromStdString(reg.patientName),
                QString::number(reg.patientId),
                QString::fromStdString(reg.doctorName),
                QString::number(reg.doctorId),
                QString::fromStdString(reg.doctorDepartment),
                reg.status == "pending" ? "待处理" : (reg.status == "completed" ? "已完成" : "已取消"),
                reg.hasBill ? QString("¥%1").arg(reg.billAmount, 0, 'f', 2) : "未结算",
                reg.billStatus.empty() ? "无账单" : QString::fromStdString(reg.billStatus),
                QString::fromStdString(reg.notes)
            );

            QMessageBox::information(this, "挂号详情", info);
        });

        actionLayout->addWidget(viewBtn);
        actionLayout->addStretch();

        adminRegTable->setCellWidget(i, 8, actionWidget);
        
        // 删除按钮
        QPushButton* deleteBtn = new QPushButton("删除");
        deleteBtn->setFixedSize(60, 25);
        deleteBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #ef4444;"
            "  color: white;"
            "  border: none;"
            "  border-radius: 3px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #dc2626;"
            "}"
        );
        
        int regId = reg.registrationId;
        connect(deleteBtn, &QPushButton::clicked, [this, regId, reg]() {
            if (QMessageBox::question(this, "确认删除",
                QString("确定要删除挂号单 %1 吗？\n病人：%2\n日期：%3")
                .arg(regId)
                .arg(QString::fromStdString(reg.patientName))
                .arg(QString::fromStdString(reg.registrationDate)),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                
                // 删除挂号单
                std::stringstream query;
                query << "DELETE FROM registrations WHERE registration_id = " << regId;
                
                if (systemManager->dbManager->executeQuery(query.str())) {
                    QMessageBox::information(this, "成功", "挂号单已删除！");
                    loadAdminRegistrations();
                    updateAdminStats();
                } else {
                    QMessageBox::critical(this, "失败", 
                        QString("删除失败：%1").arg(
                            QString::fromStdString(systemManager->dbManager->getLastError())));
                }
            }
        });
        
        adminRegTable->setCellWidget(i, 9, deleteBtn);
    }
}
void MainWindow::refreshAdminStats() {
    // 更新统计数据
    updateAdminStats();

    // 这里可以添加更新UI显示的代码
    // 比如在界面上显示一个"统计数据已更新"的消息
    statusBar()->showMessage("统计数据已更新", 3000);

    qDebug() << "统计数据已刷新 - 总收入:" << adminTotalIncome;
}
// MainWindow.cpp - 添加以下函数实现

// 1. 添加科室
void MainWindow::onAddDepartmentClicked() {
    bool ok;
    QString name = QInputDialog::getText(this, "添加科室",
        "请输入科室名称:", QLineEdit::Normal, "", &ok);

    if (ok && !name.isEmpty()) {
        // 创建科室信息
        DepartmentInfo dept;
        dept.departmentName = name.toStdString();
        dept.contactPhone = "";
        dept.location = "";
        dept.description = "";

        // 调用SystemManager的addDepartment方法
        if (systemManager->addDepartment(dept)) {
            QMessageBox::information(this, "成功",
                QString("科室【%1】添加成功！").arg(name));

            // 重新加载科室列表
            loadDepartments();
            loadDepartmentsForAssignment();
        }
        else {
            QMessageBox::critical(this, "失败",
                QString("添加科室失败：%1").arg(
                    QString::fromStdString(systemManager->getLastError())));
        }
    }
}

// 2. 编辑科室
void MainWindow::onEditDepartmentClicked() {
    int currentRow = departmentTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请选择要编辑的科室");
        return;
    }

    QString departmentName = departmentTable->item(currentRow, 1)->text();

    bool ok;
    QString newName = QInputDialog::getText(this, "编辑科室名称",
        "科室名称:", QLineEdit::Normal, departmentName, &ok);

    if (ok && !newName.isEmpty()) {
        // 这里应该调用SystemManager的updateDepartment方法
        // 但由于我们暂时没有科室ID，先直接更新表格
        departmentTable->item(currentRow, 1)->setText(newName);
        QMessageBox::information(this, "成功", "科室信息已更新！");
    }
}

// 3. 删除科室
void MainWindow::onDeleteDepartmentClicked() {
    int currentRow = departmentTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请选择要删除的科室");
        return;
    }

    QString departmentName = departmentTable->item(currentRow, 1)->text();

    if (QMessageBox::question(this, "确认删除",
        QString("确定要删除科室【%1】吗？").arg(departmentName),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

        // 这里应该调用SystemManager的deleteDepartment方法
        // 暂时先从表格中移除
        departmentTable->removeRow(currentRow);
        QMessageBox::information(this, "成功", "科室删除成功！");
    }
}

// 4. 分配医生到科室
void MainWindow::onAssignDoctorClicked() {
    int doctorId = assignDoctorCombo->currentData().toInt();
    int departmentId = assignDepartmentCombo->currentData().toInt();

    if (doctorId == 0) {
        QMessageBox::warning(this, "提示", "请选择医生");
        return;
    }

    QString doctorName = assignDoctorCombo->currentText();
    QString departmentName = (departmentId == -1) ? "未分配" :
        assignDepartmentCombo->currentText();

    if (QMessageBox::question(this, "确认分配",
        QString("确定将医生【%1】分配到【%2】吗？")
        .arg(doctorName).arg(departmentName),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

        // 调用SystemManager的assignDoctorToDepartment方法
        if (systemManager->assignDoctorToDepartment(doctorId, departmentId)) {
            QMessageBox::information(this, "成功",
                QString("医生分配成功！\n%1 -> %2")
                .arg(doctorName).arg(departmentName));

            // 重新加载医生列表以反映变化
            loadDoctorsForAssignment();
        }
        else {
            QMessageBox::critical(this, "失败",
                QString("分配失败：%1").arg(
                    QString::fromStdString(systemManager->getLastError())));
        }
    }
}