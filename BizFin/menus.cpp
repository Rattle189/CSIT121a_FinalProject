/* CSIT 121a - Computer Programming 2 (LAB)
 * MyBiz — All-In-One Business Solution
 * Group 2 Final Output
 * Created by: Christian M. Lañada (0107-1325-24)
 * menus.cpp - This file contains the functions that call upon the TUI
 */
#include <iostream> // Necessary for std::cout, std::endl and the likes
#include <vector>
#include "menus.hpp" // Let's externalize ourself
#include "inventory.hpp"
#include "employees.hpp"
#include "sales.hpp"

void showMainMenu() {
    tui::UI ui;

    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);
    // calculations for dead center on screen/terminal (please put this in a helper)
    int midH = maxy / 5, midW = maxx / 5, midY = (maxy - midH) / 2, midX = (maxx - midW) / 2;

    if (maxx < 100 || maxy < 30) {
        clear();
        mvprintw(0, 0, "Please resize your terminal to at least 100x30.");
        refresh();
        getch();
    }

    auto mainMenuHeader = std::make_shared<tui::Window>(0, 0, 2, maxx, 3);
    mainMenuHeader->add(tui::header("MyBiz - All-In-One Business Solution"));
    mainMenuHeader->setFocusable(false);
    
    std::vector<std::string> availableSystems = { "Inventory Management", "Employee Management", "Sales History", "Exit MyBiz" };

    auto mainMenuList = std::make_shared<tui::Window>(midY, midX, midH, midW, 2);
    mainMenuList->add(tui::leftVerticalMenu(availableSystems, [&](int choice) {
        switch (choice) {
            /* Believe it or not lol but the solution to cleanly deleting windows in the TUI here
             * to make room for a new screen is to just call endwin() then call the screen function
             * and then re-calling the same menu function again. */

            /* DANGER AHEAD!
             * The current set up is potentially dangerous - it is currently safe given the program is
             * still in a small scale and is not expected to run for extremely long times with a lot of
             * user interactions and input. This creates a recursive loop that will grow infinitely within 
             * the stack and potentially cause memory leaks. PLEASE IMPROVE ASAP!
             * —— Christian Lañada (0107-1325-24) */
            case 0:
                endwin();
                showInventoryScreen();
                showMainMenu();
                break;
            case 1:
                endwin();
                showEmployeesScreen();
                showMainMenu();
                break;
            case 2:
                endwin();
                showSalesScreen();
                showMainMenu();
                break;
            case 3: // EXIT
                endwin();
                std::exit(0); 
            case 4: // DEBUG MENU
                endwin();
                showTuiDemos();
                showMainMenu();
                std::exit(0); // return 0 basically
            default:
                std::cout << "\nSomething went wrong! The user picked an invalid selection in the main menu.";
                endwin();
                std::exit(1); // return 1 basically
            }
        }
    ));

    // lazy to build a footer class, so I reused header lol
    auto mainMenuFooter = std::make_shared<tui::Window>(maxy - 1, 0, 1, maxx, 4);
    mainMenuFooter->add(tui::header("Group 2 Final Project"));
    mainMenuFooter->setFocusable(false);

    auto& mainMenuLayout = ui.getLayout();
    mainMenuLayout.addWindow(mainMenuHeader);
    mainMenuLayout.addWindow(mainMenuList);
    mainMenuLayout.addWindow(mainMenuFooter);

    int ch;
    while ((ch = getch())) {

        if (ch == KEY_RESIZE) {
            ui.onResize();
            continue;
        }

        ui.getLayout().handleInput(ch);
        ui.getLayout().draw();
    }
}

