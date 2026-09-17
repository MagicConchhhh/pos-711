#include<iostream>
#include<string>
#include<sstream>
#include<iomanip>
using namespace std;
const int MAX = 100;
int itemCount = 0;
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
    itemCount = 3;
    objects[1].code = "001",objects[1].name = "Cola",objects[1].price = 3.50;
    objects[2].code = "002",objects[2].name = "Lollipop",objects[2].price = 0.50;
    objects[3].code = "003",objects[3].name = "Noodles",objects[3].price = 6.00;
}

// 按条码查找商品，返回数组下标；找不到返回 -1
int findItem(const string& code) //或许可以用二分查找
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
                drop();                 // 结账后清空购物车
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
