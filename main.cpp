#include<iostream>
#include<vector>
#include<functional>
#include<map>
using namespace std;

map<string,string> helpInfoMp;

void registerHelpInfo(string command,string tips) {
    helpInfoMp[command] = tips;
}


void mainFrameShow() {
    cout << "Tips:image --help for more information" << endl;
    string flag;
    while (getline(cin,flag) && flag != "exit") {
        if (flag == "image --help") {
            for(auto helpInfo:helpInfoMp){
                cout<<helpInfo.first<<": "<<"\t"<<helpInfo.second<<endl;
            }    
        }else if(flag!=""){
            cout<<"command:"<<flag<<" not found.Try image --help for more infomation"<<endl;
        }
    }
}

void init() {
    registerHelpInfo("image --help","show help infomation");
}
signed main() {
    init();
    mainFrameShow();
    return 0;
}