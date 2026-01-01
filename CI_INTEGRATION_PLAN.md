# CI Integration Plan for Static Analysis and Runtime Diagnostics

**Created**: 2026-01-01
**Related Issue**: #168 (Full Audit)
**Related PR**: #169

## Overview

This document outlines the plan for integrating advanced static analysis, sanitizers, and coverage tools into the CI pipeline for the StoryGraph project.

---

## 1. Current CI State

The project currently has:
- CMake-based build system
- Ninja generator support
- Catch2 test framework (FetchContent)
- Basic build and test jobs

---

## 2. Proposed CI Architecture

### 2.1 Job Matrix

| Job Name | Trigger | Duration | Purpose |
|----------|---------|----------|---------|
| `build-and-test` | Every PR | ~3 min | Basic compilation and unit tests |
| `static-analysis` | Every PR | ~5 min | clang-tidy + cppcheck |
| `sanitizers` | Every PR | ~5 min | ASan + UBSan |
| `tsan` | Nightly | ~10 min | ThreadSanitizer |
| `valgrind` | Nightly | ~15 min | Memory leak detection |
| `coverage` | Weekly/Manual | ~8 min | Code coverage report |

### 2.2 Workflow Files

#### `.github/workflows/static-analysis.yml`

```yaml
name: Static Analysis

on:
  pull_request:
    branches: [main]
  push:
    branches: [main]

jobs:
  clang-tidy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake ninja-build clang clang-tidy \
            libsdl2-dev qtbase5-dev qtmultimedia5-dev qttools5-dev

      - name: Configure with compile_commands.json
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DNOVELMIND_BUILD_TESTS=ON \
            -DNOVELMIND_BUILD_EDITOR=ON

      - name: Run clang-tidy
        run: |
          # Run on changed files only for PRs
          if [ -n "${{ github.base_ref }}" ]; then
            git diff --name-only ${{ github.base_ref }}...HEAD \
              | grep -E '\.(cpp|hpp)$' \
              | xargs -r clang-tidy -p build \
                --checks='bugprone-*,performance-*,concurrency-*' \
                --warnings-as-errors='bugprone-*'
          else
            find engine_core editor -name '*.cpp' \
              | xargs clang-tidy -p build \
                --checks='bugprone-*,performance-*,concurrency-*'
          fi

  cppcheck:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install cppcheck
        run: sudo apt-get install -y cppcheck

      - name: Run cppcheck
        run: |
          cppcheck --enable=warning,performance,portability \
            --error-exitcode=1 \
            --suppress=missingIncludeSystem \
            --suppress=unmatchedSuppression \
            -I engine_core/include \
            engine_core/src editor/src
```

#### `.github/workflows/sanitizers.yml`

```yaml
name: Sanitizers

on:
  pull_request:
    branches: [main]
  push:
    branches: [main]

jobs:
  asan-ubsan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake ninja-build clang \
            libsdl2-dev qtbase5-dev qtmultimedia5-dev qttools5-dev

      - name: Build with ASan + UBSan
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_C_COMPILER=clang \
            -DCMAKE_CXX_COMPILER=clang++ \
            -DCMAKE_BUILD_TYPE=Debug \
            -DNOVELMIND_BUILD_TESTS=ON \
            -DNOVELMIND_BUILD_EDITOR=OFF \
            -DNOVELMIND_ENABLE_ASAN=ON \
            -DCMAKE_CXX_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer"
          cmake --build build

      - name: Run tests with sanitizers
        run: |
          cd build
          ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 \
          UBSAN_OPTIONS=halt_on_error=1 \
          ctest --output-on-failure
```

#### `.github/workflows/nightly.yml`

