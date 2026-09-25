# C++ Debugging Complete Guide

## Overview
This guide covers all debugging approaches for your C++ programs.

---

## 1. COMPILATION WITH DEBUG SYMBOLS (Required for all approaches)

### Build with Debug Symbols
```bash
make              # Builds all programs with -g -O0 flags
make out/lambda   # Build specific program
```

**Flags explained:**
- `-g`: Includes debug symbols in the binary
- `-O0`: Disables optimizations (essential for accurate debugging)
- `-Wall -Wextra`: Show all warnings

---

## 2. GDBDEBUGGER - Command Line

### Start GDB Session
```bash
gdb ./out/lambda
```

### Common GDB Commands

**Navigation & Execution:**
```
run              - Start program execution
break main       - Set breakpoint at main
break lambda.cpp:10  - Set breakpoint at specific line
continue         - Continue execution
next             - Execute next line (step over)
step             - Step into function
finish           - Run until function returns
```

**Inspection:**
```
print x          - Print variable value
print &x         - Print variable address
print sizeof(int) - Evaluate expressions
info locals      - Show all local variables
info registers   - Show register contents
```

**Assembly & Disassembly:**
```
disassemble main    - Show assembly of main function
disassemble /m main - Show assembly with source code
set disassembly-flavor intel  - Use Intel syntax (easier to read)
info registers      - Show CPU registers
x/10i $pc           - Display 10 instructions from program counter
```

**Example GDB Session:**
```bash
gdb ./out/lambda
(gdb) set disassembly-flavor intel
(gdb) break main
(gdb) run
(gdb) disassemble /m main
(gdb) next
(gdb) print myVariable
(gdb) continue
(gdb) quit
```

---

## 3. VS CODE INTEGRATED DEBUGGER

### Quick Start
1. Open any `.cpp` file
2. Click left of a line number to set breakpoint (red dot appears)
3. Press `F5` or go to Run → Start Debugging
4. Select debug configuration (e.g., "GDB Debug (lambda)")
5. Debugger will pause at breakpoint

### VS Code Debugging Features
- **Variables Panel**: See all local variables with values
- **Call Stack**: See function call hierarchy
- **Watch**: Add custom expressions to monitor
- **Debug Console**: Execute GDB commands directly
- **Hover**: Hover over variables to see values

### Keyboard Shortcuts
- `F5`: Start debugging
- `F10`: Step over
- `F11`: Step into
- `Shift+F11`: Step out
- `Ctrl+K Ctrl+I`: Show hover value

---

## 4. VIEWING ASSEMBLY CODE

### Option A: Generate Assembly Files
```bash
make asm           # Generates .s files in out/ directory
cat out/lambda.s   # View assembly source code
```

### Option B: Disassemble Compiled Binary
```bash
make disasm        # Shows disassembly with objdump
objdump -d out/lambda | less  # Full disassembly with addresses
objdump -S out/lambda | less  # Disassembly with source code interleaved
```

### Option C: In GDB
```bash
gdb ./out/lambda
(gdb) disassemble /m main     # Show assembly with source
(gdb) disassemble main /0     # Show just assembly
```

---

## 5. DETAILED DEBUGGING WORKFLOW

### Step-by-Step Debugging with Assembly

**Terminal Approach:**
```bash
# 1. Build with debug symbols
make out/lambda

# 2. Start GDB
gdb ./out/lambda

# 3. Configure GDB
(gdb) set disassembly-flavor intel
(gdb) break main
(gdb) run

# 4. Step through code and view assembly
(gdb) disassemble /m main
(gdb) next
(gdb) step
(gdb) x/5i $pc        # Show 5 instructions from current position

# 5. Inspect memory and registers
(gdb) info registers  # Show all CPU registers
(gdb) print $rax      # Show specific register value
(gdb) print $rsp      # Stack pointer
```

**VS Code Approach:**
1. Set breakpoint in source code
2. Press F5 to start debugging
3. Open Debug Console (Ctrl+Shift+Y)
4. Type GDB commands:
   ```
   disassemble /m main
   info registers
   x/10i $pc
   ```

---

## 6. MEMORY DEBUGGING

### Detect Memory Leaks with Valgrind
```bash
valgrind --leak-check=full ./out/lambda
```

### Monitor Memory Usage
```bash
gdb ./out/lambda
(gdb) info locals      # Show local variables and addresses
(gdb) print &myVar     # Show memory address of variable
(gdb) x/4xw &myVar     # Show 4 words of memory at address
```

---

## 7. PERFORMANCE PROFILING

### Stack Trace with GDB
```bash
gdb ./out/lambda
(gdb) run
(gdb) bt              # Backtrace - show call stack
(gdb) frame 0         # Switch to frame
```

### Trace System Calls
```bash
strace ./out/lambda
```

---

## 8. WHICH APPROACH TO USE?

| Task | Best Approach |
|------|---|
| **Step through code line by line** | VS Code Debugger (F5) or GDB |
| **View assembly code** | `make asm` or GDB `disassemble /m` |
| **Find variable values** | GDB `print` command or VS Code Variables panel |
| **Memory leak detection** | `valgrind` |
| **Understand register contents** | GDB `info registers` |
| **System call tracing** | `strace` |
| **Quick breakpoint debugging** | VS Code with F5 |
| **Complex inspection** | Terminal GDB with full access |

---

## 9. EXAMPLE DEBUGGING SESSION

**Scenario:** Debug lambda.cpp and see assembly

```bash
# Build
make out/lambda

# Method 1: VS Code GUI (Easiest)
# - Open lambda.cpp
# - Click line 20 to set breakpoint
# - Press F5
# - Hover over variables to see values
# - Use Debug Console to run: disassemble /m

# Method 2: GDB Terminal (Full Control)
gdb ./out/lambda
(gdb) set disassembly-flavor intel
(gdb) break lambda.cpp:20
(gdb) run
(gdb) disassemble /m                    # See assembly around breakpoint
(gdb) info registers                    # See CPU registers
(gdb) next                              # Step to next line
(gdb) print memberVariable              # Check variable
(gdb) quit
```

---

## 10. TROUBLESHOOTING

**Problem:** "No debugging symbols found"
```bash
# Solution: Rebuild with -g flag
make clean
make
```

**Problem:** Cannot see source code in disassembly
```bash
# Solution: Use objdump -S instead
objdump -S out/lambda | less
```

**Problem:** GDB stops at weird locations
```bash
# Solution: Disable optimizations in Makefile
# Ensure CXXFLAGS has -O0, not -O2 or -O3
```

---

## Quick Reference Card

```bash
# Build & Run
make                    # Build all with debug symbols
make out/lambda         # Build specific program
./out/lambda            # Run program (no debugging)

# Debugging
gdb ./out/lambda        # Start GDB
make asm                # Generate assembly files
make disasm             # Show disassembly
valgrind ./out/lambda   # Check for memory leaks

# VS Code
F5                      # Start debugging
F10                     # Step over
F11                     # Step into
Ctrl+Shift+Y            # Show Debug Console
```

