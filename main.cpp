#include <iostream>
#include "Regex.h"
using namespace std;

int main()
{
    while(true)
    {
        string str1,str2;
        cin>>str1;
        cin>>str2;
        Regex re(str1);
     //  cout<<regex_match(str2, re)<<endl;
       vector<string> result;
//        cout<<regex_search(str2, re)<<endl;
        regex_search(str2, result,re);
        for(const auto& e: result)
            cout<<e<<endl;
    }
    return 0;
}
