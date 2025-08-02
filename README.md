# mugOS

This is a custom operating system, written for learning and experimentation purposes during my free time.

See:
- The [general](./Docs/General.md) documentation for OS features
- The [building](./Docs/Building.md) documentation for building locally, or with docker
- The [running](./Docs/Running.md) documentation for emulation, debugging, or running on hardware
- The [roadmap](./Docs/Roadmap.md) for future plans
- The [coding style](./Docs/CodingStyle.md) if you want to contribute

## Try it yourself !

If you want to quickly run the OS, try the tetris release ! It is a modified
version of v0.2 (now quite old), made as a Proof of Concept, to show that
the OS is capable of running a relatively complex game such as tetris.

Just [download it](https://github.com/Magyar57/mugOS/releases/tag/v0.2-tetris)
and run it in QEMU: `qemu-system-x86_64 -fda floppy.img`.
