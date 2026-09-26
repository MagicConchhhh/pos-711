// ============================================================================
//  七一一便利店 POS 系统 · Level 2 完整实现（学习注释版）
// ----------------------------------------------------------------------------
//  在 Level 1（1.1 查询 / 1.2 结账 / 1.3 销售统计）的基础上新增：
//    2.1 管理员模式：admin 密码进入 / setprice 改价 / itemadd 上架新商品
//                    itemdel 下架商品 / back 退出 / setpwd 修改密码
//    2.2 库存管理：  restock 进货（+=）/ setstock 盘点（=）/ 结账自动扣库存
//
//  两个重要架构升级（Level 2 的真正难点）：
//    ① 商品表从"硬编码在代码里"升级为 items.csv 文件驱动
//       —— 否则 setprice/itemadd 改完一重启就全忘了
//    ② 新增"模式状态机"：收银员模式提示符 >   /   管理员模式提示符 admin>
//
//  阅读说明：本文件几乎每一行都有注释；配合《Level2学习报告.md》一起看效果更好
// ============================================================================

#include<iostream>      // cout / cin：输入输出
#include<string>        // std::string：字符串
#include<sstream>       // istringstream：把一行拆成词、按逗号拆列
#include<iomanip>       // setw / left / right / fixed / setprecision：排版
#include<ctime>         // time / localtime / strftime：取当前时间
#include<fstream>       // ifstream / ofstream：读写文件
#include<cstdlib>       // atoi / atof：字符串转整数/小数
using namespace std;

// ============================ 全局常量与状态 ============================

const int MAX = 100;            // 商品表、购物车数组的最大容量（超出会拦）
int itemCount = 0;              // 当前商品个数（数组从下标 1 开始用，同条码 001→1 的直觉）
int day = 1;                    // 今天是第几天（配对文件 day.txt，重启不丢）
int saleNo = 1;                 // 本笔交易的流水号（配对 sales.csv 扫描恢复）
bool isAdmin = false;           // 【2.1 新增】当前是否处于管理员模式
string adminPwd = "admin123";   // 【2.1 新增】管理员密码（配对文件 password.txt，可修改）

// ============================ 数据结构 ============================

// 商品（Node1 = 商品表的每一行）
struct Node1
{
    string code;    // 条码："001"（用 string 存，直接比较，避免字符/数字转换的坑）
    string name;    // 名称："Cola"
    double price;   // 单价：3.50
    int stock;      // 【2.2 新增】库存数量：70
}objects[MAX];

// 购物车的一行（Node2 = 某个商品的购买记录，按下标和商品表一一对应）
// 例如 shop_basket[1] 装的就是 objects[1]（Cola）买了多少
struct Node2
{
    double price;   // 加购时的单价（用于小票显示）
    int quantity;   // 数量
    double amount;  // 金额小计 = 单价 × 数量
}shop_basket[MAX];

// ====================== 文件①：商品表 items.csv ======================
// 格式：每行一个商品，四列 —— code,name,price,stock
// 例：  001,Cola,3.50,70
// 注意：这是"当前状态"文件（像一张随时被修改的表），所以用【覆盖写】，
//       和 sales.csv 的"流水记录"（追加写、只增不改）正好相反！

// 把内存里的商品表整个写回 items.csv（覆盖写）
void save_items()
{
    ofstream fout("items.csv");             // 默认模式 = 打开即清空再写（覆盖写）
    fout << fixed << setprecision(2);       // 流操纵器：价格统一写成两位小数
    for(int i = 1;i <= itemCount;i++)       // 遍历 1..itemCount 每个商品
        fout << objects[i].code << ","      // 条码
             << objects[i].name << ","      // 名称
             << objects[i].price << ","     // 单价（两位小数）
             << objects[i].stock << "\n";   // 库存
    fout.close();                           // 关闭文件，确保数据落盘
}

