/* CSIT 121a - Computer Programming 2 (LAB)
 * MyBiz — All-In-One Business Solution
 * Group 2 Final Output
 * Created by: Christian M. Lañada (0107-1325-24)
 * sales.cpp - This file contains the functions that call upon the TUI
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime> // for UNIX time support to turn into timestamps later
#include <cassert> // required for assert
#include "menus.hpp"
#include "sales.hpp"

std::time_t now = std::time(nullptr); // pre-initialize the value of time

/* Because Microsoft considers the localtime variable/function to be unsafe and recommends using localtime_s
 * otherwise the MSVC compiler will complain, this helper function fixes that issue so that the program will
 * compile just fine both on Windows and Linux systems. */
void safeLocalTime(std::tm& out, const std::time_t& t) {
    #ifdef _WIN32
        localtime_s(&out, &t);   // for pesky Windows
    #else
        localtime_r(&t, &out);   // Linux or POSIX
    #endif
}

void loadInventoryMap(const std::string& file, std::unordered_map<int, std::string>& map, std::unordered_map<int, float>& prices) {
    std::ifstream in(file);
    if (!in.is_open()) return;

    int id;
    std::string name;
    float price;
    float qty;

    while (in >> id) {
        in >> std::ws;

        if (in.peek() == '"') {
            in.get();
            std::getline(in, name, '"');
        }
        else {
            in >> name;
        }

        in >> price >> qty;

        map[id] = name;
        prices[id] = price;
    }
}

void loadEmployeeMap(const std::string& file, std::unordered_map<int, std::string>& map) {
    std::ifstream in(file);
    if (!in.is_open()) return;

    int id;
    std::string name;
    float hourlyRate;
    int hoursWorked;

    while (in >> id) {
        in >> std::ws;

        if (in.peek() == '"') {
            in.get();
            std::getline(in, name, '"');
        }
        else {
            in >> name;
        }

        in >> hourlyRate >> hoursWorked;

        map[id] = name;
    }
}

struct SalesScreenState {
    int selectedIndex = 0;
};

// LIST ELEMENT
class SalesList : public tui::Element {
    std::vector<Sale>* items;
    SalesData* ctx; // short for (sales) context
    SalesScreenState* state;

public:
    SalesList(std::vector<Sale>* data, SalesScreenState* s, SalesData* c)
        : items(data), state(s), ctx(c) {
        assert(items && state && ctx);
    }

