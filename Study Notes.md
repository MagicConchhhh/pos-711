# Study Notes

## Day1 2026/9/11

因为我曾经学过信息学竞赛自认为对C++更为熟悉，所以我决定使用C++完成这份项目，同时也是对大学内容的一个预习，在暑假的时候我配置了我的VS Code并了解了关于git的有关内容，在Claude的帮助下，我上传了我的第一份代码进入我的仓库，接下来我将开始level 1

了解了powershell与gitbash的区别

## Day2 2026/9/12

$$\left( 1 + \frac{x}{n} \right)^n \geqslant 1 + x$$

## Day3 2026/9/15

### 📖 概念笔记：istringstream（字符串输入流）

#### 一句话定义

`istringstream` 是一个"**从字符串里读字**"的工具，用起来和 `cin` 一模一样，只是数据来源不是键盘，而是一个字符串。

名字拆解：

- **i** = input（输入）
- **string** = 字符串
- **stream** = 流（数据像水流，只能向前流动）

它住在 `<sstream>` 头文件里。

#### 和 cin 的类比

```cpp
string word;
cin >> word;                 // 从【键盘】读一个词（遇到空格/回车停下）
```

```cpp
istringstream iss(line);     // 把字符串 line 包装成一个"流"
string word;
iss >> word;                 // 从【字符串 line】里读一个词
```

`>>` 的行为完全一样：**跳过空白、读一个词、遇到空格停下**。区别只是数据来源不同。

#### 为什么项目里需要它

`getline` 拿到的是**一整行**，比如 `"001 003"`（两个条码）。我们需要把它切成一个个"词"，分别去查商品表。

用 `istringstream` 可以像剥洋葱一样，一个词一个词地取出来——**这也是对之前 `line[2]` 按字符位置取巧写法的替代方案**（那个写法遇到 `'1'` 和数字 `1` 混淆、越界等问题）。

#### 基本用法

```cpp
#include <sstream>

string line = "001 003";        // 假装这是 getline 读到的
istringstream iss(line);        // 装进"字符串流"
string word;
while (iss >> word) {           // 每转一圈拿出一个词
    cout << "拿到: " << word << endl;
}
```

运行结果：

```
拿到: 001
拿到: 003
```

**`while (iss >> word)` 怎么知道啥时候停？**
每次 `>>` 读一个词，读到字符串末尾没词可读了，循环条件自动变假、循环结束。不用手动判断长度。

#### 在项目主循环中的应用（骨架）

```cpp
getline(cin, line);              // 读一整行
istringstream iss(line);         // 包装成流
string word;
while (iss >> word) {            // 逐个拿词
    // 对每个 word 判断：
    //   是 quit / exit → 退出
    //   是 prices      → 打印商品表
    //   否则           → 当成条码，查商品表
}
```

好处：拿到的是**完整的词**，可以直接和商品条码做字符串比较，不用再关心"第几个字符"。

#### ⚠️ 三个注意事项

1. **只按空白分割**：空格、Tab 都行，连续多个空格会自动跳过（`"001    003"` 没问题）；但**逗号不行**，`"001,003"` 会被当成一个词。
2. **读过的词就没了**：流是单向消费的，读一个少一个。想重新从头读，最简单的方法是**重新构造一个 `istringstream`**。
3. **它还能直接读数字**（以后有用）：例如管理员的 `setprice 001 4.00`，可以把 `4.00` 直接从流里读成 `double`，不用自己手动转换。

#### 相关类型（知道有即可）

| 类型 | 用途 |
|---|---|
| `istringstream` | 从字符串**读**（本项目要用的） |
| `ostringstream` | 往字符串**写**（拼接格式化字符串用） |
| `stringstream` | 既能读也能写 |

#### 🏋️ 动手练习

- [ ] 把 demo_c 的第一行改成 `string line; getline(cin, line);`——变成"输入什么就拆什么"
- [ ] 测试：`001 003 002`（三个词）、`001    003`（多个空格）、`   001`（前导空格）
- [ ] 把拆出来的词接到商品查找逻辑上，修复"一行多个条码"的问题

### 🐛 今天测试踩的坑（实测记录）

| # | 坑 | 现象 | 原因 | 解决 |
|---|---|---|---|---|
| 1 | `#include<struct>` | 编译失败：`fatal error: struct: No such file or directory` | `struct` 是**关键字**不是头文件；代码用了 `string` 却忘了包含它 | 改为 `#include<string>`，删掉用不到的 `<cstring>` |
| 2 | 字符 `'1'` ≠ 数字 `1` | 输入任何条码都输出 `ERROR: code not found` | `line[2]` 是 char，`'1'` 在内存里的值是 **49**（ASCII）；`49 <= 3` 永远为假，全部掉进 else | `line[2] - '0'` 转成数字；编译器其实用 `-Wall` 警告了：`array subscript has type 'char'` |
| 3 | 越界访问 | 输入空行时读 `line[2]`，属于**未定义行为**（没崩溃是运气） | 没检查字符串长度 | 访问前判断 `line.size() >= 3` |
| 4 | 输出格式 | 价格显示 `3.5` 不是 `3.50`；商品名列没对齐 | 没用 `<iomanip>` | `fixed << setprecision(2)`；`setw(9) << left` |

