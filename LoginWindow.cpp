#include "LoginWindow.h"
#include "MainWindow.h"
#include <QApplication>
#include <QFile>

LoginWindow::LoginWindow(QWidget* parent)
    : QMainWindow(parent) {

    setWindowTitle("医院挂号系统 - 登录");
    setFixedSize(600, 400);

    // 初始化系统
    if (!initializeSystem()) {
        QMessageBox::critical(this, "错误", "系统初始化失败，请检查数据库配置！");
        QApplication::quit();
        return;
    }

    // 初始化UI
    setupUI();
    applyStyles();

    // 连接信号槽
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
}

LoginWindow::~LoginWindow() {
}

bool LoginWindow::initializeSystem() {
    systemManager = std::make_unique<SystemManager>();
    return systemManager->initialize("127.0.0.1", "aaaa", "mysql123", "hospital_system", 3306);
}

void LoginWindow::setupUI() {
    // 设置窗口背景
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(240, 245, 249));
    setPalette(pal);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30,30, 30, 30);
    // 标题
    titleLabel = new QLabel("🏥 医院挂号管理系统");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2c3e50;");
    mainLayout->addWidget(titleLabel);

    // 登录表单
    loginForm = new QWidget();
    formLayout = new QFormLayout(loginForm);
    formLayout->setSpacing(15);

    usernameLabel = new QLabel("用户名:");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("请输入用户名");
    usernameEdit->setMinimumHeight(35);

    passwordLabel = new QLabel("密码:");
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMinimumHeight(35);

    roleLabel = new QLabel("角色:");
    roleComboBox = new QComboBox();
    roleComboBox->addItem("病人", "patient");
    roleComboBox->addItem("医生", "doctor");
    roleComboBox->addItem("管理员", "admin");
    roleComboBox->setMinimumHeight(35);

    formLayout->addRow(usernameLabel, usernameEdit);
    formLayout->addRow(passwordLabel, passwordEdit);
    formLayout->addRow(roleLabel, roleComboBox);

    mainLayout->addWidget(loginForm);

    // 按钮
    buttonWidget = new QWidget();
    buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setSpacing(20);

    loginButton = new QPushButton("登录");
    loginButton->setFixedHeight(40);
    loginButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    registerButton = new QPushButton("注册");
    registerButton->setFixedHeight(40);
    registerButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);

    mainLayout->addWidget(buttonWidget);
    mainLayout->addStretch();
}

void LoginWindow::applyStyles() {
    QFile styleFile("styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        setStyleSheet(styleSheet);
    }
}

void LoginWindow::onLoginClicked() {
    QString username = usernameEdit->text().trimmed();//去掉文本前后的空白标识符
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码！");
        return;
    }

    UserInfo user = systemManager->login(username.toStdString(), password.toStdString());

    if (user.userId != 0) {
        QString roleName;
        if (user.role == "patient") roleName = "病人";
        else if (user.role == "doctor") roleName = "医生";
        else roleName = "管理员";

        QMessageBox::information(this, "登录成功",
            QString("欢迎 %1，您已成功登录！").arg(QString::fromStdString(user.name)));

        // 打开主窗口
        MainWindow* mainWindow = new MainWindow(systemManager.release(), user);
        mainWindow->show();

        // 关闭登录窗口
        this->close();
    }
    else {
        QMessageBox::critical(this, "登录失败",
            QString("登录失败：%1").arg(QString::fromStdString(systemManager->getLastError())));
    }
}

void LoginWindow::onRegisterClicked() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();
    QString role = roleComboBox->currentData().toString();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码！");
        return;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, "密码太短", "密码长度不能少于6位！");
        return;
    }

    // 准备用户信息
    UserInfo userInfo;
    userInfo.name = username.toStdString() + "(" + roleComboBox->currentText().toStdString() + ")";
    userInfo.gender = "male";
    userInfo.age = 25;
    userInfo.phone = "";

    if (role == "patient") {
        userInfo.address = "";
        userInfo.idCard = "";
        // 或者提示用户输入
        QMessageBox::information(this, "提示", "病人注册需要身份证号，请在个人信息页面补充");
    }
    else if (role == "doctor") {
        userInfo.department = "未分配科室";
    }

    if (systemManager->registerUser(username.toStdString(), password.toStdString(),
        role.toStdString(), userInfo)) {
        QMessageBox::information(this, "注册成功", "用户注册成功，请使用新账号登录！");

        // 清空表单
        usernameEdit->clear();
        passwordEdit->clear();
    }
    else {
        QMessageBox::critical(this, "注册失败",
            QString("注册失败：%1").arg(QString::fromStdString(systemManager->getLastError())));
    }
}