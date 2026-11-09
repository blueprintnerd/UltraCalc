#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "math.hpp"
#include <string>

using namespace ftxui;

int main() {
    auto screen = App::Fullscreen();
    
    // ===== STATE =====
    std::string display = "0";
    float stored_value = 0;
    std::string operation = "";
    bool new_input = true;
    
    // ===== BUTTONS =====
    auto button = [](const std::string& label, auto on_click) {
        auto btn = Button(label, on_click);
        return btn;
    };
    
    // Number buttons
    auto btn0 = button("0", [&] {
        if (new_input) { display = "0"; new_input = false; }
        display += "0";
    });
    
    auto btn1 = button("1", [&] {
        if (new_input) { display = ""; new_input = false; }
        display += "1";
    });
    auto btn2 = button("2", [&] {
        if (new_input) { display = ""; new_input = false; }
        display += "2";
    });
    auto btn3 = button("3", [&] {
        if (new_input) { display = ""; new_input = false; }
        display += "3";
    });
    auto btn4 = button("4", [&] {
        if (new_input) { display = ""; new_input = false; }
        display += "4";
    });
    auto btn5 = button("5", [&] {
        if (new_input) { display = ""; new_input = false; }
        display += "5";
    });
    auto btn6 = button("6", [&] {
        if (new_input) { display = ""; new_input = false; }
        display += "6";
    });
    auto btn7 = button("7", [&] {
        if (new_input) { display = ""; new_input = false; }
        display += "7";
    });
    auto btn8 = button("8", [&] {
        if (new_input) { display = ""; new_input = false; }
        display += "8";
    });
    auto btn9 = button("9", [&] {
        if (new_input) { display = ""; new_input = false; }
        display += "9";
    });
    
    // Operation buttons
    auto btn_add = button("+", [&] {
        stored_value = std::stof(display);
        operation = "add";
        new_input = true;
    });
    
    auto btn_sub = button("-", [&] {
        stored_value = std::stof(display);
        operation = "sub";
        new_input = true;
    });
    
    auto btn_mul = button("*", [&] {
        stored_value = std::stof(display);
        operation = "mul";
        new_input = true;
    });
    
    auto btn_div = button("/", [&] {
        stored_value = std::stof(display);
        operation = "div";
        new_input = true;
    });
    
    auto btn_equals = button("=", [&] {
        float current = std::stof(display);
        float result = 0;
        
        if (operation == "add") result = add(stored_value, current);
        else if (operation == "sub") result = subtract(stored_value, current);
        else if (operation == "mul") result = multiplier(stored_value, current);
        else if (operation == "div") result = divider(stored_value, current, 0);
        
        display = std::to_string(result);
        // Remove trailing zeros
        display.erase(display.find_last_not_of('0') + 1, std::string::npos);
        if (display.back() == '.') display.pop_back();
        
        operation = "";
        new_input = true;
    });
    
    auto btn_clear = button("C", [&] {
        display = "0";
        stored_value = 0;
        operation = "";
        new_input = true;
    });
    
    // ===== LAYOUT =====
    auto button_grid = Container::Vertical({
        Container::Horizontal({btn7, btn8, btn9, btn_add}),
        Container::Horizontal({btn4, btn5, btn6, btn_sub}),
        Container::Horizontal({btn1, btn2, btn3, btn_mul}),
        Container::Horizontal({btn0, btn_clear, btn_equals, btn_div}),
    });
    
    // ===== RENDERER =====
    auto renderer = Renderer(button_grid, [&] {
        Element status;
        if (operation != "") {
            status = text(display + " " + operation);
        } else {
            status = text(display);
        }
        
        return vbox(
            text("UltraCalc") | bold | hcenter | size(HEIGHT, EQUAL, 1),
            separator(),
            status | hcenter | bold | size(HEIGHT, EQUAL, 2) | size(WIDTH, EQUAL, 15),
            separator(),
            button_grid->Render() | hcenter
        ) | border;
    });
    
    // ===== RUN =====
    screen.Loop(renderer);
    return 0;
}
