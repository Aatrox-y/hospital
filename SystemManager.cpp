#include "SystemManager.h"
#include <sstream>
#include <iomanip>
#include<qdebug.h>
SystemManager::SystemManager()
    : dbManager(std::make_unique<DatabaseManager>()) {
}

SystemManager::~SystemManager() {
}

std::vector<std::vector<std::string>> SystemManager::executeRawQuery(const std::string& query) {
    return dbManager->getQueryResult(query);
}

bool SystemManager::initialize(const std::string& host, const std::string& user,const std::string& password, const std::string& database,unsigned int port) {

    if (!dbManager->connect(host, user, password, database, port)) {
        lastError = dbManager->getLastError();
        return false;
    }

    // 初始化数据库表结构
    if (!dbManager->initializeDatabase()) {
        lastError = dbManager->getLastError();
        return false;
    }

    return true;
}

std::vector<RegistrationInfo> SystemManager::getAllRegistrations() {
    std::vector<RegistrationInfo> registrations;

    std::string query = R"(
        SELECT r.registration_id, r.registration_date, r.patient_id, r.doctor_id, 
               r.status, r.notes, p.name as patient_name, d.name as doctor_name, d.department,
               CASE WHEN rb.bill_id IS NOT NULL THEN 1 ELSE 0 END as has_bill,
               COALESCE(b.amount, 0) as bill_amount, COALESCE(b.status, '') as bill_status
        FROM registrations r
        JOIN patients p ON r.patient_id = p.patient_id
        JOIN doctors d ON r.doctor_id = d.doctor_id
        LEFT JOIN registration_bills rb ON r.registration_id = rb.registration_id
        LEFT JOIN bills b ON rb.bill_id = b.bill_id
        ORDER BY r.registration_date DESC
    )";

    auto results = dbManager->getQueryResult(query);
    for (const auto& row : results) {
        registrations.push_back(parseRegistrationInfo(row));
    }

    return registrations;
}

RegistrationInfo SystemManager::getRegistrationById(int registrationId) {
    RegistrationInfo info;

    std::stringstream query;
    query << "SELECT r.registration_id, r.registration_date, r.patient_id, r.doctor_id, "
        << "r.status, r.notes, p.name as patient_name, d.name as doctor_name, d.department, "
        << "CASE WHEN rb.bill_id IS NOT NULL THEN 1 ELSE 0 END as has_bill, "
        << "COALESCE(b.amount, 0) as bill_amount, COALESCE(b.status, '') as bill_status "
        << "FROM registrations r "
        << "JOIN patients p ON r.patient_id = p.patient_id "
        << "JOIN doctors d ON r.doctor_id = d.doctor_id "
        << "LEFT JOIN registration_bills rb ON r.registration_id = rb.registration_id "
        << "LEFT JOIN bills b ON rb.bill_id = b.bill_id "
        << "WHERE r.registration_id = " << registrationId;

    auto results = dbManager->getQueryResult(query.str());
    if (!results.empty()) {
        info = parseRegistrationInfo(results[0]);
    }

    return info;
}

std::vector<RegistrationInfo> SystemManager::getRegistrationsByDoctor(int doctorId) {
    std::vector<RegistrationInfo> registrations;

    std::stringstream query;
    query << "SELECT r.registration_id, r.registration_date, r.patient_id, r.doctor_id, "
        << "r.status, r.notes, p.name as patient_name, d.name as doctor_name, d.department, "
        << "CASE WHEN rb.bill_id IS NOT NULL THEN 1 ELSE 0 END as has_bill, "
        << "COALESCE(b.amount, 0) as bill_amount, COALESCE(b.status, '') as bill_status "
        << "FROM registrations r "
        << "JOIN patients p ON r.patient_id = p.patient_id "
        << "JOIN doctors d ON r.doctor_id = d.doctor_id "
        << "LEFT JOIN registration_bills rb ON r.registration_id = rb.registration_id "
        << "LEFT JOIN bills b ON rb.bill_id = b.bill_id "
        << "WHERE r.doctor_id = " << doctorId
        << " ORDER BY r.registration_date DESC";

    auto results = dbManager->getQueryResult(query.str());
    for (const auto& row : results) {
        registrations.push_back(parseRegistrationInfo(row));
    }

    return registrations;
}

