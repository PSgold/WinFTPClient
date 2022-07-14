#pragma once
#include "Includes.h"

namespace SHAISOCK{
    enum ADDRESSFAMILY:int{SHAISOCK_AF_UNSPEC=0,SHAISOCK_AF_INET=2};
    enum SOCKETTYPE:int{SHAISOCK_SOCK_STREAM=1};
    enum SOCKETPROTOCOL:int{SHAISOCK_IPPROTO_TCP=6,SHAISOCK_PROVIDER_CHOICE=0};
    enum WINSOCKVERSION{One,OneOne,Two,TwoOne,TwoTwo};

/////////////////////////INITSOCK AND CLIENTSOCK////////////////////////
    class INITSOCK{
        WSADATA wsaData;
        unsigned char version[2]{};
        
        public:
        INITSOCK(WINSOCKVERSION winSockVersion);
        ~INITSOCK();
        bool checkVersion();
        float getVersion();
    };

    class CLIENTSOCK:public INITSOCK{
        std::wstring ipAddress;

        protected:
        SOCKET sock;
        sockaddr_in sockAddr;
        const unsigned int recBuffSize;
        const unsigned int sendBuffSize;
        const std::unique_ptr<char[]>recBuff;
        const std::unique_ptr<char[]>sendBuff;

        public:
        CLIENTSOCK(
            WINSOCKVERSION winSockVersion,    
            ADDRESSFAMILY addrFamily,
            SOCKETTYPE sockType,
            SOCKETPROTOCOL socProtocol,
            std::wstring ipAddress,
            unsigned int port,
            unsigned int recBuffSize,
            unsigned int sendBuffSize 
        );
        CLIENTSOCK(
            std::wstring serverDomain,
            WINSOCKVERSION winsockVersion,
            ADDRESSFAMILY addrFamily,
            SOCKETTYPE sockType,
            SOCKETPROTOCOL socProtocol,
            unsigned int port,
            unsigned int recBuffSize,
            unsigned int sendBuffSize
        );
        ~CLIENTSOCK();
        void sockSend(unsigned int buffSize=0);
        void sockRec();
        void resetRecBuff();
        void resetSendBuff();
        void printRecBuff(); 
        void printSendBuff();
        bool setRecTimeOut(unsigned int timeout); //timeout in seconds
        std::wstring getIpAddress();
    };
    /////////////////////////INITSOCK AND CLIENTSOCK////////////////////////


    //Helper Functions
    #ifdef DEBUG
    enum printType{character, base10};
    void printCharArray(const char* const buff,const printType type,const unsigned int size);
    #endif
    std::wstring resolveToIPv4(const std::wstring& domainName);
}