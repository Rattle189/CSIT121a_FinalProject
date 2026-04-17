/* CSIT 121a - Computer Programming 2 (LAB)
 * BizFin Tracker & Calculator System
 * Group 2 Final Output
 * Created by: Christian M. Lañada (0107-1325-24)
 * employees.hpp -
 */
#ifndef EMPLOYEES
#define EMPLOYEES_H

#include <string>
#include <vector>

struct Employee {
    int id;
    std::string name;
    float hourlyRate;
    float hoursWorked;
};

// Repository-style API
void loadEmployeesData(const std::string& fileName, std::vector<Employee>& employees);
void saveEmployeesData(const std::string& fileName, const std::vector<Employee>& employees);

// CRUD helpers
void addItem(std::vector<Employee>& employees, const Employee& item);
void removeItem(std::vector<Employee>& employees, int index);

void showEmployeesScreen();

#endif
