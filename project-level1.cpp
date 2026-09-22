#include<iostream>
#include<string>
#include<sstream>
#include<iomanip>
#include<ctime>
#include<fstream>
#include<cstdlib>
using namespace std;
const int MAX = 100;
int itemCount = 0;
int day = 1;
int saleNo = 1;     // 本笔交易的流水号（第一笔 = 1）
struct Node1
{
    string code;    // 条码："001"（原来存 int，字符和数字混着算容易出错，直接存字符串比较更稳）
    string name;
    double price;
}objects[MAX];

struct Node2
{                 
    //string name;
    double price;
    int quantity;
    double amount;
}shop_basket[MAX];

void initialization()
{
    ifstream fin("day.txt");
    fin >> day;                                 // ① 先恢复"今天是第几天"

    // ② 再恢复流水号：扫 sales.csv，找出"今天"已有的最大 No.，下一笔接着来
    int maxNo = 0;
    ifstream fsales("sales.csv");
    string line;
    while(getline(fsales, line))
    {
        istringstream iss(line);
        string dayS, noS;
        getline(iss, dayS, ',');
        getline(iss, noS, ',');
        if(atoi(dayS.c_str()) != day) continue;     // 不是今天的记录，跳过
        int n = atoi(noS.c_str());
        if(n > maxNo) maxNo = n;
    }
    saleNo = maxNo + 1;                         // 今天还没记录 → maxNo=0 → saleNo=1 ✓

    itemCount = 3;
    objects[1].code = "001",objects[1].name = "Cola",objects[1].price = 3.50;
    objects[2].code = "002",objects[2].name = "Lollipop",objects[2].price = 0.50;
    objects[3].code = "003",objects[3].name = "Noodles",objects[3].price = 6.00;
}

// 按条码查找商品，返回数组下标；找不到返回 -1
int findItem(const string& code) //或许可以用二分查找,应该用不上
{
    for(int i = 1;i <= itemCount;i++)
        if(objects[i].code == code) return i;
    return -1;
}

void print_price()
{
    cout << setw(9) << left << "Item" << "No. Pri." << endl;
    cout << "-----------------" << endl;
    for(int i = 1;i <= itemCount;i++)
        cout << setw(9) << left << objects[i].name << objects[i].code << " " << fixed << setprecision(2) << objects[i].price << endl;
}

void print_checkout()
{
    cout << "Receipt" << endl;
    cout << setw(9) << left << "Item" << setw(5) << left << "Pri." << setw(4) << left << "Qty" << "Amount" << endl;
    cout << "-----------------------" << endl;
    double total = 0;
    for(int i = 1;i <= itemCount;i++)
    {
        if(shop_basket[i].quantity <= 0) continue;      // 没买的不打印
        total += shop_basket[i].amount;
        cout << setw(9) << left << objects[i].name
             << setw(5) << left << fixed << setprecision(2) << shop_basket[i].price
             << setw(4) << left << ("x" + to_string(shop_basket[i].quantity))
             << "=" << shop_basket[i].amount << endl;
    }
    cout << "-----------------------" << endl;
    cout << setw(18) << left << "Total" << "=" << total << endl;
}

void save_sale()
{
    time_t now = time(nullptr);
    tm* lt = localtime(&now);
    char timeStr[16];
    strftime(timeStr,sizeof(timeStr),"%H:%M:%S", lt);

    ofstream fout("sales.csv", ios::app);
    if(!fout) 
    {
        cout << "ERROR: cannot write file" << endl;
         return;
    }
    fout << fixed << setprecision(2);       // 文件流也是流：7 会写成 7.00
    for(int i = 1;i <= itemCount;i++)       // 每件买过的商品写一行：day,No,time,name,qty,amount
    {
        if(shop_basket[i].quantity <= 0) continue;
        fout << day << "," << saleNo << "," << timeStr << ","
             << objects[i].name << "," << shop_basket[i].quantity << ","
             << shop_basket[i].amount << "\n";
    }
    fout.close();
}

void drop()//shop_basket全改为0
{
    for(int i = 1;i <= itemCount;i++)
        shop_basket[i] = Node2();       // 全部重置：数量/金额/价格归零，名字清空
}

void calculate(int idxx)
{
    shop_basket[idxx].quantity += 1;
    shop_basket[idxx].amount += objects[idxx].price;
    shop_basket[idxx].price = objects[idxx].price;
}

void newday()
{
    cout << "New day started. Today's sales records cleared." << endl;
    day++;
    saleNo = 1;
    ofstream fout("day.txt");
    fout << day;
    fout.close();
    //cout << "New day started. Today's sales records cleared." << endl;
}

// 打印一笔交易：第一行带 No./Time，续行缩进；金额打在该笔【最后一行】
void flush_sale(const string& no, const string& time, string items[], int itemCnt, double total)
{
    for(int i = 0;i < itemCnt;i++)
    {
        if(i == 0)  cout << setw(6) << left << no << setw(10) << left << time;
        else        cout << setw(16) << "";                     // 续行：缩进 16 格对齐
        cout << setw(19) << left << items[i];                   // 明细列
        if(i == itemCnt - 1)                                    // 最后一行 → 跟上金额
            cout << setw(5) << right << fixed << setprecision(2) << total;
        cout << endl;
    }
}

