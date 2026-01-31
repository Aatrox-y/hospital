#pragma once

// Windows MySQL配置
#ifdef _WIN32
#include <winsock2.h>
#include <mysql.h>
#pragma comment(lib, "libmysql.lib")
#else
#include <mysql/mysql.h>
#endif

#include <string>
#include <vector>
#include <memory>
#include <iostream>

class DatabaseManager {
private:
    MYSQL* connection;
    std::string lastError;

public:
    DatabaseManager();
    ~DatabaseManager();

    // 数据库连接
    bool connect(const std::string& host = "127.0.0.1",
        const std::string& user = "root",
        const std::string& password = "",
        const std::string& database = "hospital_system",
        unsigned int port = 3306);

    void disconnect();
    bool isConnected() const;

    // 数据库初始化
    bool initializeDatabase();

    // SQL执行
    bool executeQuery(const std::string& query);
    MYSQL_RES* executeQueryWithResult(const std::string& query);
    int getLastInsertId();
    std::vector<std::vector<std::string>> getQueryResult(const std::string& query);

    // 事务管理
    bool startTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // 工具函数
    std::string escapeString(const std::string& str);
    std::string getLastError() const;

private:
    // 防止拷贝
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
};