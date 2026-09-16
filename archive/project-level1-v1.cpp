#include<iostream>
#include<struct>
#include<cstring>
using namespace std;
const int MAX = 100;
int sum = 0;
struct Node
{
    int id;
    string name;
    double price;
}objects[MAX];

void initialization()
{
    sum = 3;
    objects[1].id = 1,objects[1].name = "Cola",objects[1].price = 3.50;
    objects[2].id = 2,objects[2].name = "Lollipop",objects[2].price = 0.50;
    objects[3].id = 3,objects[3].name = "Noodles",objects[3].price = 6.00;
}

void print_price()
{
    cout << "Item     No. Pri." << endl;
    cout << "-----------------" << endl;
    for(int i = 1;i <= sum;i++)
        cout << objects[i].name << " " << "00" << objects[i].id << " " << objects[i].price << endl;
}

int main()
{
    cout << "欢迎光临启明711" << endl;
    initialization();
    while(true)
    {
        cout << "> ";
        string line;
        getline(cin, line);
        if(line == "quit" || line == "exit")    break;
        else if(line == "prices") print_price();
        else if(line[2]>= 1 && line[2] <= sum)cout << objects[line[2]].name << " " << objects[line[2]].price << endl;
        else cout << "ERROR: code not found" << endl;
    }
    return 0;
}