void print_sales(int ask_day)
{
    cout << "Date: " << ask_day << endl;        // 对照样张："Date: 1"（单数，没有 s）
    cout << setw(6) << left << "No." << setw(10) << left << "Time" << setw(20) << left << "Items" << "Amount" << endl;
    cout << "-----------------------------------------" << endl;

    // Part 3：读文件 + 过滤 + 按 No. 分组（把每一笔"缓存"起来，等到它结束才打印）
    string curNo, curTime;              // 当前这笔的 No. 和 Time
    string curItems[50];                // 当前这笔的明细缓存（最多 50 条）
    int curCnt = 0;                     // 已缓存几条明细
    double curTotal = 0;                // 当前这笔的金额合计
    double daily = 0;                   // 全天合计

    ifstream fin("sales.csv");
    string line;
    while(getline(fin, line))                   // 文件不存在时循环直接不进，不崩溃
    {
        istringstream iss(line);
        string dayS, noS, timeS, nameS, qtyS, amtS;
        getline(iss, dayS, ',');                // 按逗号一列一列拆（模式 3 实战）
        getline(iss, noS, ',');
        getline(iss, timeS, ',');
        getline(iss, nameS, ',');
        getline(iss, qtyS, ',');
        getline(iss, amtS, ',');
        if(atoi(dayS.c_str()) != ask_day) continue;     // 过滤：不是要查的那天就跳过

        if(noS != curNo)                    // No. 变了 → 说明"上一笔"已经结束了
        {
            if(curCnt > 0)                  // 第一笔进来时缓存还是空的，不用打印
            {
                flush_sale(curNo, curTime, curItems, curCnt, curTotal);
                daily += curTotal;
            }
            curNo = noS;                    // 开始缓存新的一笔
            curTime = timeS;
            curCnt = 0;
            curTotal = 0;
        }
        curItems[curCnt] = nameS + " x" + qtyS;         // 明细进缓存
        curCnt++;
        curTotal += atof(amtS.c_str());     // 金额累加（atof：字符串→小数）
    }
    if(curCnt > 0)                          // ⚠️ 循环结束，最后一笔还没打印！
    {
        flush_sale(curNo, curTime, curItems, curCnt, curTotal);
        daily += curTotal;
    }

    cout << "-----------------------------------------" << endl;
    cout << "Daily: " << fixed << setprecision(2) << daily << endl;
     

}

int main()
{
    cout << "Welcome to Qiming 711!" << endl;
    initialization();
    while(true)
    {
        cout << "> ";
        string line;
        if(!getline(cin, line)) break;          // 输入流结束（如 Ctrl+Z）时退出，防止死循环
        if(line == "quit" || line == "exit")    break;

        istringstream iss(line);                // 把一整行拆成一个个词
        string word;
        int flag = 0;
        while(iss >> word)                      // 一行可以输入多个条码：001 003
        {

            if(word == "prices")
            {
                print_price();
                continue;
            }

            if(word == "drop")
            {
                drop();
                cout << "Cart cleared." << endl;
                continue;
            }

            if(word == "print")
            {
                print_checkout();
                continue;
            }

            if(word == "checkout")
            {
                print_checkout();
                save_sale();
                drop();                 // 结账后清空购物车
                saleNo++;
                continue;
            }

            if(word == "newday")
            {
                newday();
                continue;
            }

            if(word == "sales")
            {
                int x = day;
                string plus_information;
                if(iss >> plus_information)
                    x = atoi(plus_information.c_str());   // string 不能减 '0'，atoi 才是"字符串→整数"
                print_sales(x);
                continue;
            }

            if(word[0] == '-')
            {
                int j = findItem(word.substr(1));       // "-001" → 去掉减号查 "001"（和加法分支同款正规查找）
                if(j == -1)                             // 情况一：商品根本不存在
                {
                    cout << "ERROR: code not found" << endl;
                    continue;
                }
                if(shop_basket[j].quantity <= 0)        // 情况二：商品存在，但购物车里没有
                {
                    cout << "sorry there is no such objects in your basket" << endl;
                    continue;
                }
                shop_basket[j].quantity -= 1;
                shop_basket[j].amount = objects[j].price * shop_basket[j].quantity;
                //flag = 1;
                cout << setw(9) << left << objects[j].name << fixed << setprecision(2) << objects[j].price << " x" << shop_basket[j].quantity << " =" << shop_basket[j].amount << endl;
                continue;
            }
            int idx = findItem(word);
            if(idx != -1)
            {
                calculate(idx);
                flag = 1;
            }
            //cout << setw(9) << left << objects[idx].name << fixed << setprecision(2) << objects[idx].price << endl;
            else
                cout << "ERROR: code not found" << endl;
        }
        if(flag == 1)
        {
            for(int i = 1; i <= itemCount;i++)
            {
                if(shop_basket[i].quantity > 0)
                {
                    cout << setw(9) << left << objects[i].name << fixed << setprecision(2) << objects[i].price << " x" << shop_basket[i].quantity << " =" << shop_basket[i].amount << endl;
                }
            }
            flag = 0;
        }
    }
    return 0;
}
