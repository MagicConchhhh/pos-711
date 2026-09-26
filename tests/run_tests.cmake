# ============================================================
#  集成测试脚本（由 ctest 调用；也可以手动运行）
# ------------------------------------------------------------
#  手动运行示例：
#    cmake -DEXE=./build/pos.exe -DCASES=basic -P tests/run_tests.cmake
#
#  原理：
#    ① 把一串命令写进 input.txt（模拟用户交互输入）
#    ② execute_process 运行程序，用 INPUT_FILE 把文件接到它的标准输入
#    ③ 检查程序的输出里有没有期望的关键字 → PASS / FAIL
#  为什么要这样测：这个程序是"命令行交互式"的，最直接的自动化方式
#  就是"喂输入、查输出"（集成测试）；比逐条手工敲命令可靠得多。
# ============================================================

if(NOT DEFINED EXE)
    message(FATAL_ERROR "用法: cmake -DEXE=<被测程序> [-DCASES=basic|admin] -P tests/run_tests.cmake")
endif()
if(NOT DEFINED CASES)
    set(CASES basic)                    # 默认跑基础用例集
endif()

set(SCRATCH "${CMAKE_CURRENT_LIST_DIR}/scratch")   # 测试专用工作目录
file(REMOVE_RECURSE "${SCRATCH}")                   # 每次先清空（避免残留文件干扰）
set(FAILED 0)

# ------------------------------------------------------------
# 辅助函数：跑一个用例
#   run_case(<用例名> <stdin内容> <输出里应包含的关键字>)
#   程序会在 tests/scratch/<用例名>/ 目录里跑（它生成的
#   sales.csv / day.txt 等都留在那儿，不污染项目）
# ------------------------------------------------------------
function(run_case cname input expect)
    set(dir "${SCRATCH}/${cname}")
    file(MAKE_DIRECTORY "${dir}")
    file(WRITE "${dir}/input.txt" "${input}")

    execute_process(
        COMMAND "${EXE}"
        INPUT_FILE "${dir}/input.txt"       # 把 input.txt 接到程序的标准输入
        WORKING_DIRECTORY "${dir}"          # 在独立目录里运行
        OUTPUT_VARIABLE out                 # 收集程序输出
        TIMEOUT 10                          # 10 秒还没退出就当失败（防死循环）
        RESULT_VARIABLE rc)

    if(NOT rc EQUAL 0)
        message("  [FAIL] ${cname}：程序异常退出（返回码 ${rc}）")
        set(FAILED 1 PARENT_SCOPE)
        return()
    endif()

    string(FIND "${out}" "${expect}" idx)   # 在输出里找关键字，找不到返回 -1
    if(idx EQUAL -1)
        message("  [FAIL] ${cname}：输出中没找到 \"${expect}\"")
        message("         实际输出：\n${out}")
        set(FAILED 1 PARENT_SCOPE)
        return()
    endif()
    message("  [PASS] ${cname}")
endfunction()

# ============================ 用例集 ============================
message("运行用例集: ${CASES}   （被测程序: ${EXE}）")

if(CASES STREQUAL "basic")
    # 1.1 查询：扫一个条码，应显示商品名
    run_case(query "001\nquit\n" "Cola")

    # 1.2 结账：Cola x2 + Lollipop x1 = 7.50
    run_case(checkout "001 001 002\ncheckout\nquit\n" "=7.50")

    # 1.3 销售统计：结账后查报表，当日营业额 3.50
    run_case(report "001\ncheckout\nsales\nquit\n" "Daily: 3.50")
    # 附加检查：这笔交易要落进 sales.csv（快照格式）
    file(READ "${SCRATCH}/report/sales.csv" sales_content)
    string(FIND "${sales_content}" "Cola,1,3.50" fidx)
    if(fidx EQUAL -1)
        message("  [FAIL] sales.csv 写入：没找到 \"Cola,1,3.50\"\n         实际内容：${sales_content}")
        set(FAILED 1)
    else()
        message("  [PASS] sales.csv 写入")
    endif()

elseif(CASES STREQUAL "admin")
    # 2.1 管理员登录 + 改价
    run_case(admin_price "admin\nadmin123\nsetprice 001 4.00\nback\nquit\n" "Price updated.")

    # 2.1 密码错误应被拒
    run_case(wrong_pwd "admin\nwrongpwd\nquit\n" "ERROR: wrong password")

    # 2.1 上架新商品
    run_case(item_add "admin\nadmin123\nitemadd 004 Caddy 1.00\nprices\nback\nquit\n" "Caddy(004) added.")

    # 2.2 库存拦截：库存设成 1，扫第二件应被拦
    run_case(stock_guard "admin\nadmin123\nsetstock 001 1\nback\n001\n001\nquit\n" "only 1 in stock")

else()
    message(FATAL_ERROR "未知用例集: ${CASES}（可选 basic / admin）")
endif()

# ============================ 结果 ============================
if(FAILED)
    message(FATAL_ERROR "❌ 有用例未通过")
endif()
message("✅ 全部通过（用例集 ${CASES}）")
