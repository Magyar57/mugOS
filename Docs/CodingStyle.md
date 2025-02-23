# Coding style

This document describe the coding style that the project's code follows.

- [Kernighan & Ritchie](https://en.wikipedia.org/wiki/Indentation_style#K&R) style brackets
- Tabs are 4 spaces wide, and use the '\t' character
- Structs and enumed should **NOT** be `typedef`-ed, unless for abstraction, or when defining a class (e.g. the [Framebuffer class](../Kernel/Drivers/Graphics/Framebuffer.h))

- By default, names are lower camel case (function names, attributes, global variables, etc)
- Local variables are snake case
- Global variables are prefixed with 'g_': 'g_someGlobalVariable'
- Type names (`typedef`, `enum`, `struct` and classes) are upper camel case

- OOP may be used when necessary:
  - Classes name are upper camel case
  - Methods should be prefixed with the class name followed by an underscore: `ClassName_methodName(ClassName* this, ...)`
  - Member/static types, used at global scoped, should be prefixed with 'm_' instead of 'g_'

Note: the coding style does NOT apply to the implementation(s) of the C standard library