// 启动时加载商品表：有文件读文件；没文件（首次运行）用默认商品并立刻落盘
void load_items()
{
    ifstream fin("items.csv");              // 尝试打开商品表文件
    if(!fin)                                // 打开失败 = 还没有这个文件 = 首次运行
    {
        itemCount = 3;                      // 内置三个默认商品（库存数字参考题目 2.2 样张）
        objects[1] = {"001", "Cola",     3.50, 70};    // 聚合初始化：按字段顺序填
        objects[2] = {"002", "Lollipop", 0.50, 80};
        objects[3] = {"003", "Noodles",  6.00, 20};
        save_items();                       // 立刻写盘：从此以后一切以文件为准
        return;                             // 默认分支结束
    }
    itemCount = 0;                          // 有文件：从 0 开始数着涨（不能写死 3）
    string line;
    while(getline(fin, line))               // 逐行读（和读键盘/读字符串同一个 getline）
    {
        if(line.empty()) continue;          // 容错：文件被手改出空行也不崩
        istringstream iss(line);            // 把这一行装进字符串流
        string codeS, nameS, priceS, stockS;
        getline(iss, codeS, ',');           // 按逗号依次拆出四列（都是字符串）
        getline(iss, nameS, ',');
        getline(iss, priceS, ',');
        getline(iss, stockS, ',');
        if(codeS.empty()) continue;         // 容错：残缺行跳过
        itemCount++;                        // 数量 +1
        if(itemCount >= MAX)                // 防溢出：数组最多放 99 个商品
        {
            itemCount--;                    // 撤销这次 +1
            break;                          // 后面不再读了
        }
        objects[itemCount].code  = codeS;                       // 填条码（本来就是字符串）
        objects[itemCount].name  = nameS;                       // 填名称
        objects[itemCount].price = atof(priceS.c_str());        // "3.50" → 3.50
        objects[itemCount].stock = atoi(stockS.c_str());        // "70"   → 70
    }
}

// ====================== 文件②：管理员密码 password.txt ======================
// 只存一行密码。说明：这是"学习项目"的简化做法；
// 真实系统绝不能明文存密码，要存哈希（如 bcrypt），见学习报告的面试题

// 启动时读密码：读不到就保持默认 admin123
void load_pwd()
{
    ifstream fin("password.txt");
    string p;
    if(fin >> p) adminPwd = p;      // >> 读一个"词"（密码不含空格），失败则不动
}

// 修改密码后写回文件（覆盖写，同 items.csv 思路）
void save_pwd()
{
    ofstream fout("password.txt");
    fout << adminPwd << endl;
    fout.close();
}

// ============================ 启动状态恢复 ============================
void initialization()
{
    // ① 恢复"今天是第几天"（day.txt 只存一个数字）
    ifstream fin("day.txt");
    fin >> day;                                 // 读不到（首次运行）就保持初值 1

    // ② 恢复流水号：扫 sales.csv，找出"今天"已有的最大 No.，下一笔接着来
    int maxNo = 0;                              // 擂台：从 0 开始找最大
    ifstream fsales("sales.csv");
    string line;
    while(getline(fsales, line))                // 逐行读历史流水
    {
        istringstream iss(line);
        string dayS, noS;
        getline(iss, dayS, ',');                // 只需要前两列：day 和 No.
        getline(iss, noS, ',');
        if(atoi(dayS.c_str()) != day) continue; // 不是今天的记录，跳过
        int n = atoi(noS.c_str());
        if(n > maxNo) maxNo = n;                // 打擂台
    }
    saleNo = maxNo + 1;                         // 今天没记录 → maxNo=0 → saleNo=1 ✓

    // ③ 【Level 2】加载商品表（文件驱动，替代原来的硬编码）
    load_items();

    // ④ 【Level 2】加载管理员密码
    load_pwd();
}

// ============================ 商品查找 ============================

// 按条码查找商品，返回数组下标；找不到返回 -1
int findItem(const string& code)
{
    for(int i = 1;i <= itemCount;i++)           // 逐个比较（商品不多，线性够用）
        if(objects[i].code == code) return i;   // string 直接 == 比较
    return -1;                                  // 转完一圈没找到
}

// ============================ 商品表打印 ============================

// 打印商品表（【2.2 起】带库存列，对照样张：
//   Item     No. Pri. Stock
//   -----------------------
//   Cola     001 4.00 70        ）
void print_price()
{
    cout << setw(9) << left << "Item" << "No. Pri. Stock" << endl;  // 表头：名称列占 9 格
    cout << "-----------------------" << endl;                       // 分隔线（23 个横线）
    for(int i = 1;i <= itemCount;i++)
        cout << setw(9) << left << objects[i].name      // 名称：补到 9 格左对齐
             << objects[i].code << " "                  // 条码 + 一个空格
             << fixed << setprecision(2) << objects[i].price << " "  // 价格两位小数 + 空格
             << objects[i].stock << endl;               // 【2.2】库存列
}

// ============================ 小票打印 ============================