std::vector<DoctorInfo> SystemManager::getDoctorsByDepartment(const std::string& department) {
    std::vector<DoctorInfo> doctors;

    std::stringstream query;
    query << "SELECT doctor_id, name, gender, age, phone, department "
        << "FROM doctors WHERE department = '" << dbManager->escapeString(department) << "' "
        << "ORDER BY name";

    auto results = dbManager->getQueryResult(query.str());
    for (const auto& row : results) {
        doctors.push_back(parseDoctorInfo(row));
    }

    return doctors;
}

UserInfo SystemManager::login(const std::string& username, const std::string& password) {
    UserInfo userInfo;
    std::string hashedPassword = hashPassword(password);

    std::stringstream query;
    query << "SELECT u.user_id, u.username, u.role, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.name "
        << "WHEN 'doctor' THEN d.name "
        << "WHEN 'admin' THEN '系统管理员' "
        << "END as name, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.gender "
        << "WHEN 'doctor' THEN d.gender "
        << "WHEN 'admin' THEN 'male' "
        << "END as gender, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.age "
        << "WHEN 'doctor' THEN d.age "
        << "WHEN 'admin' THEN 30 "
        << "END as age, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.phone "
        << "WHEN 'doctor' THEN d.phone "
        << "ELSE '' "
        << "END as phone, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.address "
        << "ELSE '' "
        << "END as address, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.id_card "
        << "ELSE '' "
        << "END as id_card, "
        << "CASE u.role "
        << "WHEN 'doctor' THEN d.department "
        << "ELSE '' "
        << "END as department "
        << "FROM users u "
        << "LEFT JOIN patients p ON u.user_id = p.patient_id AND u.role = 'patient' "
        << "LEFT JOIN doctors d ON u.user_id = d.doctor_id AND u.role = 'doctor' "
        << "WHERE u.username = '" << dbManager->escapeString(username) << "' "
        << "AND u.password_hash = '" << hashedPassword << "'";

    auto results = dbManager->getQueryResult(query.str());
    if (results.empty()) {
        lastError = "用户名或密码错误";
        return userInfo;
    }

    return parseUserInfo(results[0]);
}

bool SystemManager::registerUser(const std::string& username, const std::string& password,const std::string& role, const UserInfo& userInfo) {

    // 检查用户名是否存在
    std::string checkQuery = "SELECT user_id FROM users WHERE username = '" +
        dbManager->escapeString(username) + "'";
    auto results = dbManager->getQueryResult(checkQuery);
    if (!results.empty()) {
        lastError = "用户名已存在";
        return false;
    }

    std::string hashedPassword = hashPassword(password);
    std::string escapedUsername = dbManager->escapeString(username);
    std::string escapedName = dbManager->escapeString(userInfo.name);
    std::string escapedGender = dbManager->escapeString(userInfo.gender);
    std::string escapedPhone = dbManager->escapeString(userInfo.phone);

    // 开始事务
    if (!dbManager->startTransaction()) {
        lastError = dbManager->getLastError();
        return false;
    }

    // 1. 在users表创建用户
    std::stringstream userQuery;
    userQuery << "INSERT INTO users (username, password_hash, role) VALUES ('"
        << escapedUsername << "', '" << hashedPassword << "', '" << role << "')";

    if (!dbManager->executeQuery(userQuery.str())) {
        dbManager->rollbackTransaction();
        lastError = dbManager->getLastError();
        return false;
    }

    int userId = dbManager->getLastInsertId();

    // 2. 根据角色插入详细信息
    if (role == "patient") {
        std::string escapedAddress = dbManager->escapeString(userInfo.address);
        std::string escapedIdCard = dbManager->escapeString(userInfo.idCard);

        std::stringstream patientQuery;
        patientQuery << "INSERT INTO patients (patient_id, name, gender, age, address, phone, id_card) VALUES ("
            << userId << ", '" << escapedName << "', '" << escapedGender << "', "
            << userInfo.age << ", '" << escapedAddress << "', '" << escapedPhone
            << "', '" << escapedIdCard << "')";

        if (!dbManager->executeQuery(patientQuery.str())) {
            dbManager->rollbackTransaction();
            lastError = dbManager->getLastError();
            return false;
        }
    }
    else if (role == "doctor") {
        std::string escapedDepartment = dbManager->escapeString(userInfo.department);

        std::stringstream doctorQuery;
        doctorQuery << "INSERT INTO doctors (doctor_id, name, gender, age, phone, department) VALUES ("
            << userId << ", '" << escapedName << "', '" << escapedGender << "', "
            << userInfo.age << ", '" << escapedPhone << "', '" << escapedDepartment << "')";

        if (!dbManager->executeQuery(doctorQuery.str())) {
            dbManager->rollbackTransaction();
            lastError = dbManager->getLastError();
            return false;
        }
    }

    if (!dbManager->commitTransaction()) {
        lastError = dbManager->getLastError();
        return false;
    }

    return true;
}