```yaml
name: Nightly Checks

on:
  schedule:
    - cron: '0 3 * * *'  # 3 AM UTC daily
  workflow_dispatch:

jobs:
  tsan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake ninja-build clang \
            libsdl2-dev qtbase5-dev qtmultimedia5-dev qttools5-dev

      - name: Build with TSan
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_C_COMPILER=clang \
            -DCMAKE_CXX_COMPILER=clang++ \
            -DCMAKE_BUILD_TYPE=Debug \
            -DNOVELMIND_BUILD_TESTS=ON \
            -DNOVELMIND_BUILD_EDITOR=OFF \
            -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer" \
            -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"
          cmake --build build

      - name: Run tests with TSan
        run: |
          cd build
          TSAN_OPTIONS=halt_on_error=1 ctest --output-on-failure

  valgrind:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake ninja-build valgrind \
            libsdl2-dev qtbase5-dev qtmultimedia5-dev qttools5-dev

      - name: Build for Valgrind
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_BUILD_TYPE=Debug \
            -DNOVELMIND_BUILD_TESTS=ON \
            -DNOVELMIND_BUILD_EDITOR=OFF
          cmake --build build

      - name: Run Valgrind memcheck
        run: |
          cd build
          valgrind --leak-check=full \
            --show-leak-kinds=definite,possible \
            --error-exitcode=1 \
            ./bin/unit_tests "[lexer]" "[parser]" "[fuzzing]" "[audio]"
```

#### `.github/workflows/coverage.yml`

```yaml
name: Coverage

on:
  push:
    branches: [main]
  workflow_dispatch:

jobs:
  coverage:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake ninja-build gcovr lcov \
            libsdl2-dev qtbase5-dev qtmultimedia5-dev qttools5-dev

      - name: Build with coverage
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_BUILD_TYPE=Debug \
            -DNOVELMIND_BUILD_TESTS=ON \
            -DNOVELMIND_BUILD_EDITOR=OFF \
            -DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
            -DCMAKE_EXE_LINKER_FLAGS="--coverage"
          cmake --build build

      - name: Run tests
        run: cd build && ctest --output-on-failure

      - name: Generate coverage report
        run: |
          gcovr --root . --filter 'engine_core/.*' \
            --exclude 'build/.*' \
            --html --html-details \
            -o coverage.html

      - name: Upload coverage artifact
        uses: actions/upload-artifact@v4
        with:
          name: coverage-report
          path: coverage.html
```

---

## 3. CMake Configuration Updates

Add the following to the root `CMakeLists.txt`:

```cmake
# Export compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ASan support option
option(NOVELMIND_ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(NOVELMIND_ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

# Coverage support option
option(NOVELMIND_ENABLE_COVERAGE "Enable coverage instrumentation" OFF)
if(NOVELMIND_ENABLE_COVERAGE)
  add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
  add_link_options(--coverage)
endif()
```

---

## 4. Implementation Priority

### Phase 1: Immediate (Week 1)
- [ ] Add `static-analysis.yml` with clang-tidy and cppcheck
- [ ] Update CMakeLists.txt with `CMAKE_EXPORT_COMPILE_COMMANDS`

### Phase 2: Short-term (Week 2-3)
- [ ] Add `sanitizers.yml` with ASan + UBSan
- [ ] Add NOVELMIND_ENABLE_ASAN option to CMake

### Phase 3: Medium-term (Month 1)
- [ ] Add `nightly.yml` with TSan and Valgrind
- [ ] Set up nightly schedule

### Phase 4: Long-term (Month 2+)
- [ ] Add `coverage.yml` with gcovr
- [ ] Integrate with Codecov or Coveralls
- [ ] Set coverage thresholds (target: 70%)

---

## 5. Estimated CI Runtime

| Workflow | Estimated Time | Cost Impact |
|----------|----------------|-------------|
| static-analysis | 5-8 min | Low |
| sanitizers | 5-7 min | Low |
| nightly (TSan + Valgrind) | 15-25 min | Low (runs once/day) |
| coverage | 8-12 min | Low (runs on main only) |

**Total PR check time**: ~10-15 min (parallel jobs)
**Nightly overhead**: ~25 min (separate from PR checks)

---

## 6. Success Metrics

1. **No regressions**: All sanitizer jobs pass on every PR
2. **Static analysis clean**: Zero new clang-tidy bugprone warnings
3. **Memory safety**: Valgrind reports no leaks in nightly runs
4. **Coverage growth**: Maintain or improve coverage percentage
5. **Thread safety**: TSan detects no data races

---

*This plan is part of the Deep Audit deliverables for issue #168*
