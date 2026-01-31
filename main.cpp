#include "LoginWindow.h"
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QStyleFactory>
#include <QMessageBox>
#include<qfile.h>
#include<qdebug.h>
#include<qdir.h>
int main(int argc, char* argv[]) {
    //在这！！！！！！！！！！
    QApplication app(argc, argv);
    // 设置应用程序信息
    app.setApplicationName("医院挂号管理系统");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("医院管理系统");
    QFont font("Microsoft YaHei", 10);
    app.setFont(font);

    // 设置应用程序样式
    app.setStyle("Fusion");
    // 加载样式表
    QFile styleFile("styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        app.setStyleSheet(styleSheet);
    }
    else {
        // 使用默认样式
        app.setStyleSheet(R"(
            QMainWindow {
                background-color: #f5f7fa;
            }
            
            QPushButton {
                background-color: #3498db;
                border: none;
                color: white;
                padding: 8px 16px;
                border-radius: 4px;
                font-weight: bold;
            }
            
            QPushButton:hover {
                background-color: #2980b9;
            }
            
            QPushButton:pressed {
                background-color: #1c6ea4;
            }
            
            QPushButton:disabled {
                background-color: #bdc3c7;
                color: #7f8c8d;
            }
            
            QLineEdit, QTextEdit, QComboBox {
                border: 2px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
                background-color: white;
            }
            
            QLineEdit:focus, QTextEdit:focus, QComboBox:focus {
                border-color: #3498db;
            }
            
            QGroupBox {
                border: 2px solid #3498db;
                border-radius: 6px;
                margin-top: 10px;
                padding-top: 10px;
                font-weight: bold;
            }
            
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                left: 10px;
                padding: 0 5px;
            }
            
            QTableWidget {
                background-color: white;
                alternate-background-color: #f8f9fa;
                selection-background-color: #3498db;
                selection-color: white;
                border: 1px solid #dcdde1;
            }
            
            QTableWidget::item {
                padding: 8px;
            }
            
            QHeaderView::section {
                background-color: #3498db;
                color: white;
                padding: 8px;
                border: none;
                font-weight: bold;
            }
            
            QTabWidget::pane {
                border: 1px solid #dcdde1;
                background-color: white;
            }
            
            QTabBar::tab {
                background-color: #ecf0f1;
                border: 1px solid #dcdde1;
                padding: 8px 16px;
                margin-right: 2px;
            }
            
            QTabBar::tab:selected {
                background-color: white;
                border-bottom: 2px solid #3498db;
            }
            
            QTabBar::tab:hover {
                background-color: #d6eaf8;
            }
        )");
    }

    // 创建并显示登录窗口
    LoginWindow loginWindow;
    loginWindow.show();
    return app.exec();
}