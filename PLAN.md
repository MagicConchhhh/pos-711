# 七一一便利店 POS 系统 · 开发规划

> Dian 团队 2026 秋招大一题 · C++ 实现 · 最后更新：2026-09-11

---

## 一、任务规则摘要

- 仅允许 **C/C++** 开发，纯终端命令行交互，需要清晰的输入输出提示
- **gcc 一键编译**（有余力再学 CMake / make）
- 维护**学习文档**，记录开发中的问题与思路
- 保持良好的 **git 历史**与提交习惯，提交**公开**仓库链接

> 🤝 **自我约定**：Level 0-1 的代码全部自己写。AI 工具只用于：讲概念、排查报错、代码 review、出测试用例（题目要求）。

---

## 二、环境现状 ✅ 已就绪

| 项目 | 状态 |
|---|---|
| 编译器 | g++ 8.1（MinGW-W64），位于 `D:\x86_64-8.1.0-release-posix-seh-rt_v6-rev0\mingw64\bin` |
| 编译命令 | `g++ -std=c++17 -Wall -Wextra main.cpp -o main.exe` |
| C++17 | ✅ `optional` / `map` / `vector` / `iomanip` 均验证可用 |
| ⚠️ `std::filesystem` | ❌ GCC 8 的实现不完整，**禁止使用**；文件路径用普通字符串 + `fstream` 即可 |
| git | ✅ 2.55 + Credential Manager，身份已配置（Yibo） |
| 终端 | **PowerShell**（路径写 `D:\` 风格；Git Bash 才是 `/d/` 风格） |
| 网络 | GitHub 直连不稳定；git 已配置专用代理（`127.0.0.1:7890`）→ **推送时 iKuuu 必须开着** |
| 仓库 | https://github.com/MagicConchhhh/pos-711 |

**⚠️ 每次编译都要带 `-std=c++17`**（g++ 8.1 默认是 C++14）。

---

## 三、需要掌握的最小 C++ 子集（第一周）

1. `iostream`：`cin` / `cout` / `getline`
2. `string`：拼接 `+`、比较 `==`、`substr`、`.size()`
3. `vector`：`push_back`、遍历、`erase`
4. `struct` / `class`：成员变量、成员函数、构造函数
5. `fstream`：`ofstream` 追加模式（`ios::app`）、`ifstream` 逐行读取
6. `iomanip`：`setw` / `left` / `setprecision` —— 表格对齐就靠它
7. （可选）`map`、`optional`

**暂时完全不需要**：模板、智能指针、移动语义、继承多态、异常体系。遇到报错里的生词再单个查。

---

## 四、分级任务拆解

### Level 1.1 计价与输出

- 输入条码 → 显示名称与价格；`prices` 查看全表；`quit` / `exit` 退出
- 一行可输入多个条码（如 `001 003`）
- 找不到商品 → `ERROR: code not found`

**关键点**：`getline` 读整行 + `istringstream` 拆词；`setw` 对齐表格。

### Level 1.2 订单结账

- `001` 该商品 +1；`-001` 该商品 -1；`print` 打小票；`drop` 清空；`checkout` 结账并清空
- 小票含：商品、数量、单价、小计、总计

**关键点**：购物车数据结构（商品 + 数量）；数量减到 0 时的行为（**设计决策**：删除还是提示？）；`print` 与 `checkout` 的区别（一个保留一个清空）。

### Level 1.3 当日销售统计

- 每次 `checkout` 追加写入文件：流水号、时间、商品明细、总价
- `sales [day]` 查看某日销售记录与营业额（省略参数 = 今天）
- `newday` 开始新的一天

**关键点**：`<ctime>` 获取格式化时间；**"今天是第几天"必须持久化**（重启后不能忘记天数）；CSV 写入与回读。

### Level 2.1 管理员商品价格管理

- `admin` 进入管理模式（初始密码 `admin123`，可自行修改）；`back` 退出
- `setprice <条码> <新价格>`、`itemadd <条码> <名称> <价格>`、`itemdel <条码>`
- `prices` 显示含库存的商品表

**⚠️ 核心设计决策（题目埋的坑）**：改价后**过往小票不应变化** —— 销售记录里要存**成交时的价格快照**，而不是引用商品当前价格。写进学习文档！

### Level 2.2 管理员商品库存管理

- `restock <条码> <数量>`（进货，增量）、`setstock <条码> <数量>`（盘点，绝对值）
- `prices` 显示库存

**⚠️ 题目埋的坑**：加了库存后结账会出现新情况 —— 结账要扣库存；**库存不足时怎么办**（拒绝结账？提示？部分结账？）自己定义规则并写进学习文档。另：商品信息（含库存/价格）也需要持久化文件，否则重启丢失。

### Level 3 自由发挥

- 打折、组合销售、多日报表、终端图形化统计（如用 `#` 拼柱状图）……
- **原则：挑 1~2 个做扎实，胜过全都做一半。**

