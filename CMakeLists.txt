# ============================================================
#  七一一便利店 POS 系统 · CMake 构建脚本
# ------------------------------------------------------------
#  用法（在项目根目录执行）：
#    cmake -G "MinGW Makefiles" -B build     # ① 生成构建文件（configure）
#    cmake --build build                     # ② 编译（调用 mingw32-make）
#    ctest --test-dir build --output-on-failure   # ③ 跑自动化测试
#
#  说明：-G "MinGW Makefiles" 是告诉 CMake"用你 MinGW 里的
#  mingw32-make 来干活"。之后重新编译只需要第 ② 步。
# ============================================================

cmake_minimum_required(VERSION 3.16)    # 最低要求的 CMake 版本

project(Qiming711 CXX)                  # 项目名 + 语言（CXX = C++）

set(CMAKE_CXX_STANDARD 17)              # 全项目用 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # 编译器不支持就报错，不许偷偷降级

# ---------------- 目标 1：主程序 ----------------
add_executable(pos project-level1.cpp)  # 由 project-level1.cpp 编译出 pos.exe

# ---------------- 目标 2：Level 2 学习注释版 ----------------
add_executable(pos2 project-level2.cpp) # 由 project-level2.cpp 编译出 pos2.exe

# 打开编译警告（必须在两个目标都定义之后加；只在 GCC/Clang 下生效）
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(pos PRIVATE -Wall -Wextra)
    target_compile_options(pos2 PRIVATE -Wall -Wextra)
endif()

# ---------------- 自动化测试（CTest） ----------------
enable_testing()                        # 打开测试功能，ctest 才能发现用例

# 每个 add_test 是一条测试：调用 cmake 执行 tests/run_tests.cmake 脚本，
# 脚本里会把输入喂给程序、检查输出对不对（集成测试）
add_test(NAME basic                     # 用例集 1：基础功能（两个版本都能跑）
         COMMAND ${CMAKE_COMMAND}
                 -DEXE=$<TARGET_FILE:pos>        # 被测程序路径（自动解析）
                 -DCASES=basic
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.cmake)

add_test(NAME admin                     # 用例集 2：管理员与库存（只测 Level 2 版）
         COMMAND ${CMAKE_COMMAND}
                 -DEXE=$<TARGET_FILE:pos2>
                 -DCASES=admin
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.cmake)
