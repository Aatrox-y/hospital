// CommonTypes.h - 修复版本
#pragma once

// 添加必要的包含
#include <string>
#include <vector>

struct UserInfo {
    int userId = 0;
    std::string username;
    std::string role;
    std::string name;
    std::string gender = "male";
    int age = 0;
    std::string phone;
    std::string address;
    std::string idCard;
    std::string department;

    // 添加默认构造函数
    UserInfo() = default;

    UserInfo(int id, const std::string& uname, const std::string& r,
        const std::string& n, const std::string& g, int a,
        const std::string& p, const std::string& addr = "",
        const std::string& idc = "", const std::string& dept = "")
        : userId(id), username(uname), role(r), name(n),
        gender(g), age(a), phone(p), address(addr),
        idCard(idc), department(dept) {
    }
};

struct DoctorInfo {
    int doctorId = 0;
    std::string name;
    std::string gender;
    int age = 0;
    std::string phone;
    std::string department;

    // 添加默认构造函数
    DoctorInfo() = default;

    DoctorInfo(int id, const std::string& n, const std::string& g,
        int a, const std::string& p, const std::string& dept)
        : doctorId(id), name(n), gender(g), age(a), phone(p), department(dept) {
    }
};

struct RegistrationInfo {
    int registrationId = 0;
    std::string registrationDate;
    int patientId = 0;
    int doctorId = 0;
    std::string status = "pending";
    std::string notes;
    std::string patientName;
    std::string doctorName;
    std::string doctorDepartment;

    // 结算信息
    bool hasBill = false;
    double billAmount = 0.0;
    std::string billStatus;

    // 添加默认构造函数
    RegistrationInfo() = default;

    RegistrationInfo(int id, const std::string& date, int pid, int did,
        const std::string& stat, const std::string& note = "")
        : registrationId(id), registrationDate(date), patientId(pid),
        doctorId(did), status(stat), notes(note) {
    }
};

struct BillInfo {
    int billId = 0;
    std::string billDate;
    double amount = 0.0;
    std::string status = "unpaid";

    // 添加默认构造函数
    BillInfo() = default;

    BillInfo(int id, const std::string& date, double amt, const std::string& stat)
        : billId(id), billDate(date), amount(amt), status(stat) {
    }
};
// CommonTypes.h - 在文件末尾添加
struct DepartmentInfo {
    int departmentId = 0;
    std::string departmentName;
    std::string description;
    std::string contactPhone;
    std::string location;

    // 默认构造函数
    DepartmentInfo() = default;

    DepartmentInfo(int id, const std::string& name,
        const std::string& desc = "",
        const std::string& phone = "",
        const std::string& loc = "")
        : departmentId(id), departmentName(name),
        description(desc), contactPhone(phone), location(loc) {
    }
};