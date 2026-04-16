/* CSIT 121a - Computer Programming 2 (LAB)
 * BizFin Tracker & Calculator System
 * Group 2 Final Output
 * Created by: Christian M. Lañada (0107-1325-24)
 * menus.hpp - This file contains the TUI (terminal/text user interface)
 */
#pragma once

#ifndef MENUS_H
#define MENUS_H

#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
#endif

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <cctype>

namespace tui {

    class Element;
    class Window;

    using ElementPtr = std::shared_ptr<Element>;
    using WindowPtr = std::shared_ptr<Window>;

    // Base Element class. This basically holds those classes below.
    class Element {
    protected:
        bool focused = false;
    public:
        virtual ~Element() = default;

        virtual void draw(WINDOW* win) = 0;
        virtual void handleInput(int ch) {}

        void setFocus(bool f) { focused = f; }
        bool isFocused() const { return focused; }
    };

    // Header class.
    class Header : public Element {
        std::string text;
    public:
        Header(const std::string& t) : text(t) {}

        void draw(WINDOW* win) override {
            int maxy, maxx;
            getmaxyx(win, maxy, maxx);
            wattron(win, A_REVERSE);
            mvwprintw(win, 0, (maxx - text.size()) / 2, "%s", text.c_str());
            wattroff(win, A_REVERSE);
        }
    };

    // Vertical Menu class.
    class VerticalMenu : public Element {
        std::vector<std::string> items;
        int selected = 0;
        std::function<void(int)> onSelect;
    public:
        VerticalMenu(const std::vector<std::string>& i,
            std::function<void(int)> cb = nullptr)
            : items(i), onSelect(cb) {
        }

        void draw(WINDOW* win) override {
            for (size_t i = 0; i < items.size(); ++i) {
                if ((int)i == selected && focused) wattron(win, A_STANDOUT);
                mvwprintw(win, i + 1, 1, "%s", items[i].c_str());
                if ((int)i == selected && focused) wattroff(win, A_STANDOUT);
            }
        }

        void handleInput(int ch) override {
            switch (ch) {
            case KEY_UP: if (selected > 0) selected--; break;
            case KEY_DOWN: if (selected < (int)items.size() - 1) selected++; break;
            case '\n': if (onSelect) onSelect(selected); break;
            }
        }
    };

    // Field class. We don't have WinForms or a proper GUI so this is the way to go for data entry.
    class Field : public Element {
        std::string label;
        std::string value;
        int cursor = 0;
        bool editing = false;

    public:
        Field(const std::string& l) : label(l) {}

        const std::string& getValue() const { return value; }
        void setValue(const std::string& v) { value = v; cursor = v.size(); }

        void draw(WINDOW* win) override {
            int y, x;
            getyx(win, y, x);

            if (focused) wattron(win, A_BOLD);

            mvwprintw(win, y, 1, "%s: %s", label.c_str(), value.c_str());

            if (focused) {
                wattroff(win, A_BOLD);
                if (editing) {
                    wmove(win, y, label.size() + 3 + cursor);
                    curs_set(1);
                }
            }
        }

        // Input handling.
        void handleInput(int ch) override {
            if (!focused) return;

            if (!editing) {
                if (ch == '\n') {
                    editing = true;
                    cursor = value.size();
                }
                return;
            }

            switch (ch) {
            case 27: // ESC
                editing = false;
                curs_set(0);
                break;

            case KEY_LEFT:
                if (cursor > 0) cursor--;
                break;

            case KEY_RIGHT:
                if (cursor < (int)value.size()) cursor++;
                break;
            
            case 8: // backspace support for Windows
            case KEY_BACKSPACE:
            case 127:
                if (cursor > 0) {
                    value.erase(cursor - 1, 1);
                    cursor--;
                }
                break;

            case '\n':
                editing = false;
                curs_set(0);
                break;

            default:
                if (isprint(ch)) {
                    value.insert(cursor, 1, (char)ch);
                    cursor++;
                }
            }
        }
    };

    // Form Container class.
    class Form : public Element {
        std::vector<std::shared_ptr<Field>> fields;
        int active = 0;

    public:
        Form(const std::vector<std::string>& labels) {
            for (auto& l : labels) {
                fields.push_back(std::make_shared<Field>(l));
            }
        }

        std::vector<std::shared_ptr<Field>>& getFields() { return fields; }

        void draw(WINDOW* win) override {
            int y = 1;
            for (size_t i = 0; i < fields.size(); ++i) {
                fields[i]->setFocus(focused && (int)i == active);
                wmove(win, y++, 1);
                fields[i]->draw(win);
            }
        }

