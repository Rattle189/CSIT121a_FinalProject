/* CSIT 121a - Computer Programming 2 (LAB)
 * MyBiz — All-In-One Business Solution
 * Group 2 Final Output
 * Created by: Christian M. Lañada (0107-1325-24)
 * employees.cpp - This handles the employee management system.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "menus.hpp"
#include "employees.hpp"

struct EmployeeScreenState {
    int selectedIndex = 0;
};

class EmployeesList : public tui::Element {
    std::vector<Employee>* items;
    EmployeeScreenState* state;

public:
    // This keeps dynamically keeps track of employee data loaded in memory
    EmployeesList(std::vector<Employee>* data, EmployeeScreenState* s)
        : items(data), state(s) {
    }

    void draw(WINDOW* win) override {
        // Header
        mvwprintw(win, 1, 1, "ID   Employee Name                        Hourly Rate     Hours Worked     Expected Pay");
        mvwprintw(win, 2, 1, "---------------------------------------------------------------------------------------");

        for (size_t i = 0; i < items->size(); ++i) {
            auto& item = (*items)[i];

            if ((int)i == state->selectedIndex)
                wattron(win, A_REVERSE);

            if (focused && (int)i == state->selectedIndex)
                wattron(win, A_BOLD);

            // GUIDE: % (formatting start) - (left align) 14 (minimum width) .2 (decimal places) f (floating point)
            mvwprintw(win, i + 3, 1, "%-4d %-36s %-16.2f %-16.2f %-12.2f",
                item.id,
                item.name.c_str(),
                item.hourlyRate,
                item.hoursWorked,
                item.hourlyRate * item.hoursWorked // this calculates the expected pay value on runtime
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

void loadEmployeesData(const std::string& fileName, std::vector<Employee>& employees) {
    std::ifstream file(fileName);
    if (!file.is_open()) return;

    employees.clear();

    Employee item;
    while (file >> item.id) {
        file >> std::ws;

        if (file.peek() == '"') {
            file.get();
            std::getline(file, item.name, '"');
        }
        else {
            file >> item.name;
        }

        file >> item.hourlyRate >> item.hoursWorked;
        employees.push_back(item);
    }
}

void saveEmployeesData(const std::string& fileName, const std::vector<Employee>& employees) {
    std::ofstream file(fileName);

    if (!file.is_open()) {
        std::cout << "Failed to save employee data file.\n";
        return;
    }

    for (const auto& item : employees) {
        file << item.id << " "
            << "\"" << item.name << "\" "
            << item.hourlyRate << " "
            << item.hoursWorked << "\n";
    }
}

void addItem(std::vector<Employee>& employees, const Employee& item) {
    employees.push_back(item);
}

void removeItem(std::vector<Employee>& employees, int index) {
    if (index >= 0 && index < (int)employees.size()) {
        employees.erase(employees.begin() + index);
    }
}

int generateNextId(const std::vector<Employee>& employees) {
    int maxId = 0;
    for (const auto& item : employees) {
        if (item.id > maxId) maxId = item.id;
    }
    return maxId + 1;
}

void showEmployeesScreen() {
    tui::UI ui;

    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    std::vector<Employee> employees;
    loadEmployeesData("employees.txt", employees);

    EmployeeScreenState invState;
    invState.selectedIndex = 0;

    int selectedIndex = -1;

    bool isCreatingNew = false;

    enum Mode {
        LIST_VIEW,
        EDIT_VIEW
    };

    Mode currentMode = LIST_VIEW;

    auto headerWin = std::make_shared<tui::Window>(0, 0, 3, maxx, 1);
    headerWin->add(tui::header("Employee Management"));
    headerWin->setFocusable(false);

    auto list = std::make_shared<EmployeesList>(&employees, &invState);

    auto form = std::make_shared<tui::Form>(
        std::vector<std::string>{"ID", "Employee Name", "Hourly Rate", "Hours Worked"}
    );

    auto loadItemToForm = [&](int index) {
        if (index < 0 || index >= (int)employees.size()) return;

        auto& item = employees[index];
        auto& fields = form->getFields();

        fields[0]->setValue(std::to_string(item.id));
        fields[1]->setValue(item.name);
        fields[2]->setValue(std::to_string(item.hourlyRate));
        fields[3]->setValue(std::to_string(item.hoursWorked));
        };

    auto saveFormToItem = [&](int index, bool isNew) {
        auto& fields = form->getFields();

        Employee item;
        try {
            item.id = std::stoi(fields[0]->getValue());
            item.name = fields[1]->getValue();
            item.hourlyRate = std::stof(fields[2]->getValue());
            item.hoursWorked = std::stof(fields[3]->getValue());
        }
        catch (...) {
            return;
        }

        if (isNew) {
            employees.push_back(item);
        }
        else if (index >= 0 && index < (int)employees.size()) {
            employees[index] = item;
        }

        saveEmployeesData("employees.txt", employees); // uses local file employees.txt
        };

    auto generateNextId = [&](const std::vector<Employee>& emp) {
        int maxId = 0;
        for (auto& i : emp)
            if (i.id > maxId) maxId = i.id;
        return maxId + 1;
        };

    auto rightWin = std::make_shared<tui::Window>(
        3, maxx / 4, maxy - 3, (maxx * 3) / 4, 4
    );

    rightWin->setElements({ list });

    auto menuWin = std::make_shared<tui::Window>(3, 0, maxy - 3, maxx / 4, 3);

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
            "Create New Employee Entry",
            "Edit Selected Entry",
            "Remove Selected Entry"
        },
            [&](int choice) {

                selectedIndex = invState.selectedIndex;

                switch (choice) {

                case 0: { // ADD ITEM
                    currentMode = EDIT_VIEW;
                    isCreatingNew = true;

                    auto& fields = form->getFields();
                    fields[0]->setValue(std::to_string(generateNextId(employees)));
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
                    if (selectedIndex >= 0 && selectedIndex < (int)employees.size()) {
                        employees.erase(employees.begin() + selectedIndex);
                        saveEmployeesData("employees.txt", employees);
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

    auto footerWin = std::make_shared<tui::Window>(maxy - 1, 0, 1, maxx, 4);
    footerWin->add(tui::header("Q | Return to Main Menu"));
    footerWin->setFocusable(false);

    auto& layout = ui.getLayout();
    layout.addWindow(headerWin);
    layout.addWindow(menuWin);
    layout.addWindow(rightWin);
    layout.addWindow(footerWin);

    int ch; // ch short for choice
    while ((ch = getch()) != 'q') {

        ui.getLayout().handleInput(ch);
        ui.getLayout().draw();
    }
}
