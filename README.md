# Telegram Hybrid Assistant

A high-performance, lightweight asynchronous Telegram auto-responder written in modern C++23. It operates natively via long polling using `libcurl` and features zero external dependencies for JSON parsing.

## Architectural Features
- **Hybrid API Support**: Seamlessly processes both standard direct bot interactions (`message`) and enterprise account delegations (`business_message`).
- **RAII Resource Management**: Low-level networking primitives wrapper around `libcurl` ensures automated scope-bound allocations and avoids leaks.
- **Zero-Dependency Matching**: Utilizes memory-efficient `std::string_view` for linear runtime extraction of contextual values from JSON streams, completely avoiding allocations during parse steps.
- **Standard Compliant**: Targets strictly isolated `C++23` compiler features (such as `std::string::contains`).

## Prerequisites
- CMake (version 3.20 or higher)
- Clang/AppleClang (supporting C++23 flags) or GCC 13+
- Libcurl development headers (`libcurl4-openssl-dev` on Linux, Native SDK on macOS)

## Compilation Instructions

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Configuration & Deployment

Execution context reads the entry credentials securely from system environment flags. Set the endpoint token prior to operational binary initialization:

```bash
export TELEGRAM_BOT_TOKEN="YOUR_ACTUAL_TOKEN_HERE"
./tg_assistant
```
