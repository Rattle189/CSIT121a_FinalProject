/* CSIT 121a - Computer Programming 2 (LAB)
 * BizFin Tracker & Calculator System
 * Group 2 Final Output
 * Created by: Christian M. Lañada (0107-1325-24)
 * sales.hpp - This file contains the functions that call upon the TUI
 */
#ifndef SALES_H
#define SALES_H

#include <ctime> // for UNIX time support

struct Sale {
	int id;
	int itemId;
	int employeeId;
	float quantity;
	float total;
	std::time_t timestamp;
};

/* This holds data that will be used by the sales list to retrieve and 
 * store from inventory.txt and employees.txt */
struct SalesData {
	std::unordered_map<int, std::string> items;
	std::unordered_map<int, std::string> employees;
	std::unordered_map<int, float> itemPrices;
};

void showSalesScreen();
void safeLocalTime(std::tm& out, const std::time_t& t);

#endif