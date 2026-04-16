/* CSIT 121a - Computer Programming 2 (LAB)
 * BizFin Tracker & Calculator System
 * Group 2 Final Output
 * Created by: Christian M. Lañada (0107-1325-24)
 * employees.cpp - This handles the employee management system.
 */
#include <iostream>
#include <fstream> // for file I/O operations
#include <string>
#include <vector> // for dynamically allocated arrays, used a crap ton here
#include "menus.hpp"
#include "employees.hpp"

 // not using namespace std;

struct InventoryState {
    int selectedIndex = 0;
};

class InventoryList : public tui::Element {
    std::vector<Employee>* items;
    InventoryState* state;

public:
    // This keeps dynamically keeps track of inventory items loaded in memory
    InventoryList(std::vector<Employee>* data, InventoryState* s)
        : items(data), state(s) {
    }

    void draw(WINDOW* win) override {
        // Header
        mvwprintw(win, 1, 1, "ID   Name        Price     Qty");
        mvwprintw(win, 2, 1, "--------------------------------");

        for (size_t i = 0; i < items->size(); ++i) {
            auto& item = (*items)[i];

            if ((int)i == state->selectedIndex)
                wattron(win, A_REVERSE);   // persistent selection!!! woohoo

            if (focused && (int)i == state->selectedIndex)
                wattron(win, A_BOLD);      // bolds text so the user doesn't get lost

            mvwprintw(win, i + 3, 1, "%-4d %-10s %-9.2f %-6.2f",
                item.id,
                item.name.c_str(),
                item.price,
                item.quantity);

            if (focused && (int)i == state->selectedIndex)
                wattroff(win, A_BOLD);

            if ((int)i == state->selectedIndex)
                wattroff(win, A_REVERSE);
        }
    }

    void handleInput(int ch) override {
        if (!focused) return;

        switch (ch) {
        case KEY_UP:
            if (state->selectedIndex > 0)
                state->selectedIndex--;
            break;

        case KEY_DOWN:
            if (state->selectedIndex < (int)items->size() - 1)
                state->selectedIndex++;
            break;
        }
    }
};

/* Load & save functions
 * Basically I/O operations */
void loadInventory(const std::string& fileName, std::vector<Employee>& inventory) {
    std::ifstream file(fileName);

    if (!file.is_open()) {
        std::cout << "Failed to open inventory file.\n";
        return;
    }

    inventory.clear();

    Employee item;
    while (file >> item.id >> item.name >> item.price >> item.quantity) {
        inventory.push_back(item);
    }

    file.close();
}

void saveInventory(const std::string& fileName, const std::vector<Employee>& inventory) {
    std::ofstream file(fileName);

    if (!file.is_open()) {
        std::cout << "Failed to save inventory file.\n";
        return;
    }

    for (const auto& item : inventory) {
        file << item.id << " "
            << item.name << " "
            << item.price << " "
            << item.quantity << "\n";
    }

    file.close();
}

// CRUD (Create, Read, Update, Delete) helpers
void addItem(std::vector<Employee>& inventory, const Employee& item) {
    inventory.push_back(item);
}

void removeItem(std::vector<Employee>& inventory, int index) {
    if (index >= 0 && index < (int)inventory.size()) {
        inventory.erase(inventory.begin() + index);
    }
}

// This handy little function just automatically generates the next item ID, just like how a database does it
int generateNextId(const std::vector<Employee>& inventory) {
    int maxId = 0; // Pre-initializes the value to 0 in case thing go horribly wrong
    for (const auto& item : inventory) {
        if (item.id > maxId) maxId = item.id;
    }
    return maxId + 1;
}