// 打印购物车小票（print 和 checkout 共用；不清空、不写文件，纯显示）
void print_checkout()
{
    cout << "Receipt" << endl;
    cout << setw(9) << left << "Item" << setw(5) << left << "Pri." << setw(4) << left << "Qty" << "Amount" << endl;
    cout << "-----------------------" << endl;
    double total = 0;                       // 总金额累加器
    for(int i = 1;i <= itemCount;i++)
    {
        if(shop_basket[i].quantity <= 0) continue;      // 没买的不打印
        total += shop_basket[i].amount;                 // 累加小计
        cout << setw(9) << left << objects[i].name
             << setw(5) << left << fixed << setprecision(2) << shop_basket[i].price
             << setw(4) << left << ("x" + to_string(shop_basket[i].quantity))    // "x2" 整体补到 4 格
             << "=" << shop_basket[i].amount << endl;
    }
    cout << "-----------------------" << endl;
    cout << setw(18) << left << "Total" << "=" << total << endl;    // Total 补齐 18 格后跟金额
}

// ============================ 销售流水记账 ============================

// 把当前购物车作为"一笔交易"追加写入 sales.csv（每行一条明细）
// 格式：day,No,time,name,qty,amount   —— 存的是【成交快照】：名字和金额
// 所以以后管理员改价、下架商品，都不会影响历史记录（题目 2.1 的思考题答案！）
void save_sale()
{
    time_t now = time(nullptr);             // ① 取当前时间（1970 年起的秒数）
    tm* lt = localtime(&now);               // ② 拆成年月日时分秒
    char timeStr[16];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", lt);     // ③ 格式化成 "10:15:32"

    ofstream fout("sales.csv", ios::app);   // ios::app = 追加模式：绝不覆盖历史流水
    if(!fout)                               // 打开失败（比如目录只读）要能兜住
    {
        cout << "ERROR: cannot write file" << endl;
        return;                             // 直接返回，后面不再写
    }
    fout << fixed << setprecision(2);       // 文件流也是流：7 会写成 7.00
    for(int i = 1;i <= itemCount;i++)       // 每件买过的商品写一行
    {
        if(shop_basket[i].quantity <= 0) continue;
        fout << day << "," << saleNo << "," << timeStr << ","
             << objects[i].name << "," << shop_basket[i].quantity << ","
             << shop_basket[i].amount << "\n";
    }
    fout.close();
}

// ============================ 购物车操作 ============================

// 清空购物车（drop 指令 / 结账后调用）
void drop()
{
    for(int i = 1;i <= itemCount;i++)
        shop_basket[i] = Node2();       // 整个替换成"空节点"：数量/金额/价格全归零
}

// 扫到条码 → 商品 +1（金额用加购时的单价累计）
void calculate(int idxx)
{
    shop_basket[idxx].quantity += 1;                    // 数量 +1
    shop_basket[idxx].amount += objects[idxx].price;    // 金额 += 当前单价
    shop_basket[idxx].price = objects[idxx].price;      // 记录单价（供小票显示）
}

// ============================ 新的一天 ============================

// newday：天数 +1、流水号归 1、写回 day.txt（重启不忘）
void newday()
{
    day++;                                  // 天数 +1
    saleNo = 1;                             // 新的一天，流水号从 1 重新来
    ofstream fout("day.txt");               // 覆盖写：day.txt 只存"当前是第几天"
    fout << day;
    fout.close();
    cout << "New day started. Today's sales records cleared." << endl;
}

// ============================ 销售报表 ============================

// 打印"一笔交易"的缓存内容：第一行带 No./Time，续行缩进；金额打在该笔【最后一行】
// 为什么要有这个函数：它在两处被调用（循环里打"上一笔"、循环外打"最后一笔"）
// —— 重复的逻辑抽成函数，是这个函数存在的唯一理由
void flush_sale(const string& no, const string& time, string items[], int itemCnt, double total)
{
    for(int i = 0;i < itemCnt;i++)
    {
        if(i == 0)  cout << setw(6) << left << no << setw(10) << left << time;  // 首行：编号+时间
        else        cout << setw(16) << "";                     // 续行：空 16 格缩进对齐
        cout << setw(19) << left << items[i];                   // 明细列（补到 19 格）
        if(i == itemCnt - 1)                                    // 是这一笔的最后一行
            cout << setw(5) << right << fixed << setprecision(2) << total;      // 跟上金额（右对齐）
        cout << endl;
    }
}

