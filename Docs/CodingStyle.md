# Coding style

This document describe the coding style that the project's code follows.

- [Kernighan & Ritchie](https://en.wikipedia.org/wiki/Indentation_style#K&R) style brackets
- Tabs are 4 spaces wide, and use the '\t' character
- Structs and enumed should **NOT** be `typedef`-ed, unless for abstraction, or when defining a class (e.g. the [Framebuffer class](../Kernel/Drivers/Graphics/Framebuffer.h))

- By default, names are lowerCamelCase (function names, attributes, global variables, etc)
- Local variables can be snake_case
- Global variables are prefixed with 'g_': 'g_someGlobalVariable'
- Type names (`typedef`, `enum`, `struct` and classes) are UpperCamelCase, or snake_case with optional `_t` prefix
- Constant global variables are SCREAMING_SNAKE_CASE

- OOP may be used when necessary:
  - Classes name are upper camel case
  - Methods should be prefixed with the class name followed by an underscore: `ClassName_methodName(ClassName* this, ...)`
  - Member variables, used at global scoped, should always be `static` and prefixed with 'm_' (instead of 'g_')

Note: the coding style does NOT apply to the implementation(s) of the C standard library
