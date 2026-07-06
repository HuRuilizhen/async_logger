# async_logger

![Build Status](https://img.shields.io/github/actions/workflow/status/HuRuilizhen/async_logger/cmake-multi-platform.yml?branch=release)

An industrial-grade, asynchronous C++20 logging library built on a high-performance ring buffer. Unit tests and sample usage provided. Designed for easy integration via CMake's `find_package` or `FetchContent`.

## Features

- Five log levels: `Debug`, `Info`, `Warn`, `Error`, `Fatal`  
- Non-blocking, lock-free enqueue via mpsc ring buffer  
- Background worker thread for file I/O
- Configurable idle behavior via blocking or yielding worker wait strategies
- Log file rotation on date change
- Queue drop accounting via `Logger::droppedCount()`
- Library-friendly file-open errors via `AsyncLogger::FileOpenError`
- Configurable log file, level filtering, and formatting (level, timestamp, location, message)  
- Zero-overhead when log level is below threshold  
- Simple C++ interface: `Logger::debug("…")`, `Logger::info("…")`, etc.

## Requirements

- C++20-compatible compiler  
- CMake >= 3.20  
- Network access for dependency fetching in development mode, or a local
  [`ring_buffer`](https://github.com/HuRuilizhen/ring_buffer) installation when using the
  `PACKAGE` provider
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

| Option                               | Default | Description                                                    |
| ------------------------------------ | ------- | -------------------------------------------------------------- |
| `ENABLE_TESTS`                       | OFF     | Build unit tests                                               |
| `ENABLE_EXAMPLE`                     | OFF     | Build examples                                                 |
| `CMAKE_EXPORT_COMPILE_COMMANDS`      | OFF     | Generate `compile_commands.json` for IDEs                      |
| `ASYNC_LOGGER_RING_BUFFER_PROVIDER`  | `AUTO`  | Resolve `ring_buffer` via `FETCH`, `PACKAGE`, or `AUTO`        |

### Execution

```bash
# Recommended development build
cmake --preset debug
cmake --build --preset debug

# Release build
cmake --preset release
cmake --build --preset release

# Run tests
ctest --preset debug

# Strictly use an installed ring_buffer package
cmake -S . -B build/package \
  -DASYNC_LOGGER_RING_BUFFER_PROVIDER=PACKAGE \
  -DCMAKE_BUILD_TYPE=Debug

# Try local package first, then fall back to FetchContent
cmake -S . -B build/auto \
  -DASYNC_LOGGER_RING_BUFFER_PROVIDER=AUTO \
  -DCMAKE_BUILD_TYPE=Debug

# Install from a configured build tree
sudo cmake --install build/release
```

## Running Examples

```bash
./build/debug/bin/async_logger_example
```

or 

```bash
./build/debug/bin/async_logger_macro_example
```

## Running Tests

```bash
ctest --preset debug
```

## Using the Library

### Including the Library

Without installation, pull `async_logger` via `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
  async_logger
  GIT_REPOSITORY https://github.com/YourUser/async_logger.git
  GIT_TAG        v0.2.0
)

FetchContent_MakeAvailable(async_logger)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE async_logger::async_logger)
```

By default, `async_logger` uses `ASYNC_LOGGER_RING_BUFFER_PROVIDER=AUTO`.
That means it will:

- reuse a `ring_buffer` target already provided by the caller
- otherwise try `find_package(ring_buffer 0.1.0 CONFIG)`
- otherwise fall back to `FetchContent`

If you want stricter behavior, set the provider before
`FetchContent_MakeAvailable(async_logger)`:

```cmake
set(ASYNC_LOGGER_RING_BUFFER_PROVIDER PACKAGE CACHE STRING "" FORCE)
```

or

```cmake
set(ASYNC_LOGGER_RING_BUFFER_PROVIDER FETCH CACHE STRING "" FORCE)
```

or after installation:

```cmake
find_package(async_logger CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE
  async_logger::async_logger
)
```

Installed package consumption expects an installed `ring_buffer` package that
matches `ASYNC_LOGGER_RING_BUFFER_VERSION` when `async_logger` was built with
`ASYNC_LOGGER_RING_BUFFER_PROVIDER=PACKAGE`.

When `async_logger` is built with `FETCH` or with `AUTO` falling back to
`FetchContent`, `cmake --install` also installs the fetched `ring_buffer`
package into the same prefix, so `find_package(async_logger CONFIG REQUIRED)`
remains self-contained for downstream consumers.

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
  WaitStrategy wait_strategy; // worker idle strategy: Blocking or Yielding
};
```

**Wait strategies:**

* `WaitStrategy::Blocking` $\rightarrow$ default, lower idle CPU usage
* `WaitStrategy::Yielding` $\rightarrow$ lower wake-up latency at the cost of CPU noise

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
* Log entries capture their timestamp at enqueue time and are written asynchronously by a worker thread.
* If the queue is full, entries are dropped and counted via `Logger::droppedCount()`.

#### Error Handling

When file output is enabled and the log file cannot be opened, `Logger::init()`
throws `AsyncLogger::FileOpenError`.

You can inspect queue pressure after shutdown with:

```cpp
auto dropped = AsyncLogger::Logger::droppedCount();
```

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
    AsyncLogger::Config config;
    config.flag = AsyncLogger::out_stdout | AsyncLogger::out_file;
    config.wait_strategy = AsyncLogger::WaitStrategy::Blocking;

    AsyncLogger::Logger::init(config);
    AsyncLogger::Logger::info("Application started");
    AsyncLogger::Logger::shutdown();
    return 0;
}
```

## Uninstall

If you installed via CMake:

```bash
sudo cmake --build build/release --target uninstall_async_logger
```

## Contributing

Contributions are welcome! Feel free to open issues or pull requests on GitHub.
