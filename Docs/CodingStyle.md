# Coding style

This document describe the coding style that the project's code follows.

Note: a 'typedef' and a 'type alias', or simply 'alias', are the same thing

- [Kernighan & Ritchie](https://en.wikipedia.org/wiki/Indentation_style#K&R) style brackets
- Tabs are 4 spaces wide, and use the '\t' character
- Structs and enums should **NOT** be typedef-ed, unless for abstraction or OOP (see below)

- By default, names are lowerCamelCase (function names, attributes, global variables, etc)
- Local variables *can* be snake_case
- Global variables are prefixed with 'g_': 'g_someGlobalVariable'
- Un-aliased structs and enums are UpperCamelCase (`struct MyStruct`)
- Non-class types aliases are snake_case, with a `_t` suffix (eg [`irqhandler_t`](../Kernel/IRQ.h))
- Macros that defines values (eg `define MY_MACRO 10`), and constant global variables are SCREAMING_SNAKE_CASE

- OOP may be used when necessary:
  - Modules and classes names are UpperCamelCase
  - Modules should be prefixed with their name, followed by an underscore:
    `ModuleName_functionName(...)`
  - Modules' privates members should always be `static` and prefixed with `m_` (instead of `g_`)
  - Do **NOT** define classes in modules, modules should only contain members and functions
  - Classes should be typedef-ed, with either the UpperCamelCase name
    (eg [`Framebuffer`](../Kernel/Drivers/Graphics/Framebuffer.h)), or a non-class type alias
  - Methods should be prefixed with the UpperCamelCase class name,
    as modules: `ClassName_methodName(ClassName* this, ...)`

The use of modules is STRONGLY recommended. But they can be avoided, for example when implementing
basic features such as logging (`log`, `hexdump`) or a heap (`kmalloc`, `kfree`).

Note: the coding style does NOT apply to the implementation(s) of the C standard library, which
      must follow the C standard.
