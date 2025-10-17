# 🕹️ cart
Simple Gameboy emulator written for fun. After implementing all DMG functionality I plan to also add Gameboy Color emulation.

## 🔧 Building
### Prerequisites
- C compiler (any)
- [CMake](https://cmake.org/)
- [SDL3](https://github.com/libsdl-org/SDL)

### Steps
1. Clone and navigate to this repository
2. **If you don't have SDL3 installed** place its [source](https://github.com/libsdl-org/SDL) in a `third-party` directory inside this repo
3. Run:
```bash
cmake -Bbuild .
```
4. Run:
```bash
cmake --build build
```
