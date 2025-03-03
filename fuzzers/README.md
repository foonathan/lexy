# Lexy Fuzzers

This directory contains fuzzers for testing the lexy library. The fuzzing infrastructure supports both libFuzzer and AFL++ as fuzzing engines, with various sanitizer options.

## Available Fuzzers

- **json_fuzzer**: Tests JSON parsing
- **xml_fuzzer**: Tests XML parsing
- **parse_tree_fuzzer**: Tests parse tree generation
- **visualize_fuzzer**: Tests visualization components
- **buffer_fuzzer**: Tests the internal buffer implementation

## Building the Fuzzers

### Basic Build with libFuzzer (default)

```bash
# Create and enter a build directory
mkdir -p build_fuzzer && cd build_fuzzer

# Configure with fuzzing enabled
cmake -DLEXY_BUILD_FUZZERS=ON ..

# Build all fuzzers
cmake --build . --parallel
```

### Building with AFL++

Requires AFL++ to be installed on the system

```bash
# Create and enter a build directory
mkdir -p build_afl && cd build_afl

# Configure with AFL++ (make sure you're using the afl-clang-fast compiler)
export CC=afl-clang-fast
export CXX=afl-clang-fast++
cmake -DLEXY_BUILD_FUZZERS=ON ..

# Build all fuzzers
cmake --build . --parallel
```

### Building with different sanitizers

```bash
# Create and enter a build directory
mkdir -p build_fuzzer_asan && cd build_fuzzer_asan

# Configure with Address Sanitizer (default if not specified)
export SANITIZER=address
cmake -DLEXY_BUILD_FUZZERS=ON ..

# Build all fuzzers
cmake --build . --parallel
```

### Build with static linking

```bash
# Create and enter a build directory
mkdir -p build_fuzzer_static && cd build_fuzzer_static

# Configure with static linking
cmake -DLEXY_BUILD_FUZZERS=ON -DFORCE_STATIC_LINKING=ON ..

# Build all fuzzers
cmake --build . --parallel
```

### Combining options

```bash
# Create and enter a build directory
mkdir -p build_afl_msan_static && cd build_afl_msan_static

# Configure with AFL++, memory sanitizer, and static linking
export CC=afl-clang-fast
export CXX=afl-clang-fast++
export SANITIZER=memory
cmake -DLEXY_BUILD_FUZZERS=ON -DFORCE_STATIC_LINKING=ON ..

# Build all fuzzers
cmake --build . --parallel
```
