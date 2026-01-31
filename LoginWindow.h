#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include "SystemManager.h"

class LoginWindow : public QMainWindow {
    Q_OBJECT

public:
    LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    // UI组件
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;

    QLabel* titleLabel;

    QWidget* loginForm;
    QFormLayout* formLayout;
    QLabel* usernameLabel;
    QLineEdit* usernameEdit;
    QLabel* passwordLabel;
    QLineEdit* passwordEdit;
    QLabel* roleLabel;
    QComboBox* roleComboBox;

    QWidget* buttonWidget;
    QHBoxLayout* buttonLayout;
    QPushButton* loginButton;
    QPushButton* registerButton;

    // 系统管理器
    std::unique_ptr<SystemManager> systemManager;

    // 初始化函数
    void setupUI();
    void applyStyles();
    bool initializeSystem();
};