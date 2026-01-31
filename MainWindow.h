#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QStatusBar>
#include <QSpinBox>
#include <QLineEdit>
#include "SystemManager.h"
#include "CommonTypes.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(SystemManager* systemManager, const UserInfo& userInfo, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLogoutClicked();
    void onNewRegistrationClicked();
    void onDepartmentSelected(int index);
    void refreshRegistrations();
    void onSaveProfileClicked();
    void onChangePasswordClicked();
    void onAddDepartmentClicked();
    void onEditDepartmentClicked();
    void onDeleteDepartmentClicked();
    void onAssignDoctorClicked();
    void onAddPrescriptionClicked();
    void onHandleRegistrationClicked(int registrationId);
    void onFilterChanged();
    void onDoctorFilterChanged();
    void onAdminAddRegistrationClicked();
    void refreshAdminStats();
private:
    void initializeTestDoctors();
    // 系统管理器
    SystemManager* systemManager;
    UserInfo currentUser;

    // UI组件
    QTabWidget* tabWidget;

    // 首页标签页
    QWidget* homeTab;
    QLabel* welcomeLabel;

    // 挂号管理标签页 - 根据角色不同显示不同界面
    QWidget* registrationTab;

    // 病人挂号相关组件
    QDateEdit* dateEdit;
    QComboBox* departmentCombo;
    QComboBox* doctorCombo;
    QTextEdit* notesEdit;
    QPushButton* submitButton;

    // 通用组件
    QTableWidget* regTable;
    QPushButton* refreshButton;
    QComboBox* filterCombo;

    // 医生工作台组件
    QTableWidget* doctorRegTable;
    QPushButton* doctorRefreshButton;
    QTableWidget* todayTable;
    QWidget* statsWidget;
    QHBoxLayout* statsLayout;
    QLabel* todayCountLabel;
    QLabel* pendingCountLabel;
    QLabel* completedCountLabel;
    QLabel* totalCountLabel;
    QPushButton* addPrescriptionBtn;
    QPushButton* viewScheduleBtn;      // 添加
    QPushButton* patientStatsBtn;      // 添加
    QPushButton* settingBtn;           // 添加
    QPushButton* exportButton;         // 添加

    // 个人信息标签页
    QWidget* profileTab;
    QLineEdit* nameEdit;
    QComboBox* genderCombo;
    QSpinBox* ageSpin;
    QLineEdit* phoneEdit;
    QLineEdit* addressEdit;
    QLineEdit* idCardEdit;
    QLineEdit* departmentEdit;
    QPushButton* saveProfileButton;

    // 密码修改
    QLineEdit* oldPasswordEdit;
    QLineEdit* newPasswordEdit;
    QLineEdit* confirmPasswordEdit;
    QPushButton* changePasswordButton;

    // 医生列表标签页
    QWidget* doctorsTab;
    QTableWidget* doctorsTable;

    // 科室管理标签页
    QWidget* departmentTab;
    QTableWidget* departmentTable;
    QPushButton* addDepartmentButton;
    QPushButton* editDepartmentButton;
    QPushButton* deleteDepartmentButton;
    QPushButton* refreshDepartmentButton;
    QComboBox* assignDoctorCombo;
    QComboBox* assignDepartmentCombo;
    QPushButton* assignButton;

    // 管理员界面组件
    QDateEdit* adminDateEdit;
    QComboBox* adminPatientCombo;
    QComboBox* adminDoctorCombo;
    QTextEdit* adminNotesEdit;
    QTableWidget* adminRegTable;
    QDateEdit* startDateEdit;          // 添加
    QDateEdit* endDateEdit;            // 添加
    QComboBox* deptFilterCombo;        // 添加
    QComboBox* doctorFilterCombo;      // 添加
    QComboBox* statusFilterCombo;      // 添加
    QPushButton* filterButton;         // 添加
    QPushButton* resetButton;          // 添加
    QPushButton* exportAllButton;      // 添加
    QPushButton* backupBtn;            // 添加
    QPushButton* restoreBtn;           // 添加
    QPushButton* reportBtn;            // 添加
    QPushButton* systemLogBtn;         // 添加

    int adminTodayCount;
    double adminTotalIncome;
    int adminDoctorCount;
    int adminPatientCount;

    // 初始化函数
    void setupUI();
    void setupHomeTab();
    void setupRegistrationTab();
    void setupProfileTab();
    void setupDoctorsTab();
    void setupDepartmentTab();
    void applyStyles();

    // 管理员相关函数
    void loadPatientsForAdmin();
    void loadDoctorsForAdmin();
    void updateAdminStats();
    void onAdminDeleteRegistrationClicked(int registrationId);

    // 根据角色设置不同的挂号管理界面
    void setupPatientRegistrationTab();
    void setupDoctorRegistrationTab();
    void setupAdminRegistrationTab();

    // 根据角色加载不同的挂号数据
    void loadPatientRegistrations();
    void loadDoctorRegistrations();
    void loadAdminRegistrations();

    // 医生工作台相关函数
    void loadTodayRegistrations();
    void updateDoctorStats();

    // 过滤函数
    void filterRegistrations(const QString& status);

    // 数据加载函数
    void loadUserData();
    void loadDoctors();
    void loadRegistrations();
    void loadDepartments();
    void loadDoctorsForAssignment();
    void loadDepartmentsForAssignment();

    // 工具函数
    QString getRoleDisplayName(const std::string& role);
    QString getGenderDisplayName(const std::string& gender);

    // 处方相关函数
    void showPrescriptionDialog(int registrationId);

    // 结算相关函数
    void settleRegistration(int registrationId, double amount);

    // 挂号取消函数
    void cancelRegistration(int registrationId);

    // 查看病人信息
    void showPatientInfo(int patientId);

    // 查看挂号详情
    void showRegistrationDetails(const RegistrationInfo& reg);

    // 导出数据函数
    void exportRegistrations();
    void exportDoctorRegistrations();

    // 系统管理函数
    void backupDatabase();
    void restoreDatabase();
    void generateReport();
    void showSystemLog();

    // 科室管理相关函数
    void showAddDepartmentDialog();
    void showEditDepartmentDialog(int departmentId);
    void showAssignDoctorDialog();

    // 医生分配函数
    void assignDoctorToDepartment(int doctorId, int departmentId);

    // 刷新函数
    void refreshDoctorData();
    void refreshAdminData();
    void refreshAllData();
};