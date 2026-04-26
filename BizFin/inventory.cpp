/* CSIT 121a - Computer Programming 2 (LAB)
 * BizFin Tracker & Calculator System
 * Group 2 Final Output
 * Created by: Christian M. Lañada (0107-1325-24)
 * inventory.cpp - This handles the inventory system.
 */

/* inventory.cpp is used as the template for the other screns - sales.cpp and employees.cpp
 * are basically copies of this file (and any other future menus). I haven't figured out a way
 * to completely compartmentalize /everything/ into menus.cpp or something and just have the
 * different files call and make up their own version of a parent. */

/* Also... there's a crap ton of vectors used here and throughout the program. I haven't
 * done any exclusive memory management yet */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "menus.hpp"
#include "inventory.hpp"

// not using namespace std;

/* This basically holds the toggle whether the screen is on
 * VIEW_MODE or EDIT_MODE */
struct InventoryState {
    int selectedIndex = 0;
};

// Inventory List class.
class InventoryList : public tui::Element {
    std::vector<InventoryItem>* items;
    InventoryState* state;

public:
    // This keeps dynamically keeps track of inventory items loaded in memory
    InventoryList(std::vector<InventoryItem>* data, InventoryState* s)
        : items(data), state(s) {
    }

    void draw(WINDOW* win) override {
        // Inventory list header
        mvwprintw(win, 1, 1, "ID   Name                                         Price     Qty");
        mvwprintw(win, 2, 1, "------------------------------------------------------------------");

        for (size_t i = 0; i < items->size(); ++i) {
            auto& item = (*items)[i];

            if ((int)i == state->selectedIndex)
                wattron(win, A_REVERSE);   // persistent selection!!! woohoo

            if (focused && (int)i == state->selectedIndex)
                wattron(win, A_BOLD);      // bolds text so the user doesn't get lost

            mvwprintw(win, i + 3, 1, "%-4d %-44s %-9.2f %-6.2f",
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

    /* This has its own input handler thing because of the state system
     * I should probably get around to integrating that state system into
     * menus.hpp when I have the time so that less reused code */
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
void loadInventory(const std::string& fileName, std::vector<InventoryItem>& inventory) {
    std::ifstream file(fileName);
    if (!file.is_open()) return;

    inventory.clear();

    InventoryItem item;
    while (file >> item.id) {
        file >> std::ws;
        /* std::ws is an input manipulator from <ifstream> that discards leading whitespaces
        * this ensures clean output into the file */

        /* This part handles items in the file with quotation marks for whitespace support */
        if (file.peek() == '"') {
            file.get(); // remove opening quote
            std::getline(file, item.name, '"'); // read until closing quote
        }
        else {
            file >> item.name; // if things go wrong, it just returns a single word
        }

        file >> item.price >> item.quantity;
        inventory.push_back(item);
    }
}

void saveInventory(const std::string& fileName, const std::vector<InventoryItem>& inventory) {
    std::ofstream file(fileName);

    if (!file.is_open()) {
        std::cout << "Failed to save inventory file.\n";
        return;
    }

    /* And this is how items with whitespaces in the name are saved, they are encapsulated
       between quotation marks into the file. */
    for (const auto& item : inventory) {
        file << item.id << " "
            << "\"" << item.name << "\" "
            << item.price << " "
            << item.quantity << "\n";
    }
}

// CRUD (Create, Read, Update, Delete) helpers
void addItem(std::vector<InventoryItem>& employees, const InventoryItem& item) {
    employees.push_back(item);
}

void removeItem(std::vector<InventoryItem>& employees, int index) {
    if (index >= 0 && index < (int)employees.size()) {
        employees.erase(employees.begin() + index);
    }
}

// This handy little function just automatically generates the next item ID, just like how a database does it
int generateNextId(const std::vector<InventoryItem>& inventory) {
    int maxId = 0; // Pre-initializes the value to 0 in case thing go horribly wrong
    for (const auto& item : inventory) {
        if (item.id > maxId) maxId = item.id;
    }
    return maxId + 1;
}

void showInventoryScreen() {
    // Initializes the Terminal/Text User Interface
    tui::UI ui;

    // Defines the maximum y and x for the TUI's windows
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    // Loads data or whatever
    std::vector<InventoryItem> inventory;
    loadInventory("inventory.txt", inventory);

    // Inventory State handling
    InventoryState invState;
    invState.selectedIndex = 0;

    // I honestly forgot what this is for lol. I think this is for the menu selections
    int selectedIndex = -1;

    // This boolean flag is to make sure the ADD and EDIT features don't mix each other up
    bool isCreatingNew = false;

    // Modes on what state the inventory screen is in
    enum Mode {
        LIST_VIEW,
        EDIT_VIEW
    };

    // Default mode is LIST_VIEW
    Mode currentMode = LIST_VIEW;

    // ===== THIS IS THE PART WHERE IT DRAWS SHIT ON THE SCREEN! =====
    // menus.hpp IS SUPPOSED TO BE MODULAR, LIKE BOOTSTRAP OR WHATEVER
    // SO IT'S BASICALLY LIKE... RANDOM BULLSHIT, GO! AND RENDER STUFF

    // HEADER
    auto headerWin = std::make_shared<tui::Window>(0, 0, 3, maxx, 1);
    headerWin->add(tui::header("Inventory Management"));
    headerWin->setFocusable(false);

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

        InventoryItem item;
        try {
            item.id = std::stoi(fields[0]->getValue());
            item.name = fields[1]->getValue();
            item.price = std::stof(fields[2]->getValue());
            item.quantity = std::stof(fields[3]->getValue());
        }
        catch (...) {
            std::cout << "\nSomething went wrong saving the inventory data.";
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

    auto generateNextId = [&](const std::vector<InventoryItem>& inv) {
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

    /* So this is out of our lesson plan, but these particular two sections required the use
     * of a lambda (er, anonymous functions) for the changing of the left-side menu */
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
            "Remove Selected Item"
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
    menuWin->setElements({ buildListMenu() });

    // FOOTER
    auto footerWin = std::make_shared<tui::Window>(maxy - 1, 0, 1, maxx, 4);
    footerWin->add(tui::header("Q | Return to Main Menu"));
    footerWin->setFocusable(false);

    // LAYOUT
    auto& layout = ui.getLayout();
    layout.addWindow(headerWin);
    layout.addWindow(menuWin);
    layout.addWindow(rightWin);
    layout.addWindow(footerWin);

    // MAIN LOOP
    int ch; // ch short for choice
    while ((ch = getch()) != 'q') {
        if (currentMode == EDIT_VIEW && ch == 27) {
            // saveFormToItem(selectedIndex, selectedIndex < 0);

            currentMode = LIST_VIEW;

            rightWin->setElements({ list });

            menuWin->setElements({ buildEditMenu() });
        }

        ui.getLayout().handleInput(ch);
        ui.getLayout().draw();
    }
}