void showTuiDemos() {
    tui::UI graphicalMagician;

    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);
    int midH = maxy / 5, midW = maxx / 5, midY = (maxy - midH) / 2, midX = (maxx - midW) / 2;
    std::vector<std::string> availableDemos = {"Demo 1", "Demo 2", "Demo 3", "Demo 4", "Return to Main Menu"};

    auto leHeader = std::make_shared<tui::Window>(0, 0, 3, maxx, 1);
    leHeader->add(tui::header("View Demo Menus"));
    leHeader->setFocusable(false);

    auto leDemos = std::make_shared<tui::Window>(midY, midX, midH, midW, 2);
    leDemos->add(tui::leftVerticalMenu(availableDemos, [&](int demoOfMyChoice) {
        switch (demoOfMyChoice) {
        case 0:
            endwin();
            tuiDemo1();
            showMainMenu();
            break;
        case 1:
            endwin();
            tuiDemo2();
            break;
        case 2:
            endwin();
            tuiDemo3();
            break;
        case 3:
            endwin();
            tuiDemo4();
            break;
        case 4:
            endwin();
            break;
        case 5:
            endwin();
            break;
        default:
            std::cout << "\nSomething went wrong! The user picked an invalid selection in the demo selection menu.";
            endwin();
            std::exit(1); // return 1 basically
        }
        }
    ));

    auto leFootFetish = std::make_shared<tui::Window>(maxy - 1, 0, 1, maxx, 4);
    leFootFetish->add(tui::header("Now viewing TUI screen demo options! TUIs are very cool!"));
    leFootFetish->setFocusable(false);

    auto& mainMenuLayout = graphicalMagician.getLayout();
    mainMenuLayout.addWindow(leHeader);
    mainMenuLayout.addWindow(leDemos);
    mainMenuLayout.addWindow(leFootFetish);

    int woomy;
    while ((woomy = getch()) != 'q') {

        graphicalMagician.getLayout().handleInput(woomy);
        graphicalMagician.getLayout().draw();
    }
}

void tuiDemo1() {
    // Initializes the Terminal/Text User Interface as an object called graphicsAreCool
    tui::UI graphicsAreCool;

    /* This defines the maximum Y and X values for the windows. The reason why Y is first
     * before X is for whatever reason the function is called getmaxyx() and not getmaxxy
     * But then again... getmaxxy seems dumb. */
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    // This is where we define the windows that will be rendered

    /* <tui::Window>(A, B, C, D, E) accepts these parameters: 
     * A: Y-position, controls where it vertically starts on screen
     * B: X-position, controls where it horizontally starts on screen
     * C: Max height of the window
     * D: Max width of the window
     * E: Color palette (see menus.hpp for reference)
     */

    // The header object named imTheHeader
    auto imTheHeader = std::make_shared<tui::Window>(0, 0, 3, maxx, 1);
    // Uses the add() function to add a header into imTheHeader
    imTheHeader->add(tui::header("Demo Menu #1"));
    /* setFocusable() tells whether this window can be focused on or not (via the TAB key)
     * By default the value of this property is true so you need to change it via that function
     * if necessary. */
    imTheHeader->setFocusable(false);

    // The left side list object named someLeftSideList
    auto someLeftSideList = std::make_shared<tui::Window>(3, 0, maxy - 3, maxx / 4, 3);
    // A vector called theItemsInLeftSideList that contains strings to populate someLeftSideList
    std::vector<std::string> theItemsInLeftSideList = {"Banana", "Potato", "Apple"};
    // Uses the add() function to add a leftVerticalMenu into someLeftSideList
    someLeftSideList->add(tui::leftVerticalMenu(theItemsInLeftSideList));

    // This is where we define the layout of the screen
    // First declare the layout using getLayout()
    auto& imTheLayout = graphicsAreCool.getLayout();

    // Add the windows you want using addWindow()
    imTheLayout.addWindow(imTheHeader);
    imTheLayout.addWindow(someLeftSideList);

    // Main loop logic, generally reserved for user input handling, while the function runs
    /* This ain't where the switch() cases or whatever for user input will be used, it should
     * be built into the add() constructor! */
    int ch; // Declare an integer (for character #'s) that will be used to keep track of user keypresses
    while ((ch = getch()) != 'q') { // While the user did not press Q

        /* This is where you would put some sort of check or other behavior
        *  while waiting for user input. The example below is if the user
        *  presses ESC it would terminate the whole screen. */
        if (ch == 27) {
            return;
        }

        // This tells the TUI to handle user input using the ch integer
        graphicsAreCool.getLayout().handleInput(ch);
        // This tells the TUI to draw the stuff defined earlier from the layout above
        graphicsAreCool.getLayout().draw();
    }
}

void tuiDemo2() {

}

void tuiDemo3() {

}

void tuiDemo4() {

}