UserInfo SystemManager::getUserInfo(int userId) {
    UserInfo userInfo;

    std::stringstream query;
    query << "SELECT u.user_id, u.username, u.role, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.name "
        << "WHEN 'doctor' THEN d.name "
        << "WHEN 'admin' THEN '系统管理员' "
        << "END as name, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.gender "
        << "WHEN 'doctor' THEN d.gender "
        << "WHEN 'admin' THEN 'male' "
        << "END as gender, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.age "
        << "WHEN 'doctor' THEN d.age "
        << "WHEN 'admin' THEN 30 "
        << "END as age, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.phone "
        << "WHEN 'doctor' THEN d.phone "
        << "ELSE '' "
        << "END as phone, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.address "
        << "ELSE '' "
        << "END as address, "
        << "CASE u.role "
        << "WHEN 'patient' THEN p.id_card "
        << "ELSE '' "
        << "END as id_card, "
        << "CASE u.role "
        << "WHEN 'doctor' THEN d.department "
        << "ELSE '' "
        << "END as department "
        << "FROM users u "
        << "LEFT JOIN patients p ON u.user_id = p.patient_id AND u.role = 'patient' "
        << "LEFT JOIN doctors d ON u.user_id = d.doctor_id AND u.role = 'doctor' "
        << "WHERE u.user_id = " << userId;

    auto results = dbManager->getQueryResult(query.str());
    if (results.empty()) {
        lastError = "用户不存在";
        return userInfo;
    }

    return parseUserInfo(results[0]);
}

bool SystemManager::updateUserInfo(const UserInfo& userInfo) {
    std::string escapedName = dbManager->escapeString(userInfo.name);
    std::string escapedGender = dbManager->escapeString(userInfo.gender);
    std::string escapedPhone = dbManager->escapeString(userInfo.phone);

    if (userInfo.role == "patient") {
        std::string escapedAddress = dbManager->escapeString(userInfo.address);
        std::string escapedIdCard = dbManager->escapeString(userInfo.idCard);

        std::stringstream query;
        query << "UPDATE patients SET name = '" << escapedName
            << "', gender = '" << escapedGender
            << "', age = " << userInfo.age
            << ", address = '" << escapedAddress
            << "', phone = '" << escapedPhone
            << "', id_card = '" << escapedIdCard
            << "' WHERE patient_id = " << userInfo.userId;

        return dbManager->executeQuery(query.str());
    }
    else if (userInfo.role == "doctor") {
        std::string escapedDepartment = dbManager->escapeString(userInfo.department);

        std::stringstream query;
        query << "UPDATE doctors SET name = '" << escapedName
            << "', gender = '" << escapedGender
            << "', age = " << userInfo.age
            << ", phone = '" << escapedPhone
            << "', department = '" << escapedDepartment
            << "' WHERE doctor_id = " << userInfo.userId;

        return dbManager->executeQuery(query.str());
    }

    lastError = "不支持的用户角色";
    return false;
}