    void draw(WINDOW* win) override {        
        mvwprintw(win, 1, 1,
            "ID   Item Name                By                  Qty         Total     Date");

        mvwprintw(win, 2, 1,
            "---------------------------------------------------------------------------------------");

        for (size_t i = 0; i < items->size(); ++i) {
            auto& sale = (*items)[i];

            if ((int)i == state->selectedIndex)
                wattron(win, A_REVERSE);

            if (focused && (int)i == state->selectedIndex)
                wattron(win, A_BOLD);

            // Converts timestamp to readable date
            char dateBuf[64];
            std::tm tm;
            safeLocalTime(tm, sale.timestamp);
            std::strftime(dateBuf, sizeof(dateBuf),
                "%Y-%m-%d %H:%M", &tm);

            std::string itemName =
                ctx->items.count(sale.itemId)
                ? ctx->items[sale.itemId]
                : "UNKNOWN ITEM";

            std::string employeeName =
                ctx->employees.count(sale.employeeId)
                ? ctx->employees[sale.employeeId]
                : "UNKNOWN EMPLOYEE";

            float price = ctx->itemPrices.count(sale.itemId)
                ? ctx->itemPrices[sale.itemId]
                : 0.0f;

            float computedTotal = price * sale.quantity;

            mvwprintw(win, i + 3, 1,
                "%-4d %-24s %-18s %-10.2f %-10.2f %-10s",
                sale.id,
                itemName.c_str(),
                employeeName.c_str(),
                sale.quantity,
                computedTotal,
                dateBuf
            );

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

// FILE I/O
void loadSalesData(const std::string& fileName, std::vector<Sale>& sales) {
    std::ifstream file(fileName);
    if (!file.is_open()) return;

    sales.clear();

    Sale s;
    while (file >> s.id >> s.itemId >> s.employeeId
        >> s.quantity >> s.timestamp) {
        sales.push_back(s);
    }
}

void loadSalesContext(SalesData& ctx) {
    loadInventoryMap("inventory.txt", ctx.items, ctx.itemPrices);
    loadEmployeeMap("employees.txt", ctx.employees);
}

void saveSalesData(const std::string& fileName, const std::vector<Sale>& sales) {
    std::ofstream file(fileName);

    for (const auto& s : sales) {
        file << s.id << " "
            << s.itemId << " "
            << s.employeeId << " "
            << s.quantity << " "
            << s.timestamp << "\n";
    }
}

// HELPERS
int generateNextId(const std::vector<Sale>& sales) {
    int maxId = 0;
    for (const auto& s : sales)
        if (s.id > maxId) maxId = s.id;
    return maxId + 1;
}

// MAIN SCREEN
void showSalesScreen() {
    tui::UI ui;

    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    // Load data into memory
    std::vector<Sale> sales;
    SalesData ctx; // ctx is short for (sales) context
    loadSalesData("sales.txt", sales);
    loadSalesContext(ctx);

    SalesScreenState state;
    state.selectedIndex = 0;

    int selectedIndex = -1;
    bool isCreatingNew = false;

    enum Mode { LIST_VIEW, EDIT_VIEW };
    Mode currentMode = LIST_VIEW;

    // HEADER
    auto headerWin = std::make_shared<tui::Window>(0, 0, 3, maxx, 1);
    headerWin->add(tui::header("Sales Management"));
    headerWin->setFocusable(false);

    // RIGHT PANEL
    auto list = std::make_shared<SalesList>(&sales, &state, &ctx);

    auto form = std::make_shared<tui::Form>(
        std::vector<std::string>{
        "Sale ID", "Item ID", "Employee ID", "Quantity"
    }
    );

    auto rightWin = std::make_shared<tui::Window>(
        3, maxx / 4, maxy - 4, (maxx * 3) / 4, 4
    );

    rightWin->setElements({ list });

    // FORM HELPERS
    auto loadToForm = [&](int index) {
        if (index < 0 || index >= (int)sales.size()) return;

        auto& s = sales[index];
        auto& f = form->getFields();

        f[0]->setValue(std::to_string(s.id));
        f[1]->setValue(std::to_string(s.itemId));
        f[2]->setValue(std::to_string(s.employeeId));
        f[3]->setValue(std::to_string(s.quantity));
        };

    auto saveForm = [&](int index, bool isNew) {
        auto& f = form->getFields();

        Sale s;
        try {
            s.id = std::stoi(f[0]->getValue());
            s.itemId = std::stoi(f[1]->getValue());
            s.employeeId = std::stoi(f[2]->getValue());
            s.quantity = std::stof(f[3]->getValue());
            s.timestamp = std::time(nullptr); // auto timestamp
        }
        catch (...) {
            std::cout << "\nSomething went wrong in attempting to load sales data.";
            return;
        }

        if (isNew) {
            sales.push_back(s);
        }
        else if (index >= 0 && index < (int)sales.size()) {
            sales[index] = s;
        }

        saveSalesData("sales.txt", sales);
        };

    // LEFT MENU
    auto menuWin = std::make_shared<tui::Window>(
        3, 0, maxy - 4, maxx / 4, 3
    );

    std::function<std::shared_ptr<tui::VerticalMenu>()> buildListMenu;
    std::function<std::shared_ptr<tui::VerticalMenu>()> buildEditMenu;

    buildEditMenu = [&]() {
        return std::make_shared<tui::VerticalMenu>(
            std::vector<std::string>{
            "Save Sale",
            "Cancel"
        },
            [&](int choice) {
                if (choice == 0) {
                    saveForm(selectedIndex, isCreatingNew);
                }

                currentMode = LIST_VIEW;
                rightWin->setElements({ list });
                menuWin->setElements({ buildListMenu() });
            }
        );
        };

    buildListMenu = [&]() {
        return std::make_shared<tui::VerticalMenu>(
            std::vector<std::string>{
            "Create New Sale",
            "Edit Selected Sale",
            "Delete Selected Sale"
        },
            [&](int choice) {

                selectedIndex = state.selectedIndex;

                switch (choice) {

                case 0: { // CREATE
                    currentMode = EDIT_VIEW;
                    isCreatingNew = true;

                    auto& f = form->getFields();
                    f[0]->setValue(std::to_string(generateNextId(sales)));
                    f[1]->setValue("");
                    f[2]->setValue("");
                    f[3]->setValue("1");

                    rightWin->setElements({ form });
                    menuWin->setElements({ buildEditMenu() });
                    break;
                }

                case 1: { // EDIT
                    if (selectedIndex < 0) break;

                    currentMode = EDIT_VIEW;
                    isCreatingNew = false;
                    loadToForm(selectedIndex);

                    rightWin->setElements({ form });
                    menuWin->setElements({ buildEditMenu() });
                    break;
                }

                case 2: { // DELETE
                    if (selectedIndex >= 0 &&
                        selectedIndex < (int)sales.size()) {
                        sales.erase(sales.begin() + selectedIndex);
                        saveSalesData("sales.txt", sales);
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
    int ch;
    while ((ch = getch()) != 'q') {
        layout.handleInput(ch);
        layout.draw();
    }
}
