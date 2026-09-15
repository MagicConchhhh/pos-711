#include<iostream>
#include<string>
#include<sstream>
#include<iomanip>
using namespace std;
const int MAX = 100;
int itemCount = 0;
struct Node
{
    string code;    // 条码："001"（原来存 int，字符和数字混着算容易出错，直接存字符串比较更稳）
    string name;
    double price;
}objects[MAX];

void initialization()
{
    itemCount = 3;
    objects[1].code = "001",objects[1].name = "Cola",objects[1].price = 3.50;
    objects[2].code = "002",objects[2].name = "Lollipop",objects[2].price = 0.50;
    objects[3].code = "003",objects[3].name = "Noodles",objects[3].price = 6.00;
}

// 按条码查找商品，返回数组下标；找不到返回 -1
int findItem(const string& code)
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

int main()
{
    cout << "欢迎光临启明711" << endl;
    initialization();
    while(true)
    {
        cout << "> ";
        string line;
        if(!getline(cin, line)) break;          // 输入流结束（如 Ctrl+Z）时退出，防止死循环
        if(line == "quit" || line == "exit")    break;

        istringstream iss(line);                // 把一整行拆成一个个词
        string word;
        while(iss >> word)                      // 一行可以输入多个条码：001 003
        {
            if(word == "prices")
            {
                print_price();
                continue;
            }
            int idx = findItem(word);
            if(idx != -1)
                cout << setw(9) << left << objects[idx].name << fixed << setprecision(2) << objects[idx].price << endl;
            else
                cout << "ERROR: code not found" << endl;
        }
    }
    return 0;
}