int SystemManager::createRegistration(int patientId, int doctorId,const std::string& date, const std::string& notes) {

    std::string escapedDate = dbManager->escapeString(date);
    std::string escapedNotes = dbManager->escapeString(notes);

    std::stringstream query;
    query << "INSERT INTO registrations (registration_date, patient_id, doctor_id, notes) "
        << "VALUES ('" << escapedDate << "', " << patientId << ", " << doctorId
        << ", '" << escapedNotes << "')";

    if (!dbManager->executeQuery(query.str())) {
        lastError = dbManager->getLastError();
        return -1;
    }

    return dbManager->getLastInsertId();
}

std::vector<RegistrationInfo> SystemManager::getRegistrationsByPatient(int patientId) {
    std::vector<RegistrationInfo> registrations;

    std::stringstream query;
    query << "SELECT r.registration_id, r.registration_date, r.patient_id, r.doctor_id, "
        << "r.status, r.notes, p.name as patient_name, d.name as doctor_name, d.department, "
        << "CASE WHEN rb.bill_id IS NOT NULL THEN 1 ELSE 0 END as has_bill, "
        << "COALESCE(b.amount, 0) as bill_amount, COALESCE(b.status, '') as bill_status "
        << "FROM registrations r "
        << "JOIN patients p ON r.patient_id = p.patient_id "
        << "JOIN doctors d ON r.doctor_id = d.doctor_id "
        << "LEFT JOIN registration_bills rb ON r.registration_id = rb.registration_id "
        << "LEFT JOIN bills b ON rb.bill_id = b.bill_id "
        << "WHERE r.patient_id = " << patientId
        << " ORDER BY r.registration_date DESC";

    auto results = dbManager->getQueryResult(query.str());
    for (const auto& row : results) {
        registrations.push_back(parseRegistrationInfo(row));
    }

    return registrations;
}

int SystemManager::createBill(int registrationId, double amount) {
    // 开始事务
    if (!dbManager->startTransaction()) {
        lastError = dbManager->getLastError();
        return -1;
    }

    // 1. 创建账单
    std::stringstream billQuery;
    billQuery << "INSERT INTO bills (bill_date, amount) VALUES (CURDATE(), " << amount << ")";

    if (!dbManager->executeQuery(billQuery.str())) {
        dbManager->rollbackTransaction();
        lastError = dbManager->getLastError();
        return -1;
    }

    int billId = dbManager->getLastInsertId();

    // 2. 关联挂号单和账单
    std::stringstream linkQuery;
    linkQuery << "INSERT INTO registration_bills (registration_id, bill_id) VALUES ("
        << registrationId << ", " << billId << ")";

    if (!dbManager->executeQuery(linkQuery.str())) {
        dbManager->rollbackTransaction();
        lastError = dbManager->getLastError();
        return -1;
    }

    // 3. 更新挂号单状态
    std::stringstream updateQuery;
    updateQuery << "UPDATE registrations SET status = 'completed' WHERE registration_id = " << registrationId;

    if (!dbManager->executeQuery(updateQuery.str())) {
        dbManager->rollbackTransaction();
        lastError = dbManager->getLastError();
        return -1;
    }

    if (!dbManager->commitTransaction()) {
        lastError = dbManager->getLastError();
        return -1;
    }

    return billId;
}

// 辅助函数
std::string SystemManager::hashPassword(const std::string& password) {
    std::string query = "SELECT MD5('" + dbManager->escapeString(password) + "')";
    auto results = dbManager->getQueryResult(query);
    if (!results.empty() && !results[0].empty()) {
        return results[0][0];
    }
    return password;
}

UserInfo SystemManager::parseUserInfo(const std::vector<std::string>& row) {
    UserInfo info;
    if (row.size() >= 9) {
        info.userId = !row[0].empty() ? std::stoi(row[0]) : 0;
        info.username = row[1];
        info.role = row[2];
        info.name = row[3];
        info.gender = row[4];
        info.age = !row[5].empty() ? std::stoi(row[5]) : 0;
        info.phone = row[6];
        info.address = row.size() > 7 ? row[7] : "";
        info.idCard = row.size() > 8 ? row[8] : "";
        info.department = row.size() > 9 ? row[9] : "";
    }
    return info;
}

