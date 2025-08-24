# async_logger

An industrial-grade, asynchronous C++20 logging library built on a high-performance ring buffer. Designed for easy integration via CMake’s `find_package` or `FetchContent`.

## Features

- Five log levels: `Debug`, `Info`, `Warn`, `Error`, `Fatal`  
- Non-blocking, lock-free enqueue via ring buffer  
- Background worker thread for file I/O  
- Configurable log file, level filtering, and formatting (timestamp, level tag, message)  
- Zero-overhead when log level is below threshold  
- Simple C++ interface: `Logger::debug("…")`, `Logger::info("…")`, etc.

## Requirements

- C++20-compatible compiler  
- CMake >= 3.15  
- [`ring_buffer`](https://github.com/HuRuilizhen/ring_buffer) library (headers & CMake config)  
- (Optional) GoogleTest for unit tests

## Table of Contents

- [Building](#building)  
- [Running Tests](#running-tests)  
- [Using the Library](#using-the-library)  
- [Using FetchContent](#using-fetchcontent)  
- [Uninstall](#uninstall)  
- [Contributing](#contributing)  

## Building

Choose your generator and configure:

```bash
cmake -S . -B build -G Ninja    # or Xcode / "Unix Makefiles"
cmake --build build
````

Enable tests and example (and fetch GoogleTest) in one go:

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DENABLE_TESTS=ON -DENABLE_EXAMPLE=ON
cmake --build build
sudo cmake --install build
```

Build without tests for a lean installation:

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
sudo cmake --install build
```

## Running Tests

After a test-enabled build:

```bash
cd build && ctest --output-on-failure
```

## Using the Library

### Via `find_package`

After installing `async_logger`, add to your project:

```cmake
find_package(async_logger CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE
  async_logger::async_logger
)
```

In code:

```cpp
#include <async_logger/logger.h>

int main() {
    async_logger::Logger::init("app.log", async_logger::Level::Debug);
    async_logger::Logger::info("Application started");
    // …
    async_logger::Logger::shutdown();
    return 0;
}
```

## Using FetchContent

Without installation, pull both `ring_buffer` and `async_logger`:

```cmake
include(FetchContent)

# 1) ring_buffer dependency
FetchContent_Declare(
  ring_buffer
  GIT_REPOSITORY https://github.com/HuRuilizhen/ring_buffer.git
  GIT_TAG        v1.0.3
)
# 2) async_logger itself
FetchContent_Declare(
  async_logger
  GIT_REPOSITORY https://github.com/YourUser/async_logger.git
  GIT_TAG        v1.0.0
)

FetchContent_MakeAvailable(ring_buffer async_logger)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE
  async_logger::async_logger
)
```

## Uninstall

If you installed via CMake:

```bash
sudo cmake --build build --target uninstall_async_logger
```

## Contributing

Contributions are welcome! Feel free to open issues or pull requests on GitHub.
