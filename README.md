# async_logger

![Build Status](https://img.shields.io/github/actions/workflow/status/HuRuilizhen/async_logger/cmake-multi-platform.yml?branch=release)

An industrial-grade, asynchronous C++20 logging library built on a high-performance ring buffer. Unit tests and sample usage provided. Designed for easy integration via CMake’s `find_package` or `FetchContent`.

## Features

- Five log levels: `Debug`, `Info`, `Warn`, `Error`, `Fatal`  
- Non-blocking, lock-free enqueue via mpsc ring buffer  
- Background worker thread for file I/O
- Log file rotation on date change
- Configurable log file, level filtering, and formatting (level, timestamp, location, message)  
- Zero-overhead when log level is below threshold  
- Simple C++ interface: `Logger::debug("…")`, `Logger::info("…")`, etc.

## Requirements

- C++20-compatible compiler  
- CMake >= 3.15  
- [`ring_buffer`](https://github.com/HuRuilizhen/ring_buffer) library (headers & CMake config)  
- (Optional) GoogleTest for unit tests

## Table of Contents

- [async\_logger](#async_logger)
  - [Features](#features)
  - [Requirements](#requirements)
  - [Table of Contents](#table-of-contents)
  - [Building](#building)
    - [Configuration](#configuration)
    - [Execution](#execution)
  - [Running Examples](#running-examples)
  - [Running Tests](#running-tests)
  - [Using the Library](#using-the-library)
    - [Including the Library](#including-the-library)
    - [Supported APIs](#supported-apis)
      - [Initialization \& Shutdown](#initialization--shutdown)
      - [Logging API (static methods)](#logging-api-static-methods)
      - [Logging via Macros](#logging-via-macros)
    - [Quick Example](#quick-example)
  - [Uninstall](#uninstall)
  - [Contributing](#contributing)

## Building

### Configuration

| Option                          | Default | Description                               |
| ------------------------------- | ------- | ----------------------------------------- |
| `ENABLE_TESTS`                  | OFF     | Build unit tests (requires GTest)         |
| `ENABLE_EXAMPLE`                | OFF     | Build examples                            |
| `CMAKE_EXPORT_COMPILE_COMMANDS` | OFF     | Generate `compile_commands.json` for IDEs |

### Execution

```bash
# Default build
cmake -S . -B build
cmake --build build

# Build with tests + examples
cmake -S . -B build -DENABLE_TESTS=ON -DENABLE_EXAMPLE=ON
cmake --build build

# Install
sudo cmake --install build
```

## Running Examples

```bash
cd ./build/bin && ./async_logger_example
```

or 

```bash
cd ./build/bin && ./async_logger_macro_example
```

## Running Tests

```bash
cd ./build && ctest --output-on-failure
```

## Using the Library

### Including the Library

Without installation, pull both `ring_buffer` and `async_logger`:

```cmake
include(FetchContent)

# 1) ring_buffer dependency (optional)
FetchContent_Declare(
  ring_buffer
  GIT_REPOSITORY https://github.com/HuRuilizhen/ring_buffer.git
  GIT_TAG        v0.1.0
)
# 2) async_logger itself
FetchContent_Declare(
  async_logger
  GIT_REPOSITORY https://github.com/YourUser/async_logger.git
  GIT_TAG        v0.1.0
)

FetchContent_MakeAvailable(ring_buffer async_logger)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE async_logger)
```

or after installation:

```cmake
find_package(async_logger CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE
  async_logger::async_logger
)
```

### Supported APIs

Namespace: `AsyncLogger`

#### Initialization & Shutdown

```cpp
// Initialize the logger (default config logs to stdout + file)
static void Logger::init(const Config& config = Config());

// Gracefully stop background thread, flush pending logs
static void Logger::shutdown();
```

**Config fields:**

```cpp
struct Config {
  std::string filename{}; // log file path (if out_file enabled), if empty use time-stamped filename with rotation
  int flag;               // output flags, see OutstreamFlag
  Level level;            // minimum log level
};
```

**Flags (`OutstreamFlag`):**

* `out_stdout`  $\rightarrow$ log to stdout
* `out_stderr`  $\rightarrow$ log to stderr
* `out_file`    $\rightarrow$ log to file (`Config::filename`)
* `out_color`   $\rightarrow$ enable colored log levels (stdout and stderr only)
* `mode_append` $\rightarrow$ append to file instead of overwrite

#### Logging API (static methods)

```cpp
Logger::debug(const std::string& msg,
              const std::source_location& loc = std::source_location::current());

Logger::info(const std::string& msg,
             const std::source_location& loc = std::source_location::current());

Logger::warn(const std::string& msg,
             const std::source_location& loc = std::source_location::current());

Logger::error(const std::string& msg,
              const std::source_location& loc = std::source_location::current());

Logger::fatal(const std::string& msg,
              const std::source_location& loc = std::source_location::current());
```

* All methods automatically capture file/line/function via `std::source_location`.
* Log entries are enqueued into a `MPSCRingBuffer` and written asynchronously by a worker thread.

#### Logging via Macros

Convenient macros are provided for inline logging:

```cpp
LOG_DEBUG("message");
LOG_INFO("message");
LOG_WARN("message");
LOG_ERROR("message");
LOG_FATAL("message");

LOGF_INFO("formatted number: {}", 42);
LOGF_ERROR("failed: {} ({})", filename, errno);
```

* `LOG_*` $\rightarrow$ plain message
* `LOGF_*` $\rightarrow$ formatted message via `std::format`

### Quick Example

```cpp
#include <async_logger/async_logger.h>

int main() {
    AsyncLogger::Logger::info("Application started");
    // …
    AsyncLogger::Logger::shutdown();
    return 0;
}
```

## Uninstall

If you installed via CMake:

```bash
sudo cmake --build build --target uninstall_async_logger
```

## Contributing

Contributions are welcome! Feel free to open issues or pull requests on GitHub.
