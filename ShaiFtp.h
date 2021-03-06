#pragma once
#include "ShaiSock.h"

namespace SHAIFTP{
    ////////////////////////////FTPCLIENTCONTROL///////////////////////////////
    class FTPCLIENTCONTROL:public SHAISOCK::CLIENTSOCK{
        std::u8string user;
        std::u8string pass;
        bool loggedIn;
        unsigned short ftpCommandLength;
        char responseCode[3]{};
        
        public:
        FTPCLIENTCONTROL(
            SHAISOCK::WINSOCKVERSION winSockVersion,
            SHAISOCK::ADDRESSFAMILY addrFamily,
            SHAISOCK::SOCKETTYPE sockType,
            SHAISOCK::SOCKETPROTOCOL socProtocol,
            std::wstring ipAddress,
            unsigned int port,
            unsigned int recBuffSize=1024,
            unsigned int sendBuffSize=1024
        );
        FTPCLIENTCONTROL(
            std::wstring serverDomain,
            SHAISOCK::WINSOCKVERSION winSockVersion,
            SHAISOCK::ADDRESSFAMILY addrFamily,
            SHAISOCK::SOCKETTYPE sockType,
            SHAISOCK::SOCKETPROTOCOL socProtocol,
            unsigned int port,
            unsigned int recBuffSize=1024,
            unsigned int sendBuffSize=1024
        );
        FTPCLIENTCONTROL(
            SHAISOCK::WINSOCKVERSION winSockVersion,
            SHAISOCK::ADDRESSFAMILY addrFamily,
            SHAISOCK::SOCKETTYPE sockType,
            SHAISOCK::SOCKETPROTOCOL socProtocol,
            std::wstring ipAddress,
            unsigned int port,
            std::u8string user,
            std::u8string pass,
            unsigned int recBuffSize=1024,
            unsigned int sendBuffSize=1024
        );
        FTPCLIENTCONTROL(
            std::wstring serverDomain,
            SHAISOCK::WINSOCKVERSION winSockVersion,
            SHAISOCK::ADDRESSFAMILY addrFamily,
            SHAISOCK::SOCKETTYPE sockType,
            SHAISOCK::SOCKETPROTOCOL socProtocol,
            unsigned int port,
            std::u8string user,
            std::u8string pass,
            unsigned int recBuffSize=1024,
            unsigned int sendBuffSize=1024
        );
        ~FTPCLIENTCONTROL();
        void setFtpCommand(std::u8string command);
        unsigned short getFtpCommandLength();
        void login();
        unsigned int enterPASV();
        void enterBinaryMode();
        void writeResponseCode(char* responseCodeToWrite);
        bool checkReponseCode(char* codeToCheck);
        char* getResponseCode();
        bool confirmFileSizeTransfer(const std::u8string& serverPath, const std::wstring& localFilePath);
        char* getRecBuff();
    
        private:
        static bool compareStrToCharStr(std::string str,char* charStr);
    };
    ////////////////////////////FTPCLIENTCONTROL///////////////////////////////
    

    ////////////////////////////FTPCLIENTDATA///////////////////////////////
    class FTPCLIENTDATA:public SHAISOCK::CLIENTSOCK{
        std::wstring localPath;
        public:
        FTPCLIENTDATA(
            SHAISOCK::WINSOCKVERSION winSockVersion,
            SHAISOCK::ADDRESSFAMILY addrFamily,
            SHAISOCK::SOCKETTYPE sockType,
            SHAISOCK::SOCKETPROTOCOL socProtocol,
            const std::wstring ipAddress,
            unsigned int port,
            const std::wstring& localPath,
            unsigned int recBuffSize=1024,
            unsigned int sendBuffSize=1024
        );
        ~FTPCLIENTDATA();
        void close();
        bool getFile();
        bool putFile();
        void setLocalPath(std::wstring& localPath);
        void printRecBuffBase10();
    };
    ////////////////////////////FTPCLIENTDATA///////////////////////////////


    ////////////////////////////FTPCLIENT///////////////////////////////
    enum DOMAINFLAG{domainFlag};
    class FTPCLIENT{
        FTPCLIENTCONTROL ftpClientControl;
        SHAISOCK::WINSOCKVERSION version;
        const std::u8string pathToFile;

        public:
        FTPCLIENT(const std::wstring serverIpAddress,unsigned int port=21);
        FTPCLIENT(const std::wstring domain,DOMAINFLAG flag,unsigned int port=21);
        FTPCLIENT(
            const std::wstring serverIpAddress,
            std::u8string user, 
            std::u8string pass, 
            unsigned int port=21
        );
        FTPCLIENT(
            const std::wstring domain,
            std::u8string user,
            std::u8string pass,
            DOMAINFLAG flag,
            unsigned int port=21
        );
        ~FTPCLIENT();
        short getFile(const std::u8string pathFromServer,const std::wstring localTargetPath);
        short putFile(const std::wstring localPath, const std::u8string serverPath);
    };
    ////////////////////////////FTPCLIENT///////////////////////////////
    
}