#include<ctime>
#include<iostream>

using namespace std;
int main()
{
    time_t now = time(nullptr);
    tm* lt = localtime(&now);
    char timeStr[16];
    strftime(timeStr,sizeof(timeStr),"%H:%M:%S", lt);
    printf("%s",timeStr);
    return 0;
}