// sales [day]：打印某一天的销售报表
// 核心思想：读文件 → 过滤 → 按 No. 分组（"缓存一笔"解决"金额打最后一行"的前瞻问题）
void print_sales(int ask_day)
{
    cout << "Date: " << ask_day << endl;        // 对照样张："Date: 1"（单数，没有 s）
    cout << setw(6) << left << "No." << setw(10) << left << "Time" << setw(20) << left << "Items" << "Amount" << endl;
    cout << "-----------------------------------------" << endl;

    string curNo, curTime;              // 当前这一笔的 No. 和 Time（身份）
    string curItems[50];                // 当前这一笔的明细缓存（先攒着，不着急打印）
    int curCnt = 0;                     // 已缓存几条明细
    double curTotal = 0;                // 当前这一笔的金额合计
    double daily = 0;                   // 全天合计

    ifstream fin("sales.csv");          // 打开流水文件
    string line;
    while(getline(fin, line))           // 文件不存在时循环直接不进，不崩溃（空表）
    {
        istringstream iss(line);        // 每一行按逗号拆成 6 列
        string dayS, noS, timeS, nameS, qtyS, amtS;
        getline(iss, dayS, ',');
        getline(iss, noS, ',');
        getline(iss, timeS, ',');
        getline(iss, nameS, ',');
        getline(iss, qtyS, ',');
        getline(iss, amtS, ',');
        if(atoi(dayS.c_str()) != ask_day) continue;     // 过滤：不是要查的那天就跳过

        if(noS != curNo)                // No. 变了 → 说明"上一笔"已经结束了
        {
            if(curCnt > 0)              // 第一笔进来时缓存还是空的，不用打印
            {
                flush_sale(curNo, curTime, curItems, curCnt, curTotal);     // 打印上一笔
                daily += curTotal;      // 全天合计累加
            }
            curNo = noS;                // 开始缓存新的一笔：记下身份、清空缓存
            curTime = timeS;
            curCnt = 0;
            curTotal = 0;
        }
        curItems[curCnt] = nameS + " x" + qtyS;         // 明细进缓存（如 "Cola x2"）
        curCnt++;
        curTotal += atof(amtS.c_str()); // 金额累加（"3.50" → 3.50）
    }
    if(curCnt > 0)                      // ⚠️ 循环里只打了"上一笔"，最后一笔在这里补打！
    {
        flush_sale(curNo, curTime, curItems, curCnt, curTotal);
        daily += curTotal;
    }

    cout << "-----------------------------------------" << endl;
    cout << "Daily: " << fixed << setprecision(2) << daily << endl;      // 全天营业额
}