> 我的排查过程补充（待写）：
> （提示：第 2 条其实我自己跑的时候完全没发现——是 Claude 实际编译测试后指出来的。想想为什么"看起来没问题"的代码实际全错？）

### 📌 下一步 TODO

- [ ] 修复：`#include<struct>` → `#include<string>`
- [ ] 修复：条码查找（字符转数字；或改用字符串比较）
- [ ] 修复：一行多个条码（用 istringstream 拆词）
- [ ] 修复：输出格式（`setw` 对齐 + 两位小数）
- [ ] 思考：条码改用 `string` 存储、循环比较查找（为 1.2 购物车做准备）

## Day4 2026/9/16

✅ 进度：1.1 已修复完成并提交（commit `a2eefaa`）——上面的 TODO 全部完成：条码改为 `string` 存储 + `findItem()` 字符串查找、`istringstream` 拆词支持多条码、`setw` 表格对齐。初版代码已归档到 `archive/project-level1-v1.cpp`。

### 📖 概念笔记：setw 与表格对齐

#### 核心思想

表格对齐 = **让每一行的同一列"占一样宽"**。

`setw(n)` 的工作只有一句话：**本次输出不足 n 个字符，就补空格凑够**（setw = set width 的缩写）。

#### 实验：把填充字符换成 `.`，让补齐肉眼可见

```cpp
cout << setw(9) << left << "Cola"     << "|"
     << setw(9) << left << "Lollipop" << "|"
     << setw(9) << left << "Noodles"  << "|" << endl;
```

输出：

```
Cola.....|Lollipop.|Noodles..|
```

- `Cola`（4 字符）补 5 个 → 9 格
- `Lollipop`（8）补 1 个 → 9 格
- `Noodles`（7）补 2 个 → 9 格

**每列都刚好 9 格宽，后面的竖线（下一列）自然对齐。**

#### ⚠️ 最重要的特性：setw 只管"下一个"输出

```cpp
cout << setw(9) << "Cola" << "Lollipop" << endl;
// 输出：Cola     Lollipop   ← 只有 Cola 被补齐，Lollipop 没被管
```

#### 粘性对照表（为什么有的写一遍、有的每行都要写）

| 操纵器 | 作用 | 持续生效？ |
|---|---|---|
| `setw(n)` | 下一个输出的最小宽度 | ❌ 只管下一个 |
| `left` / `right` | 对齐方向（默认右对齐） | ✅ 持续 |
| `setfill(c)` | 填充字符（默认空格） | ✅ 持续 |
| `fixed` / `setprecision(n)` | 小数格式 | ✅ 持续 |

→ 这解释了为什么：`setw(9)` 每行都要重新写，而 `fixed << setprecision(2)` 在循环里只写一次却一直生效。

#### printf 对照表（信息学竞赛知识迁移）

| printf 写法 | C++ 流写法 |
|---|---|
| `%-9s`（左对齐补到 9） | `setw(9) << left << s` |
| `%9s`（右对齐补到 9） | `setw(9) << s` |
| `%.2f`（两位小数） | `fixed << setprecision(2) << d` |
| `%9.2f`（补位 + 小数） | `setw(9) << fixed << setprecision(2) << d` |

#### 项目中的实际应用

```cpp
cout << setw(9) << left << objects[i].name          // 名字列：补到 9 格
     << objects[i].code << " "                       // 条码列
     << fixed << setprecision(2) << objects[i].price // 价格列：两位小数
     << endl;
```

**列宽为什么选 9？** 最长名字 `Lollipop` 是 8 个字符，**列宽 = 最长内容 + 1~2 格间距**。

#### ⚠️ 边界提醒

`setw(9)` 是"**最小**宽度"，不是最大——名字超过 9 字符（比如以后加的 `Watermelon`）会直接溢出、顶歪后面的列，**不会被截断**。

### 📖 补充：istringstream 的三种用法模式

#### 模式 1️⃣ 拆词（项目主循环在用）

```cpp
istringstream iss(line);
string word;
while (iss >> word) { /* 每次循环取一个词，取完自动结束 */ }
```

#### 模式 2️⃣ 混合解析（命令 + 数字参数，2.x 会用）

```cpp
istringstream cmd("restock 001 50");
string op, code;
int amount;
cmd >> op >> code >> amount;    // 字符串读进 string，数字直接读成 int
```

#### 模式 3️⃣ 自定义分隔符（1.3 读 CSV 用）

```cpp
istringstream csv("001,Cola,3.50");
string field;
while (getline(csv, field, ',')) { /* 读到逗号切一刀 */ }
```

#### 心智模型：盒子 + 游标

`istringstream` = 装着若干词的盒子 + 一个记住"读到哪了"的游标：

- **每写一次 `>>` 只取一个词**（不是"一次性全读出来"！），取完游标前进
- 取完后再取 → 失败（条件为假）
- `while (iss >> word)` 的巧妙：`iss >> word` 既取了一个词、又返回"成功了吗"给 while 判断——失败即结束循环
