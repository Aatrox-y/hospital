#include "DatabaseManager.h"
#include <sstream>
#include <iomanip>


DatabaseManager::DatabaseManager() : connection(nullptr) {
    // 初始化MySQL库（Windows需要）
#ifdef _WIN32
    if (mysql_library_init(0, NULL, NULL) != 0) {
        std::cerr << "MySQL库初始化失败" << std::endl;
    }
#endif
}

DatabaseManager::~DatabaseManager() {
    disconnect();
#ifdef _WIN32
    mysql_library_end();
#endif
}

bool DatabaseManager::connect(const std::string& host, const std::string& user,const std::string& password, const std::string& database,unsigned int port) {


    connection = mysql_init(nullptr);
    if (!connection) {
        lastError = "MySQL初始化失败";
        return false;
    }

    // 设置连接选项
    mysql_options(connection, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    // 连接数据库
    if (!mysql_real_connect(connection, host.c_str(), user.c_str(), password.c_str(),
        database.c_str(), port, nullptr, CLIENT_MULTI_STATEMENTS)) {
        lastError = mysql_error(connection);
        mysql_close(connection);
        connection = nullptr;
        return false;
    }

    // 设置自动重连
    bool reconnect = 1;
    mysql_options(connection, MYSQL_OPT_RECONNECT, &reconnect);

    std::cout << "成功连接到数据库: " << database << std::endl;
    return true;
}

void DatabaseManager::disconnect() {
    if (connection) {
        mysql_close(connection);
        connection = nullptr;
        std::cout << "数据库连接已关闭" << std::endl;
    }
}

bool DatabaseManager::isConnected() const {
    if (!connection) return false;
    return mysql_ping(connection) == 0;
}

bool DatabaseManager::initializeDatabase() {

    // 创建数据库
    if (!executeQuery("CREATE DATABASE IF NOT EXISTS hospital_system CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci")) {
        return false;
    }

    // 使用数据库
    if (!executeQuery("USE hospital_system")) {
        return false;
    }

    // 创建用户表
    std::string createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id INT AUTO_INCREMENT PRIMARY KEY,
            username VARCHAR(50) UNIQUE NOT NULL,
            password_hash VARCHAR(255) NOT NULL,
            role ENUM('patient', 'doctor', 'admin') NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!executeQuery(createUsersTable)) {
        return false;
    }

    // 创建病人表
   // 修改原来的CREATE TABLE语句
    std::string createPatientsTable = R"(
    CREATE TABLE IF NOT EXISTS patients (
        patient_id INT PRIMARY KEY,
        name VARCHAR(100) NOT NULL,
        gender ENUM('male', 'female', 'other') DEFAULT 'male',
        age INT,
        address TEXT,
        phone VARCHAR(20),
        id_card VARCHAR(18),
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (patient_id) REFERENCES users(user_id) ON DELETE CASCADE
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
)";

    if (!executeQuery(createPatientsTable)) {
        return false;
    }
        std::string createDepartmentsTable = R"(
        CREATE TABLE IF NOT EXISTS departments (
            department_id INT AUTO_INCREMENT PRIMARY KEY,
            department_name VARCHAR(50) NOT NULL UNIQUE,
            description TEXT,
            contact_phone VARCHAR(20),
            department_id INT NULL,
            location VARCHAR(100),
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!executeQuery(createDepartmentsTable)) {
        return false;
    }

    // 插入默认科室数据
    std::string insertDepartments = R"(
        INSERT IGNORE INTO departments (department_name, description, contact_phone, location) VALUES
        ('内科', '诊治内科疾病，如感冒、高血压、糖尿病等', '010-12345671', '门诊楼2层201-210室'),
        ('外科', '手术治疗，包括普外科、骨科、神经外科等', '010-12345672', '门诊楼3层301-315室'),
        ('儿科', '0-14岁儿童疾病诊治', '010-12345673', '门诊楼1层101-110室'),
        ('妇产科', '妇科和产科疾病，女性健康', '010-12345674', '门诊楼4层401-410室'),
        ('眼科', '眼部疾病诊治，视力检查', '010-12345675', '门诊楼5层501-505室'),
        ('口腔科', '牙齿和口腔疾病治疗', '010-12345676', '门诊楼6层601-605室'),
        ('耳鼻喉科', '耳鼻喉相关疾病', '010-12345677', '门诊楼3层316-320室'),
        ('皮肤科', '皮肤病诊治', '010-12345678', '门诊楼2层211-215室'),
        ('中医科', '中医诊疗服务', '010-12345679', '门诊楼1层111-115室'),
        ('康复科', '术后康复、物理治疗', '010-12345680', '康复中心1层'),
        ('急诊科', '24小时急诊服务', '010-12345681', '急诊楼1层')
    )";

    if (!executeQuery(insertDepartments)) {
        return false;
    }

   
    // 创建医生表
    std::string createDoctorsTable = R"(
        CREATE TABLE IF NOT EXISTS doctors (
            doctor_id INT PRIMARY KEY,
            name VARCHAR(100) NOT NULL,
            gender ENUM('male', 'female', 'other') DEFAULT 'male',
            age INT,
            phone VARCHAR(20),
            department VARCHAR(100) NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (doctor_id) REFERENCES users(user_id) ON DELETE CASCADE
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!executeQuery(createDoctorsTable)) {
        return false;
    }
    // 修改医生表，添加科室外键
    std::string alterDoctorsTable = R"(
        ALTER TABLE doctors 
        ADD COLUMN IF NOT EXISTS department_id INT,
        ADD FOREIGN KEY IF NOT EXISTS fk_doctors_department 
        FOREIGN KEY (department_id) REFERENCES departments(department_id) ON DELETE SET NULL
    )";
    // 创建挂号表
    std::string createRegistrationsTable = R"(
        CREATE TABLE IF NOT EXISTS registrations (
            registration_id INT AUTO_INCREMENT PRIMARY KEY,
            registration_date DATE NOT NULL,
            patient_id INT NOT NULL,
            doctor_id INT NOT NULL,
            status ENUM('pending', 'completed', 'cancelled') DEFAULT 'pending',
            notes TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (patient_id) REFERENCES patients(patient_id),
            FOREIGN KEY (doctor_id) REFERENCES doctors(doctor_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!executeQuery(createRegistrationsTable)) {
        return false;
    }

    // 创建账单表
    std::string createBillsTable = R"(
        CREATE TABLE IF NOT EXISTS bills (
            bill_id INT AUTO_INCREMENT PRIMARY KEY,
            bill_date DATE NOT NULL,
            amount DECIMAL(10,2) NOT NULL,
            status ENUM('unpaid', 'paid') DEFAULT 'unpaid',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!executeQuery(createBillsTable)) {
        return false;
    }

    // 创建挂号-账单关联表
    std::string createRegistrationBillsTable = R"(
        CREATE TABLE IF NOT EXISTS registration_bills (
            registration_id INT PRIMARY KEY,
            bill_id INT UNIQUE,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (registration_id) REFERENCES registrations(registration_id),
            FOREIGN KEY (bill_id) REFERENCES bills(bill_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!executeQuery(createRegistrationBillsTable)) {
        return false;
    }

    // 创建默认管理员账户
    std::string createAdminUser = R"(
        INSERT IGNORE INTO users (username, password_hash, role) 
        VALUES ('admin', MD5('admin123'), 'admin')
    )";

    if (!executeQuery(createAdminUser)) {
        return false;
    }

    std::cout << "数据库初始化完成" << std::endl;
    return true;
}