DoctorInfo SystemManager::parseDoctorInfo(const std::vector<std::string>& row) {
    DoctorInfo info;
    if (row.size() >= 6) {
        info.doctorId = !row[0].empty() ? std::stoi(row[0]) : 0;
        info.name = row[1];
        info.gender = row[2];
        info.age = !row[3].empty() ? std::stoi(row[3]) : 0;
        info.phone = row[4];
        info.department = row[5];
    }
    return info;
}

RegistrationInfo SystemManager::parseRegistrationInfo(const std::vector<std::string>& row) {
    RegistrationInfo info;
    if (row.size() >= 12) {
        info.registrationId = !row[0].empty() ? std::stoi(row[0]) : 0;
        info.registrationDate = row[1];
        info.patientId = !row[2].empty() ? std::stoi(row[2]) : 0;
        info.doctorId = !row[3].empty() ? std::stoi(row[3]) : 0;
        info.status = row[4];
        info.notes = row[5];
        info.patientName = row[6];
        info.doctorName = row[7];
        info.doctorDepartment = row[8];
        info.hasBill = !row[9].empty() && row[9] == "1";
        info.billAmount = !row[10].empty() ? std::stod(row[10]) : 0.0;
        info.billStatus = row.size() > 11 ? row[11] : "";
    }
    return info;
}

std::string SystemManager::getLastError() const {
    return lastError;
}
// 获取所有科室
std::vector<DoctorInfo> SystemManager::getAllDoctors() {
    std::vector<DoctorInfo> doctors;

    if (!dbManager || !dbManager->isConnected()) {
        std::cerr << "错误：数据库未连接" << std::endl;
        return doctors;
    }

    try {
        // 使用更简单的查询，确保能获取数据
        std::string query = "SELECT doctor_id, name, gender, age, phone, department "
            "FROM doctors ORDER BY name";

        std::cout << "执行SQL查询医生: " << query << std::endl;

        auto results = dbManager->getQueryResult(query);
        std::cout << "查询结果行数: " << results.size() << std::endl;

        // 输出查询结果以便调试
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << "第" << i << "行数据: ";
            for (const auto& field : results[i]) {
                std::cout << field << " | ";
            }
            std::cout << std::endl;
        }

        for (const auto& row : results) {
            if (row.size() >= 6) {
                DoctorInfo info;
                try {
                    info.doctorId = !row[0].empty() ? std::stoi(row[0]) : 0;
                    info.name = row[1];
                    info.gender = row[2];
                    info.age = !row[3].empty() ? std::stoi(row[3]) : 0;
                    info.phone = row[4];
                    info.department = row[5];

                    doctors.push_back(info);

                    std::cout << "成功解析医生: " << info.name
                        << " (ID: " << info.doctorId
                        << "), 科室: " << info.department << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "解析医生数据异常: " << e.what() << std::endl;
                }
            }
        }

        if (doctors.empty()) {
            std::cout << "警告：没有找到医生数据" << std::endl;

            // 检查users表中是否有医生用户
            std::string checkUsersQuery = "SELECT COUNT(*) FROM users WHERE role = 'doctor'";
            auto userResults = dbManager->getQueryResult(checkUsersQuery);
            if (!userResults.empty() && userResults[0][0] != "0") {
                std::cout << "users表中有医生用户，但doctors表中没有对应记录" << std::endl;
            }
        }

    }
    catch (const std::exception& e) {
        std::cerr << "获取医生列表异常: " << e.what() << std::endl;
        lastError = "获取医生列表失败: " + std::string(e.what());
    }

    std::cout << "最终返回医生数量: " << doctors.size() << std::endl;
    return doctors;
}
// 根据ID获取科室
DepartmentInfo SystemManager::getDepartmentById(int departmentId) {
    DepartmentInfo dept;

    std::stringstream query;
    query << "SELECT department_id, department_name, "
        << "description, contact_phone, location "
        << "FROM departments WHERE department_id = " << departmentId;

    auto results = dbManager->getQueryResult(query.str());
    if (!results.empty()) {
        dept = parseDepartmentInfo(results[0]);
    }

    return dept;
}
// 根据名称获取科室
DepartmentInfo SystemManager::getDepartmentByName(const std::string& name) {
    DepartmentInfo dept;

    std::stringstream query;
    query << "SELECT department_id, department_name, "
        << "description, contact_phone, location "
        << "FROM departments WHERE department_name = '"
        << dbManager->escapeString(name) << "'";

    auto results = dbManager->getQueryResult(query.str());
    if (!results.empty()) {
        dept = parseDepartmentInfo(results[0]);
    }

    return dept;
}

