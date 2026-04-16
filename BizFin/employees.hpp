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
    float price;
    float quantity;
};

// Repository-style API
void loadInventory(const std::string& fileName, std::vector<Employee>& inventory);
void saveInventory(const std::string& fileName, const std::vector<Employee>& inventory);

// CRUD helpers
void addItem(std::vector<Employee>& inventory, const Employee& item);
void removeItem(std::vector<Employee>& inventory, int index);

void showEmployeesScreen();

#endif