// ============================ 主程序 ============================
int main()
{
    cout << "Welcome to Qiming 711!" << endl;
    initialization();                       // 启动时恢复：天数、流水号、商品表、密码

    while(true)                             // 主循环：一直读命令，直到 quit/exit
    {
        cout << (isAdmin ? "admin> " : "> ");   // 【2.1】提示符随模式切换
        string line;
        if(!getline(cin, line)) break;          // 输入流结束（如 Ctrl+Z）时退出
        if(line == "quit" || line == "exit")    break;      // 退出指令

        istringstream iss(line);                // 把一整行拆成一个个词
        string word;
        int flag = 0;                           // 本行有没有"成功加购"（决定要不要回声）
        while(iss >> word)                      // 一行可以输入多个条码：001 003
        {
            // ---------------- 收银员/管理员都能用 ----------------

            if(word == "prices")                // 看商品表（含库存）
            {
                print_price();
                continue;                       // 处理完这个词，看下一个
            }

            if(word == "drop")                  // 清空购物车
            {
                drop();
                cout << "Cart cleared." << endl;
                continue;
            }

            if(word == "print")                 // 打小票（不清空）
            {
                print_checkout();
                continue;
            }

            if(word == "checkout")              // 结账：打小票 → 记账 → 扣库存 → 清车
            {
                print_checkout();               // 1) 打印小票
                save_sale();                    // 2) 写 sales.csv（快照）
                for(int i = 1;i <= itemCount;i++)           // 3) 【2.2】扣库存
                    if(shop_basket[i].quantity > 0)
                        objects[i].stock -= shop_basket[i].quantity;
                save_items();                   // 4) 库存变了 → 写回商品表
                drop();                         // 5) 清空购物车
                saleNo++;                       // 6) 下一笔的流水号
                continue;
            }

            if(word == "newday")                // 开新的一天
            {
                newday();
                continue;
            }

            if(word == "sales")                 // 销售报表：sales / sales 3
            {
                int x = day;                    // 默认查"今天"
                string arg;
                if(iss >> arg)                  // 后面还有词 → 用户指定了天数
                    x = atoi(arg.c_str());      // "3" → 3（atoi 转换失败返回 0，不抛异常）
                print_sales(x);
                continue;
            }

            // ---------------- 【2.1】管理员模式 ----------------

            if(word == "admin")                 // 进入管理员模式
            {
                if(isAdmin)                     // 已经在里面了
                {
                    cout << "Already in admin mode." << endl;
                    continue;
                }
                cout << "Password: ";           // 提示输入密码（对照样张）
                string pwd;
                getline(cin, pwd);              // 读"下一行"作为密码（密码不按空格拆词）
                if(pwd == adminPwd)             // 密码正确
                {
                    isAdmin = true;             // 切换模式 → 提示符也会变成 admin>
                    cout << "Admin mode." << endl;
                }
                else                            // 密码错误
                    cout << "ERROR: wrong password" << endl;
                continue;
            }

            if(word == "back")                  // 退出管理员模式
            {
                if(!isAdmin)                    // 没在管理员模式下，提示一下
                {
                    cout << "ERROR: not in admin mode" << endl;
                    continue;
                }
                isAdmin = false;                // 切回收银员模式
                cout << "Bye." << endl;         // 对照样张
                continue;
            }

            if(word == "setprice")              // 改价：setprice 001 4.00
            {
                if(!isAdmin)                    // 权限检查：管理命令必须管理员模式
                {
                    cout << "ERROR: please enter admin mode first (type: admin)" << endl;
                    continue;
                }
                string code;                    // 要改的商品条码
                double p;                       // 新价格
                if(!(iss >> code >> p))         // 从同一个流里继续读两个参数
                {
                    cout << "Usage: setprice <code> <new price>" << endl;
                    continue;
                }
                int idx = findItem(code);       // 找商品
                if(idx == -1)                   // 不存在
                {
                    cout << "ERROR: code not found" << endl;
                    continue;
                }
                objects[idx].price = p;         // 改价
                save_items();                   // 立刻落盘（重启不丢）
                cout << "Price updated." << endl;   // 对照样张
                continue;
            }

            if(word == "itemadd")               // 上架新商品：itemadd 004 Caddy 1.00
            {
                if(!isAdmin)
                {
                    cout << "ERROR: please enter admin mode first (type: admin)" << endl;
                    continue;
                }
                string code, name;              // 新商品的条码和名称（名称不含空格）
                double p;                       // 价格
                if(!(iss >> code >> name >> p)) // 三个参数都要读到
                {
                    cout << "Usage: itemadd <code> <name> <price>" << endl;
                    continue;
                }
                if(findItem(code) != -1)        // 条码不能重复
                {
                    cout << "ERROR: code already exists" << endl;
                    continue;
                }
                if(itemCount >= MAX - 1)        // 容量检查（数组最多 99 个商品）
                {
                    cout << "ERROR: item table is full" << endl;
                    continue;
                }
                itemCount++;                    // 数量 +1，新商品放在末尾
                objects[itemCount].code = code;         // 填条码
                objects[itemCount].name = name;         // 填名称
                objects[itemCount].price = p;           // 填价格
                objects[itemCount].stock = 0;           // 库存从 0 开始（之后用 restock 补）
                save_items();                   // 落盘
                cout << name << "(" << code << ") added." << endl;      // 对照样张
                continue;
            }

            if(word == "itemdel")               // 下架商品：itemdel 003
            {
                if(!isAdmin)
                {
                    cout << "ERROR: please enter admin mode first (type: admin)" << endl;
                    continue;
                }
                string code;
                if(!(iss >> code))              // 需要一个参数
                {
                    cout << "Usage: itemdel <code>" << endl;
                    continue;
                }
                int idx = findItem(code);       // 找商品
                if(idx == -1)
                {
                    cout << "ERROR: code not found" << endl;
                    continue;
                }
                string rmName = objects[idx].name;      // 先记下名字和条码（删除后就没了，
                string rmCode = objects[idx].code;      //   用来打提示）
                for(int i = idx;i < itemCount;i++)      // 从删除位置开始，把后面的元素整体前移
                {
                    objects[i]     = objects[i + 1];    // 商品表前移
                    shop_basket[i] = shop_basket[i + 1]; // ⚠️ 购物车必须【同步前移】！
                                                        // 购物车是按下标对应商品的——
                                                        // 只移商品不移购物车，就会"张冠李戴"：
                                                        // 结账时把 A 的数量算到 B 头上！
                }
                shop_basket[itemCount] = Node2();       // 清掉尾部的残留数据
                itemCount--;                    // 数量 -1
                save_items();                   // 落盘
                cout << rmName << "(" << rmCode << ") removed." << endl;    // 对照样张
                continue;
            }

            if(word == "restock")               // 进货：restock 001 50（库存 += 50）
            {
                if(!isAdmin)
                {
                    cout << "ERROR: please enter admin mode first (type: admin)" << endl;
                    continue;
                }
                string code;
                int n;                          // 进货数量
                if(!(iss >> code >> n))
                {
                    cout << "Usage: restock <code> <amount>" << endl;
                    continue;
                }
                int idx = findItem(code);
                if(idx == -1)
                {
                    cout << "ERROR: code not found" << endl;
                    continue;
                }
                objects[idx].stock += n;        // 增量：在原有库存上加
                save_items();
                cout << objects[idx].name << " stock: " << objects[idx].stock << endl;      // 反馈最新库存
                continue;
            }

            if(word == "setstock")              // 盘点：setstock 001 70（库存 = 70，直接覆盖）
            {
                if(!isAdmin)
                {
                    cout << "ERROR: please enter admin mode first (type: admin)" << endl;
                    continue;
                }
                string code;
                int n;                          // 盘出来的实际数量
                if(!(iss >> code >> n))
                {
                    cout << "Usage: setstock <code> <amount>" << endl;
                    continue;
                }
                int idx = findItem(code);
                if(idx == -1)
                {
                    cout << "ERROR: code not found" << endl;
                    continue;
                }
                objects[idx].stock = n;         // 绝对值：直接设为 n（restock 是 +=，这里是 =）
                save_items();
                cout << objects[idx].name << " stock: " << objects[idx].stock << endl;
                continue;
            }

            if(word == "setpwd")                // 修改管理员密码：setpwd <新密码>
            {
                if(!isAdmin)
                {
                    cout << "ERROR: please enter admin mode first (type: admin)" << endl;
                    continue;
                }
                string np;                      // 新密码
                if(!(iss >> np))
                {
                    cout << "Usage: setpwd <new password>" << endl;
                    continue;
                }
                adminPwd = np;                  // 更新内存
                save_pwd();                     // 写入 password.txt（重启后仍生效）
                cout << "Password updated." << endl;
                continue;
            }

            // ---------------- 购物车加减 ----------------

            if(word[0] == '-')                  // -001：减一件
            {
                int j = findItem(word.substr(1));   // 去掉减号再查（substr(1)=掐掉第一个字符）
                if(j == -1)                     // 情况一：商品根本不存在
                {
                    cout << "ERROR: code not found" << endl;
                    continue;
                }
                if(shop_basket[j].quantity <= 0)    // 情况二：商品存在，但购物车里没有
                {
                    cout << "sorry there is no such objects in your basket" << endl;
                    continue;
                }
                shop_basket[j].quantity -= 1;       // 数量 -1
                shop_basket[j].amount = objects[j].price * shop_basket[j].quantity;   // 金额重算（更准）
                cout << setw(9) << left << objects[j].name << fixed << setprecision(2) << objects[j].price << " x" << shop_basket[j].quantity << " =" << shop_basket[j].amount << endl;
                continue;
            }

            // ---------------- 扫码加购 ----------------

            int idx = findItem(word);           // 当条码查
            if(idx != -1)                       // 找到了
            {
                if(shop_basket[idx].quantity >= objects[idx].stock)     // 【2.2】库存拦截：
                {                                                       // 想买的量已经到库存上限
                    cout << "ERROR: only " << objects[idx].stock << " in stock" << endl;
                }
                else                            // 库存够
                {
                    calculate(idx);             // 加购一件
                    flag = 1;                   // 标记"本行有成功加购"
                }
            }
            else                                // 查不到这个条码
                cout << "ERROR: code not found" << endl;
        }

        // 整行处理完：把本行涉及的商品状态各打印一行（回显）
        if(flag == 1)
        {
            for(int i = 1; i <= itemCount;i++)
            {
                if(shop_basket[i].quantity > 0)     // 只显示买过的
                {
                    cout << setw(9) << left << objects[i].name << fixed << setprecision(2) << objects[i].price << " x" << shop_basket[i].quantity << " =" << shop_basket[i].amount << endl;
                }
            }
            flag = 0;                       // 复位，给下一行用
        }
    }
    return 0;
}