---

## 五、推荐架构

```
pos-711/
├── src/
│   ├── main.cpp          # 主循环：读行 → 拆词 → 分发命令
│   ├── item.hpp/.cpp     # 商品表：内存管理 + items.csv 读写
│   ├── cart.hpp/.cpp     # 购物车：加/减/打印小票
│   ├── sales.hpp/.cpp    # 结算：sales.csv 追加、查询、newday
│   └── util.hpp/.cpp     # 表格打印、时间格式化等工具
├── .gitignore            # *.exe *.o
├── PLAN.md               # 本文件
└── 学习文档.md            # 记录问题 & 设计思路（题目要求）
```

**核心设计**（先想清楚再动手）：

```cpp
struct Item { std::string code, name; double price; int stock; };
```

- **商品表**：`std::vector<Item>`（或 `map<string, Item>`），负责查找与持久化
- **购物车**：`{商品编号, 数量}` 的列表；加/减/清空/生成小票
- **销售**：checkout 时把**价格快照**写入 `sales.csv`；维护"当前是第几天"
- **命令解析**：`getline` 读整行 → `istringstream` 拆词 → 第一个词查命令表，否则当条码处理（注意 `-001` 负号前缀）

---

## 六、三周计划

| 时间 | 任务 | 里程碑（一个 commit） |
|---|---|---|
| Day 1-2 | 环境搭建 + hello world + 建仓库 | ✅ 已完成 `4d745f4` |
| Day 3-5 | **Level 1.1** 计价与输出 | `feat: 1.1 商品查询与价格表` |
| Day 6-8 | **Level 1.2** 购物车与结账 | `feat: 1.2 购物车与结账` |
| Day 9-11 | **Level 1.3** 文件持久化 + 销售统计 | `feat: 1.3 销售记录持久化` |
| Day 12-14 | **Level 2.1 + 2.2** 管理员与库存 | `feat: 2.x 管理员功能` |
| Day 15-17 | **Level 3** 挑 1~2 个亮点 | `feat: 折扣/报表 ...` |
| Day 18-20 | 打磨：边界情况、README、学习文档、检查仓库公开性 | 最终提交 |

**节奏**：边学边写，每个小功能完成就 commit，绝不攒巨型提交。

---

## 七、Git 工作流（PowerShell）

```powershell
cd D:\code\pos-711
# ……写代码、编译、测试……
git add .
git commit -m "feat: 这次改了什么"
git push
```

- **提交信息前缀**：`feat:` 新功能 / `fix:` 修 bug / `docs:` 文档 / `refactor:` 重构
- **推送前确认 iKuuu 开着**（git 走它的代理连 GitHub）
- 排错第一招：`git remote -v`（八成是地址问题）
- 查看历史：`git log --oneline`

---

## 八、学习文档素材清单（随做随记）

- 环境类：PowerShell vs Git Bash 路径差异；remote URL typo 与 `git remote -v` 排错；git 直连被墙 → 代理方案；OAuth 授权邮件
- 设计类：快照 vs 引用（历史小票为什么不随改价变化）；数量减到 0 的行为；库存不足的策略；"第几天"的持久化方案
- 语法类：`bits/stdc++.h` 为什么不是标准 C++；`std::filesystem` 在老 GCC 上的坑
- 待补充……

---

*活文档：边做边更新，面试前回看一遍。*
