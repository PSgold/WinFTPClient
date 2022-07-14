#include "Includes.h"
#include "ShaiFtp.h"

int main(int argc, char const *argv[]){
    try{
        std::wstring serverName{L"UbuntuServer"};
        //std::wstring serverName{L"192.168.56.2"};
        SHAIFTP::FTPCLIENT ftpClient{serverName,u8"testftp",u8"P@ssw0rd",SHAIFTP::DOMAINFLAG::domainFlag};
        //SHAIFTP::FTPCLIENT ftpClient{serverName,u8"testftp",u8"P@ssw0rd"};
        //short result{ftpClient.putFile(L"e:\\VirtualBox VMs\\Shared\\Windows8.1Pro.iso",u8"/Windows8.1Pro.iso")};
        short result{ftpClient.getFile(u8"/Windows8.1Pro.iso",L".\\Downloaded\\Windows8.1Pro.iso")};

        if(result!=1)std::cout<<"Error downloading: "<<result<<std::endl;
        //ftpClient.downloadFile(u8"/Pcgaming - Gaming At Its Best.pptx",L".\\Downloaded\\Pcgaming - Gaming At Its Best.pptx");
    }    
    catch(const int& error){
        std::cerr <<"Exception thrown: "<<error<<std::endl;
        return 1;
    }
    return 0;
}