void showEmployeesScreen() {
    // Initializes the Terminal/Text User Interface
    tui::UI ui;

    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    // Loads data or whatever
    std::vector<Employee> inventory;
    loadInventory("inventory.txt", inventory);

    // Inventory State handling
    InventoryState invState;
    invState.selectedIndex = 0;

    int selectedIndex = -1;

    // This boolean flag is to make sure the ADD and EDIT features don't mix each other up
    bool isCreatingNew = false;

    // Modes on what state the inventory screen is in
    enum Mode {
        LIST_VIEW,
        EDIT_VIEW
    };

    Mode currentMode = LIST_VIEW;

    // HEADER
    auto headerWin = std::make_shared<tui::Window>(0, 0, 3, maxx, 1);
    headerWin->add(tui::header("Inventory Management"));

    // RIGHT PANEL CONTENT
    auto list = std::make_shared<InventoryList>(&inventory, &invState);

    auto form = std::make_shared<tui::Form>(
        std::vector<std::string>{"ID", "Name", "Price", "Quantity"}
    );

    // FORM HELPERS, SO THE FORM HAS STUFF!
    auto loadItemToForm = [&](int index) {
        if (index < 0 || index >= (int)inventory.size()) return;

        auto& item = inventory[index];
        auto& fields = form->getFields();

        fields[0]->setValue(std::to_string(item.id));
        fields[1]->setValue(item.name);
        fields[2]->setValue(std::to_string(item.price));
        fields[3]->setValue(std::to_string(item.quantity));
        };

    // This handles saving
    auto saveFormToItem = [&](int index, bool isNew) {
        auto& fields = form->getFields();

        Employee item;
        try {
            item.id = std::stoi(fields[0]->getValue());
            item.name = fields[1]->getValue();
            item.price = std::stof(fields[2]->getValue());
            item.quantity = std::stof(fields[3]->getValue());
        }
        catch (...) {
            // basic safety fallback
            return;
        }

        if (isNew) {
            inventory.push_back(item);
        }
        else if (index >= 0 && index < (int)inventory.size()) {
            inventory[index] = item;
        }

        saveInventory("inventory.txt", inventory); // uses local file inventory.txt
        };

    auto generateNextId = [&](const std::vector<Employee>& inv) {
        int maxId = 0;
        for (auto& i : inv)
            if (i.id > maxId) maxId = i.id;
        return maxId + 1;
        };

    // RIGHT PANEL WINDOW
    auto rightWin = std::make_shared<tui::Window>(
        3, maxx / 4, maxy - 3, (maxx * 3) / 4, 4
    );

    rightWin->setElements({ list }); // this used to be ->add(list) but was changed in favor of the new setElements({})

    // LEFT MENU WINDOW
    auto menuWin = std::make_shared<tui::Window>(3, 0, maxy - 3, maxx / 4, 3);

    /* std::function is used here for forward declarations
     * the two functions below are lambdas that require one another and to
     * make sure they see each other, this is the way */
    std::function<std::shared_ptr<tui::VerticalMenu>()> buildListMenu;
    std::function<std::shared_ptr<tui::VerticalMenu>()> buildEditMenu;

    buildEditMenu = [&]() {
        return std::make_shared<tui::VerticalMenu>(
            std::vector<std::string>{
            "Save Changes",
                "Discard & Exit"
        },
            [&](int choice) {

                switch (choice) {

                case 0: { // SAVE CHANGES
                    saveFormToItem(selectedIndex, isCreatingNew);

                    currentMode = LIST_VIEW;

                    rightWin->setElements({ list });

                    menuWin->setElements({ buildListMenu() });
                    break;
                }

                case 1: { // DISCARD CHANGES
                    currentMode = LIST_VIEW;

                    rightWin->setElements({ list });

                    menuWin->setElements({ buildListMenu() });
                    break;
                }
                }
            }
        );
        };

    buildListMenu = [&]() {
        return std::make_shared<tui::VerticalMenu>(
            std::vector<std::string>{
            "Add New Item",
                "Edit Selected Item",
                "Remove Selected Item",
                "Return to Main Menu"
        },
            [&](int choice) {

                selectedIndex = invState.selectedIndex;

                switch (choice) {

                case 0: { // ADD ITEM
                    currentMode = EDIT_VIEW;
                    isCreatingNew = true;

                    auto& fields = form->getFields();
                    fields[0]->setValue(std::to_string(generateNextId(inventory)));
                    fields[1]->setValue("");
                    fields[2]->setValue("0");
                    fields[3]->setValue("0");

                    // switch right panel
                    rightWin->setElements({ form });

                    // switch menu
                    menuWin->setElements({ buildEditMenu() });
                    break;
                }

                case 1: { // EDIT ITEM
                    if (selectedIndex < 0) break;

                    currentMode = EDIT_VIEW;
                    isCreatingNew = false;
                    loadItemToForm(selectedIndex);

                    rightWin->setElements({ form });

                    menuWin->setElements({ buildEditMenu() });
                    break;
                }

                case 2: { // DELETE
                    if (selectedIndex >= 0 && selectedIndex < (int)inventory.size()) {
                        inventory.erase(inventory.begin() + selectedIndex);
                        saveInventory("inventory.txt", inventory);
                    }
                    break;
                }

                case 3:
                    endwin();
                    return;
                }
            }
        );
        };

    // Mandatory initialization of menuWin's contents
    // menuWin->add(buildListMenu()); <--- Deprecated approach
    menuWin->setElements({ buildListMenu() });

    // LAYOUT
    auto& layout = ui.getLayout();
    layout.addWindow(headerWin);
    layout.addWindow(menuWin);
    layout.addWindow(rightWin);

    // MAIN LOOP
    int ch; // ch short for choice
    while ((ch = getch()) != 'q') {

        /* ===== Getting rid of this because this can cause unwanted behavior! =====
        // ESC exits EDIT mode and saves
        if (currentMode == EDIT_VIEW && ch == 27) {
            saveFormToItem(selectedIndex, selectedIndex < 0);

            currentMode = LIST_VIEW;

            rightWin->setElements({ list });

            menuWin->setElements({ buildEditMenu() });
        } */

        ui.getLayout().handleInput(ch);
        ui.getLayout().draw();
    }
}
