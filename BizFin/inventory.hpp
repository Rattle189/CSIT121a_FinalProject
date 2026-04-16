/* CSIT 121a - Computer Programming 2 (LAB)
 * BizFin Tracker & Calculator System
 * Group 2 Final Output
 * Created by: Christian M. Lañada (0107-1325-24)
 * inventory.hpp -
 */
#ifndef INVENTORY_H
#define INVENTORY_H

#include <map>
#include <string>
#include <vector>

struct InventoryItem {
    int id;
    std::string name;
    float price;
    float quantity;
};

// Repository-style API
void loadInventory(const std::string& fileName, std::vector<InventoryItem>& inventory);
void saveInventory(const std::string& fileName, const std::vector<InventoryItem>& inventory);

// CRUD helpers
void addItem(std::vector<InventoryItem>& inventory, const InventoryItem& item);
void removeItem(std::vector<InventoryItem>& inventory, int index);

void showInventoryScreen();

#endif
