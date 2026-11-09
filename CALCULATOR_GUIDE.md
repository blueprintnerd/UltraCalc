# UltraCalc TUI Calculator

Your **simple TUI calculator** is ready! Built with FTXUI and your math functions.

## Quick Start

```bash
# Build
cd build && cmake --build .

# Run
./ultra_calc
```

**Press Q to quit**

---

## How It Works

### The Calculator has:
- **Display** at the top showing current number and operation
- **Number buttons** 0-9
- **Operation buttons**: +, -, *, /
- **Equals button** = to calculate
- **Clear button** C to reset

### Usage:
1. Enter first number (e.g., press 5, then 0 → displays "50")
2. Press operation (e.g., +)
3. Enter second number (e.g., press 2, then 5 → displays "25 +")
4. Press = to see result (displays "75")
5. Press C to clear and start over

---

## Code Structure

### `tui.cpp` - The Main Calculator

```cpp
// State variables
std::string display = "0";      // Current display text
float stored_value = 0;          // First operand
std::string operation = "";      // Current operation (+, -, *, /)
bool new_input = true;           // Whether to start fresh

// Button creation helper
auto button = [](const std::string& label, auto on_click) {
    return Button(label, on_click);
};

// Number buttons
btn0, btn1, btn2, ... btn9

// Operation buttons
btn_add, btn_sub, btn_mul, btn_div, btn_equals, btn_clear

// Layout
Container::Vertical({
    Container::Horizontal({btn7, btn8, btn9, btn_add}),
    Container::Horizontal({btn4, btn5, btn6, btn_sub}),
    Container::Horizontal({btn1, btn2, btn3, btn_mul}),
    Container::Horizontal({btn0, btn_clear, btn_equals, btn_div}),
})

// Renderer
Renderer(button_grid, [&] {
    return vbox({
        text("UltraCalc"),
        separator(),
        text(display + " " + operation),  // Display
        separator(),
        button_grid->Render()
    }) | border;
})
```

---

## Customizing Your Calculator

### Change the Title
```cpp
// Change line 143 in tui.cpp
text("UltraCalc") → text("My Calculator")
```

### Add More Operations
Add to the operation buttons section:
```cpp
auto btn_mod = button("%", [&] {
    stored_value = std::stof(display);
    operation = "mod";
    new_input = true;
});
```

Then update the equals handler:
```cpp
else if (operation == "mod") {
    result = fmod(stored_value, current);
}
```

### Change Button Layout
Edit the `button_grid` Container::Vertical block. Example - add a row:
```cpp
Container::Vertical({
    Container::Horizontal({btn7, btn8, btn9, btn_add}),
    Container::Horizontal({btn4, btn5, btn6, btn_sub}),
    Container::Horizontal({btn1, btn2, btn3, btn_mul}),
    Container::Horizontal({btn0, btn_clear}),
    Container::Horizontal({btn_equals, btn_div}),
})
```

### Style Improvements
Make the display bigger:
```cpp
text(display) | size(HEIGHT, EQUAL, 3) | size(WIDTH, EQUAL, 20)
```

Add colors:
```cpp
text(display) | color(Color::White) | bgcolor(Color::Black)
```

---

## Key Files

| File | Purpose |
|------|---------|
| `tui.cpp` | Main calculator code |
| `math.cpp` | Your math functions |
| `math.hpp` | Math function headers |
| `CMakeLists.txt` | Build configuration |
| `build/ultra_calc` | Compiled executable |

---

## Common Modifications

### Fix Division by Zero
In `math.cpp`, add a check:
```cpp
float divider(int num1, int num2, int flag_nodec) {
    if (num2 == 0) {
        return 0;  // Or throw an exception
    }
    // ... rest of the function
}
```

### Add Decimal Support
Update button handlers to accept decimal points:
```cpp
auto btn_decimal = button(".", [&] {
    if (new_input) { display = "0"; new_input = false; }
    if (display.find('.') == std::string::npos) {
        display += ".";
    }
});
```

### Better Number Formatting
Improve the display formatting in btn_equals:
```cpp
// Remove trailing zeros from floats
std::string format_float(float value) {
    std::string result = std::to_string(value);
    result.erase(result.find_last_not_of('0') + 1, std::string::npos);
    if (result.back() == '.') result.pop_back();
    return result;
}
// Then use: display = format_float(result);
```

---

## Troubleshooting

### "Button was not declared"
Make sure you have:
```cpp
#include <ftxui/component/component.hpp>
```

### "Renderer was not declared"
Make sure you have:
```cpp
#include <ftxui/component/component.hpp>
```

### Build fails
```bash
rm -rf build && mkdir build && cd build && cmake .. && cmake --build .
```

---

## Extending the Calculator

### Add Memory Functions
```cpp
float memory = 0;

auto btn_ms = button("MS", [&] { memory = std::stof(display); });
auto btn_mr = button("MR", [&] { display = std::to_string(memory); });
auto btn_mc = button("MC", [&] { memory = 0; });
auto btn_m_plus = button("M+", [&] { memory += std::stof(display); });
```

### Add History
```cpp
std::vector<std::string> history;

// In btn_equals, after calculation:
history.push_back(display + " " + operation + " " + std::to_string(stored_value) + " = " + display);

// Display history
for (const auto& item : history) {
    elements.push_back(text(item));
}
```

### Add Keyboard Support
FTXUI automatically supports keyboard navigation. Use:
- **Arrow keys** to navigate
- **Enter** to click buttons
- **Q** to quit

---

## Next Steps

1. **Try it:** `./build/ultra_calc`
2. **Customize:** Change the title, colors, or layout
3. **Extend:** Add memory functions, history, or scientific operations
4. **Improve:** Better error handling, decimal support

---

## Need Help?

Ask me to:
- "Add memory functions to the calculator"
- "Make the display show history"
- "Add scientific operations (sin, cos, sqrt)"
- "Fix division by zero"
- "Add keyboard shortcuts"