bool SystemManager::addDepartment(const DepartmentInfo& department) {
    // 检查科室名称是否已存在
    std::string checkQuery = "SELECT COUNT(*) FROM departments WHERE department_name = '"
        + dbManager->escapeString(department.departmentName) + "'";

    auto results = dbManager->getQueryResult(checkQuery);
    if (!results.empty() && results[0][0] != "0") {
        lastError = "科室名称已存在";
        return false;
    }

    std::stringstream query;
    query << "INSERT INTO departments (department_name, description, "
        << "contact_phone, location) VALUES ('"
        << dbManager->escapeString(department.departmentName) << "', '"
        << dbManager->escapeString(department.description) << "', '"
        << dbManager->escapeString(department.contactPhone) << "', '"
        << dbManager->escapeString(department.location) << "')";

    if (dbManager->executeQuery(query.str())) {
        return true;
    }
    else {
        lastError = dbManager->getLastError();
        return false;
    }
}

bool SystemManager::updateDepartment(const DepartmentInfo& department) {
    if (department.departmentId <= 0) {
        lastError = "无效的科室ID";
        return false;
    }

    // 检查新的科室名称是否与其他科室冲突
    std::string checkQuery = "SELECT COUNT(*) FROM departments WHERE department_name = '"
        + dbManager->escapeString(department.departmentName)
        + "' AND department_id != " + std::to_string(department.departmentId);

    auto results = dbManager->getQueryResult(checkQuery);
    if (!results.empty() && results[0][0] != "0") {
        lastError = "科室名称已存在";
        return false;
    }

    std::stringstream query;
    query << "UPDATE departments SET "
        << "department_name = '" << dbManager->escapeString(department.departmentName) << "', "
        << "description = '" << dbManager->escapeString(department.description) << "', "
        << "contact_phone = '" << dbManager->escapeString(department.contactPhone) << "', "
        << "location = '" << dbManager->escapeString(department.location) << "' "
        << "WHERE department_id = " << department.departmentId;

    if (dbManager->executeQuery(query.str())) {
        return true;
    }
    else {
        lastError = dbManager->getLastError();
        return false;
    }
}

bool SystemManager::deleteDepartment(int departmentId) {
    // 检查是否有医生关联
    std::string checkQuery = "SELECT COUNT(*) FROM doctors WHERE department_id = "
        + std::to_string(departmentId);

    auto results = dbManager->getQueryResult(checkQuery);
    if (!results.empty() && results[0][0] != "0") {
        lastError = "该科室下还有医生，无法删除";
        return false;
    }

    std::string query = "DELETE FROM departments WHERE department_id = "
        + std::to_string(departmentId);

    if (dbManager->executeQuery(query)) {
        return true;
    }
    else {
        lastError = dbManager->getLastError();
        return false;
    }
}

// 获取科室下的医生
std::vector<DoctorInfo> SystemManager::getDoctorsByDepartment(int departmentId) {
    std::vector<DoctorInfo> doctors;

    std::stringstream query;
    query << "SELECT d.doctor_id, d.name, d.gender, d.age, "
        << "d.phone, dept.department_name "
        << "FROM doctors d "
        << "LEFT JOIN departments dept ON d.department_id = dept.department_id "
        << "WHERE d.department_id = " << departmentId
        << " ORDER BY d.name";

    auto results = dbManager->getQueryResult(query.str());
    for (const auto& row : results) {
        doctors.push_back(parseDoctorInfo(row));
    }

    return doctors;
}