        void handleInput(int ch) override {
            if (!focused) return;

            if (ch == '\t') {
                active = (active + 1) % fields.size();
                return;
            }

            if (ch == KEY_UP && active > 0) active--;
            else if (ch == KEY_DOWN && active < (int)fields.size() - 1) active++;
            else fields[active]->handleInput(ch);
        }
    };

    // Window system class.
    class Window {
        WINDOW* win;
        int x, y, w, h;
        int colorPair = 0;
        std::vector<ElementPtr> elements;
        int focusedElement = 0;
        bool focusable = true;

    public:
        Window(int _y, int _x, int _h, int _w, int color = 0)
            : x(_x), y(_y), w(_w), h(_h), colorPair(color) {
            win = newwin(h, w, y, x);
        }

        void setFocusable(bool f) { focusable = f; }
        bool isFocusable() const { return focusable; }

        ~Window() { delwin(win); }

        void setColor(int pair) { colorPair = pair; }

        void add(ElementPtr el) {
            elements.push_back(el);
        }

        void draw(bool focusedWin) {
            werase(win);

            if (colorPair > 0 && has_colors()) {
                /* This fills the entire window's background with color.
                 * Not yet tested on Windows or PDcurses, if it misbehaves
                 * then this line needs to go. */
                wbkgd(win, COLOR_PAIR(colorPair));

                wattron(win, COLOR_PAIR(colorPair));
            }

            box(win, 0, 0);

            for (size_t i = 0; i < elements.size(); ++i) {
                elements[i]->setFocus(focusedWin && (int)i == focusedElement);
                elements[i]->draw(win);
            }

            if (colorPair > 0 && has_colors()) {
                wattroff(win, COLOR_PAIR(colorPair));
            }

            wrefresh(win);
        }

        // This is supposed to be a drop-in replacement to clear() because it was too destructive with the new
        // mode system introduced.
        void setElements(const std::vector<ElementPtr>& elems) {
            elements = elems;
        }

        void clear() {
            elements.clear();
            focusedElement = 0;
        }

        void handleInput(int ch) {
            if (elements.empty()) return;

            if (ch == '	') {
                focusedElement = (focusedElement + 1) % elements.size();
                return;
            }

            elements[focusedElement]->handleInput(ch);
        }
    };

    // Layout system.
    class Layout {
        std::vector<WindowPtr> windows;
        int focusedIndex = 0;

    public:
        void addWindow(WindowPtr w) {
            windows.push_back(w);
        }

        void nextFocus() {
            if (windows.empty()) return;

            int start = focusedIndex;

            do {
                focusedIndex = (focusedIndex + 1) % windows.size();
            } while (!windows[focusedIndex]->isFocusable() && focusedIndex != start);
        }

        void draw() {
            for (size_t i = 0; i < windows.size(); ++i) {
                windows[i]->draw(i == focusedIndex);
            }
        }

        void clear() {
            windows.clear();
            focusedIndex = 0;
        }

        void handleInput(int ch) {
            if (ch == '\t') { // global next window (like old POS)
                nextFocus();
                return;
            }
            if (windows[focusedIndex]->isFocusable())
                windows[focusedIndex]->handleInput(ch);
        }
    };

    // UI
    class UI {
        Layout layout;

        // Color system
        void initColors() {
            if (has_colors()) {
                start_color();
                use_default_colors();

                // Color palettes
                init_pair(1, COLOR_BLACK, COLOR_CYAN);    // header
                init_pair(2, COLOR_BLACK, COLOR_WHITE);   // footer
                init_pair(3, COLOR_WHITE, COLOR_BLUE);    // left panel
                init_pair(4, COLOR_WHITE, COLOR_BLACK);   // main
                init_pair(5, COLOR_WHITE, COLOR_MAGENTA); // right panel
            }
        }

    public:
        // UI initiation logic
        UI() {
            initscr();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
            curs_set(0);
            initColors();
        }

        ~UI() { endwin(); }

        Layout& getLayout() { return layout; }

        void loop() {
            int ch;
            while ((ch = getch()) != 'q') {
                layout.handleInput(ch);
                layout.draw();
            }
        }
    };

    // Builders
    inline ElementPtr header(const std::string& text) {
        return std::make_shared<Header>(text);
    }

    inline ElementPtr leftVerticalMenu(const std::vector<std::string>& items,
        std::function<void(int)> cb = nullptr) {
        return std::make_shared<VerticalMenu>(items, cb);
    }

    inline ElementPtr form(const std::vector<std::string>& fields) {
        return std::make_shared<Form>(fields);
    }

} // namespace tui

// Make other screens (dunno why I call them menus in the code) available
void showMainMenu();

// Some test or example screens on how to use the TUI for yourself!
void showTuiDemos();
void tuiDemo1();
void tuiDemo2();
void tuiDemo3();
void tuiDemo4();

#endif
