# Debugging Guide for Annatar in IntelliJ/CLion

## Quick Start: Debug in Terminal

The game is now built with debug symbols. Run it and watch the console output:

```bash
cd /Users/dan.clarke/workspace/personal/Annatar/build/src
./Annatar
```

You should see detailed output showing exactly where it crashes.

---

## Setup IntelliJ/CLion Debugging

### Option 1: Import CMake Project (Recommended)

1. **Open IntelliJ IDEA/CLion**
2. **File → Open** → Select `/Users/dan.clarke/workspace/personal/Annatar`
3. CLion will automatically detect CMakeLists.txt
4. **Wait for CMake to configure** (may take 30-60 seconds)
5. CLion will build the project automatically

### Option 2: Manual Configuration

1. **Open Project** in IntelliJ
2. **Run → Edit Configurations**
3. **Add New Configuration** → CMake Application
4. Set:
   - **Name**: Annatar (Debug)
   - **Target**: Annatar
   - **Executable**: `build/src/Annatar`
   - **Program arguments**: (leave empty)
   - **Working directory**: `$PROJECT_DIR$/build/src`
   - **Before launch**: Build

---

## Debugging the Crash

### Current Crash Location

Based on the output:
```
[ECS] Setting up ECS Play State...
Loaded 8 weapons from config/weapons.toml
[segmentation fault]
```

The crash is happening **after loading weapons** but **before printing "Loading configuration..."**.

This means it's likely crashing in:
- `config.LoadEnemies()`
- `config.LoadConstants()`

### Debug Steps

1. **Set breakpoint** in `src/ecs/config/config_loader.cc::LoadAll()`
2. **Run in debugger**
3. **Step through** to find exact line causing crash
4. **Check variables**:
   - Are TOML files parsing correctly?
   - Are arrays being accessed out of bounds?
   - Are color/vector arrays the right size?

### Likely Issues

**Issue 1**: TOML parsing error in enemies.toml or constants.toml
- Missing required fields
- Wrong array sizes
- Type mismatches

**Issue 2**: toml::array access returning nullptr
- Arrays might not have enough elements
- Need to check size before accessing

---

## CLion Specific Features

### Enable Address Sanitizer (Detects Memory Issues)

1. **Run → Edit Configurations**
2. **Annatar configuration → Edit**
3. **Environment variables**: `ASAN_OPTIONS=detect_leaks=1`
4. **CMake options**: Add `-DCMAKE_CXX_FLAGS="-fsanitize=address -g"`
5. **Rebuild** and **Run**

Address Sanitizer will give detailed crash information:
- Exact line number
- Stack trace
- Type of memory error

### Debug Console

When running in debug mode, CLion shows:
- **stdout**: Your `std::cout` messages
- **stderr**: Error messages
- **Debugger**: Variable inspection, call stack

---

## Quick Fix: Add Error Handling to ConfigLoader

The crash is likely in `config_loader.cc` when parsing arrays. Let me identify the risky code:

### Risky Code Pattern

```cpp
// In ConfigLoader::ParseVector2f()
float x = vec_array.get(0)->value_or(0.0);  // Could crash if get(0) returns nullptr
float y = vec_array.get(1)->value_or(0.0);
```

### Safe Pattern

```cpp
if (vec_array.size() >= 2) {
    float x = vec_array.get(0)->value_or(0.0);
    float y = vec_array.get(1)->value_or(0.0);
    return {x, y};
}
return {0.0f, 0.0f};
```

This pattern is already in the code! So the crash might be elsewhere...

---

## Run With Detailed Output

Try running the game directly from the terminal to see all output:

```bash
cd /Users/dan.clarke/workspace/personal/Annatar/build/src
./Annatar 2>&1 | tee game.log
```

This will:
- Show all output (stdout + stderr)
- Save it to `game.log` for analysis
- Show exactly which line printed before crash

---

## Expected Console Output (Successful Run)

```
[ECS] Setting up ECS Play State...
[ECS] Loading configuration...
Loaded 8 weapons from config/weapons.toml
Loaded 5 enemies from config/enemies.toml
Loaded constants from config/constants.toml
[ECS] Getting constants...
[ECS] Configuration loaded successfully
[ECS] Creating entity factory...
[ECS] Entity factory created
[ECS] Creating player...
[ECS] Player created at (400, 500)
[ECS] Spawning enemy 1...
[ECS] Spawning enemy 2...
[ECS] Spawning enemy 3...
[ECS] Initial enemies spawned
[ECS] Total entities: 4
```

---

## Crash Analysis

### What We Know
1. ✅ Weapons loaded successfully (8 weapons)
2. ❌ Crashes after that
3. ❌ Before "Getting constants..." message

### Most Likely Cause
The crash is in `config.LoadEnemies()` or `config.LoadConstants()`.

Suspect line in config_loader.cc might be accessing:
- `vec_array.get(0)` or `.get(1)` when array is empty
- `color_array.get(0/1/2)` when array has < 3 elements
- `table->get("field")` when field doesn't exist

---

## IntelliJ lldb Commands

If debugging in IntelliJ/CLion console, you can use lldb commands:

```lldb
# Run until crash
r

# Show backtrace
bt

# Show current line
frame info

# Print variable
p variable_name

# Continue execution
c

# Step over
n

# Step into
s
```

---

## Next Steps

1. **Run game from terminal** with detailed output
2. **See exactly where it crashes** (which file isn't loading)
3. **Fix the config issue** (likely a TOML format problem)
4. **OR** open in CLion and **set breakpoint** in `config_loader.cc::LoadAll()`

**Let me know what the detailed output shows and I'll help fix it!**
