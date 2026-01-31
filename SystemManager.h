#pragma once
#include "DatabaseManager.h"
#include "CommonTypes.h"
#include <memory>
#include <string>
#include <vector>

class SystemManager {
private:
   
    std::string lastError;

public:
    std::unique_ptr<DatabaseManager> dbManager;
    SystemManager();
    ~SystemManager();
    DatabaseManager* getDatabaseManager() {
        return dbManager.get();
    }

    // 新增：执行原始查询
    std::vector<std::vector<std::string>> executeRawQuery(const std::string& query);

    // 初始化系统
    bool initialize(const std::string& host = "127.0.0.1",
        const std::string& user = "root",
        const std::string& password = "",
        const std::string& database = "hospital_system",
        unsigned int port = 3306);

    // 用户认证
    UserInfo login(const std::string& username, const std::string& password);
    bool registerUser(const std::string& username, const std::string& password,
        const std::string& role, const UserInfo& userInfo);
    bool changePassword(int userId, const std::string& oldPassword,
        const std::string& newPassword);

    // 用户管理
    UserInfo getUserInfo(int userId);
    bool updateUserInfo(const UserInfo& userInfo);

    // 医生管理
    std::vector<DoctorInfo> getAllDoctors();
    std::vector<DoctorInfo> getDoctorsByDepartment(const std::string& department);

    // 病人管理
    UserInfo getPatientInfo(int patientId);

    // 挂号管理
    int createRegistration(int patientId, int doctorId,
        const std::string& date, const std::string& notes = "");
    std::vector<RegistrationInfo> getRegistrationsByPatient(int patientId);
    std::vector<RegistrationInfo> getRegistrationsByDoctor(int doctorId);
    std::vector<RegistrationInfo> getAllRegistrations();
    RegistrationInfo getRegistrationById(int registrationId);
    bool updateRegistrationStatus(int registrationId,
        const std::string& status, const std::string& notes = "");

    // 结算管理
    int createBill(int registrationId, double amount);
    BillInfo getBillByRegistrationId(int registrationId);
    std::vector<DepartmentInfo> getAllDepartments();

    // 根据ID获取科室
    DepartmentInfo getDepartmentById(int departmentId);

    // 根据名称获取科室
    DepartmentInfo getDepartmentByName(const std::string& name);

    // 添加科室
    bool addDepartment(const DepartmentInfo& department);

    // 更新科室信息
    bool updateDepartment(const DepartmentInfo& department);

    // 删除科室（如果没有医生关联）
    bool deleteDepartment(int departmentId);

    // 获取科室下的医生
    std::vector<DoctorInfo> getDoctorsByDepartment(int departmentId);

    // 分配医生到科室
    bool assignDoctorToDepartment(int doctorId, int departmentId);

    // 获取可挂号的科室（有医生的科室）
    std::vector<DepartmentInfo> getAvailableDepartmentsForRegistration();
    // 错误处理
    std::string getLastError() const;

private:
    std::string hashPassword(const std::string& password);
    UserInfo parseUserInfo(const std::vector<std::string>& row);
    DoctorInfo parseDoctorInfo(const std::vector<std::string>& row);
    RegistrationInfo parseRegistrationInfo(const std::vector<std::string>& row);
    // 解析科室信息
    DepartmentInfo parseDepartmentInfo(const std::vector<std::string>& row);

};