std::vector<DepartmentInfo> SystemManager::getAllDepartments() {
    std::vector<DepartmentInfo> departments;

    std::string query = "SELECT department_id, department_name, "
        "description, contact_phone, location "
        "FROM departments ORDER BY department_name";

    auto results = dbManager->getQueryResult(query);
    for (const auto& row : results) {
        departments.push_back(parseDepartmentInfo(row));
    }

    return departments;
}
// 分配医生到科室
bool SystemManager::assignDoctorToDepartment(int doctorId, int departmentId) {
    // 1. 检查医生是否存在
    std::string checkDoctorQuery = "SELECT COUNT(*) FROM doctors WHERE doctor_id = "
        + std::to_string(doctorId);

    auto doctorResults = dbManager->getQueryResult(checkDoctorQuery);
    if (doctorResults.empty() || doctorResults[0][0] == "0") {
        lastError = "医生不存在";
        return false;
    }

    // 2. 检查科室是否存在（如果分配的是有效科室）
    if (departmentId > 0) {
        std::string checkDeptQuery = "SELECT COUNT(*) FROM departments WHERE department_id = "
            + std::to_string(departmentId);

        auto deptResults = dbManager->getQueryResult(checkDeptQuery);
        if (deptResults.empty() || deptResults[0][0] == "0") {
            lastError = "科室不存在";
            return false;
        }
    }

    // 3. 获取科室名称（如果分配的是有效科室）
    std::string departmentName = "";
    if (departmentId > 0) {
        std::string getDeptNameQuery = "SELECT department_name FROM departments WHERE department_id = "
            + std::to_string(departmentId);

        auto nameResults = dbManager->getQueryResult(getDeptNameQuery);
        if (!nameResults.empty() && !nameResults[0].empty()) {
            departmentName = nameResults[0][0];
        }
    }

    // 4. 执行更新操作
    std::stringstream query;
    query << "UPDATE doctors SET ";

    if (departmentId > 0) {
        query << "department_id = " << departmentId << ", ";
        if (!departmentName.empty()) {
            query << "department = '" << dbManager->escapeString(departmentName) << "' ";
        }
        else {
            query << "department = NULL ";
        }
    }
    else {
        query << "department_id = NULL, department = NULL ";
    }

    query << "WHERE doctor_id = " << doctorId;

    // 打印调试信息
    std::cout << "执行SQL: " << query.str() << std::endl;

    if (dbManager->executeQuery(query.str())) {
        // 记录操作日志（去掉currentUser的引用）
        std::string logQuery = "INSERT INTO operation_logs (operation_type, target_id, details) VALUES (";
        logQuery += "'assign_doctor', " + std::to_string(doctorId) + ", '";
        logQuery += "分配医生到科室: " + (departmentId > 0 ? std::to_string(departmentId) : "未分配") + "')";

        dbManager->executeQuery(logQuery); // 忽略日志错误
        return true;
    }
    else {
        lastError = dbManager->getLastError();
        std::cout << "SQL执行错误: " << lastError << std::endl;
        return false;
    }
}

// 获取可挂号的科室（有医生的科室）
std::vector<DepartmentInfo> SystemManager::getAvailableDepartmentsForRegistration() {
    std::vector<DepartmentInfo> departments;

    std::string query = R"(
        SELECT DISTINCT d.department_id, d.department_name, 
               d.description, d.contact_phone, d.location
        FROM departments d
        INNER JOIN doctors doc ON d.department_id = doc.department_id
        WHERE doc.department_id IS NOT NULL
        ORDER BY d.department_name
    )";

    auto results = dbManager->getQueryResult(query);
    for (const auto& row : results) {
        departments.push_back(parseDepartmentInfo(row));
    }

    return departments;
}

// 解析科室信息
DepartmentInfo SystemManager::parseDepartmentInfo(const std::vector<std::string>& row) {
    DepartmentInfo info;
    if (row.size() >= 5) {
        info.departmentId = !row[0].empty() ? std::stoi(row[0]) : 0;
        info.departmentName = row[1];
        info.description = row[2];
        info.contactPhone = row[3];
        info.location = row[4];
    }
    return info;
}