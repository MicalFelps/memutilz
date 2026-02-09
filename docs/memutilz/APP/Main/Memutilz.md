### GUI

Unified GUI frontend for cross-platform debugging. The front-end only works on x86-64 because you'll never need anything more than that.

### Debugging Backend

Depends on the platform
- gdb for linux, (either MI = Machine Interface via subprocess, or direct ptrace via libgdbr or your own ptrace code)
- wrapper around DbgEng ( dbgeng.dll ) -> COM interfaces like IDebugClient, IDebugControl, IDebugSystemObjects
- LLDB for something that works on both

With a common abstraction unifying them all, something like `IDebuggerBackend` with methods like `attach(pid)`, `continue()`,or `stepInto()`.

### Tracing

since x64dbg is user-mode, it uses the windows debug API: (DebugActiveProcess / WaitForDebugEvent / ContinueDebugEvent loop) + hardware single-step (Trap Flag).

When you do trace into/over or conditional tracing:
Single-step exceptions (via CONTEXT.EFlags.TF = 1 or hardware breakpoints), then on each single-step exception (DEBUG_EVENT)
- log current instruction (disasm via Zydis)
- dump registers
- log memory operands read/written to (by parsing instruction, then checking before and after at the memory location)
- store all this information in some memory buffer or stream to disk (.trace32/.trace64 files - binary packed format with JSON header + blocks for each step)

Conditional tracing works by only tracing / breaking if x condition is true, pretty self explanatory. It's very slow but that's to be expected with single-stepping via debug API / TF
### Scripting

Either embed python, lua, or your own DSL for easy scripting. This could be really really fun.
### Memory Scanning

Relies on SIMD and platform specific APIs, but the flow is the same, get valid regions, scan all regions with x threads.

### Plugins

For windows
- DLL injection and ImGui
Custom VM Bytecode
- Make the disassembler work with different programming languages, and their custom bytecodes, like java, C# and other .Net languages, and so on.
My own decompiler (eventually)
Kernel support (hopefully eventually)

## Ideas

- Anti-Anti-Debugging, something like scyllaHide, or TitanHide
- With signatures, you can do a lot
- Qemu Support

### ARM SUPPORT

Use LLVM MCDisassembler / MCInst layer (what LLDB, clang, llvm-objdump use internally)