bool DatabaseManager::executeQuery(const std::string& query) {

    if (!isConnected()) {
        lastError = "数据库未连接";
        return false;
    }

    if (mysql_query(connection, query.c_str()) != 0) {
        lastError = mysql_error(connection);
        return false;
    }

    return true;
}

MYSQL_RES* DatabaseManager::executeQueryWithResult(const std::string& query) {

    if (!isConnected()) {
        lastError = "数据库未连接";
        return nullptr;
    }

    if (mysql_query(connection, query.c_str()) != 0) {
        lastError = mysql_error(connection);
        return nullptr;
    }

    return mysql_store_result(connection);
}

int DatabaseManager::getLastInsertId() {
    return mysql_insert_id(connection);
}

std::vector<std::vector<std::string>> DatabaseManager::getQueryResult(const std::string& query) {
    std::vector<std::vector<std::string>> results;

    MYSQL_RES* result = executeQueryWithResult(query);
    if (!result) {
        return results;
    }

    MYSQL_ROW row;
    int num_fields = mysql_num_fields(result);

    while ((row = mysql_fetch_row(result))) {
        std::vector<std::string> row_data;
        for (int i = 0; i < num_fields; i++) {
            row_data.push_back(row[i] ? row[i] : "");
        }
        results.push_back(row_data);
    }

    mysql_free_result(result);
    return results;
}

bool DatabaseManager::startTransaction() {
    return executeQuery("START TRANSACTION");
}

bool DatabaseManager::commitTransaction() {
    return executeQuery("COMMIT");
}

bool DatabaseManager::rollbackTransaction() {
    return executeQuery("ROLLBACK");
}

std::string DatabaseManager::escapeString(const std::string& str) {
    if (!connection) return str;

    char* escaped = new char[str.length() * 2 + 1];
    mysql_real_escape_string(connection, escaped, str.c_str(), str.length());
    std::string result(escaped);
    delete[] escaped;
    return result;
}

std::string DatabaseManager::getLastError() const {
    return lastError;
}