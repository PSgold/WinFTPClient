#pragma once
#include "ShaiSock.h"

namespace SHAIFTP{
    ////////////////////////////FTPCLIENTCONTROL///////////////////////////////
    struct U8STRING{
        std::u8string str;

        U8STRING();
        //U8STRING(U8STRING& u8str);
        U8STRING(const std::u8string& str8);
        unsigned int resize();
    };
    
    struct DIRCHILDITEM{
        enum TYPE{file,dir};
        std::u8string path;
        //std::u8string name;
        TYPE type;

        DIRCHILDITEM();
    };
    
    class FTPCLIENTCONTROL:public SHAISOCK::CLIENTSOCK{
        std::u8string user;
        std::u8string pass;
        bool loggedIn;
        unsigned short ftpCommandLength;
        char responseCode[3]{};
        std::vector<DIRCHILDITEM> dirChildItemVec{}; 
        
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
        void login(bool anonPass=0);
        unsigned int enterPASV();
        bool enterBinaryMode();
        bool retrieveFile(const std::u8string& pathFromServer);
        bool storeFile(const std::u8string& serverPath);
        bool mkDir(std::u8string& dirPath);
        bool list(const std::u8string dirPath);
        bool listFull(const std::u8string dirPath);
        void writeListToArray();
        void features();
        void writeResponseCode(char* responseCodeToWrite);
        bool checkReponseCode(char* codeToCheck);
        char* getResponseCode();
        bool confirmFileSizeTransfer(const std::u8string& serverPath, const std::wstring& localFilePath);
        char* getRecBuff();
        bool quit();
    
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
            unsigned int recBuffSize=16384,
            unsigned int sendBuffSize=16384
        );
        ~FTPCLIENTDATA();
        void close();
        bool getFile();
        bool putFile();
        bool getChildItems(
            std::vector<SHAIFTP::U8STRING>& childItemArray,
            SHAIFTP::FTPCLIENTCONTROL& ftpClientControl
        );
        void addChildItems(
            char* buff,
            std::vector<SHAIFTP::U8STRING>& childItemArray,
            unsigned short& childItemArrayIndex
        );
        bool getChildItemsFull(
            std::vector<SHAIFTP::U8STRING>& childItemArray,
            std::vector<SHAIFTP::U8STRING>& dirChildItemArray,
            std::vector<SHAIFTP::U8STRING>& fileChildItemArray,
            unsigned int& dirCiaIndexEmpty,
            unsigned int& fileCiaIndex,
            unsigned short& startIndexOfDir,
            unsigned short& indexOfDestDir,
            const std::u8string& destination
        );        
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
        short putFile(const std::wstring localPath,const std::u8string serverPath);
        short getDir(std::u8string dirPath,std::u8string destination);
        short putDir(std::u8string& localDir, std::u8string targetPath=u8"/");
        short getFeatures();
        short makeDir(std::u8string& dirPath);
    };
    ////////////////////////////FTPCLIENT///////////////////////////////
    
    //Helper Templates
    template <typename ALLSTDSTR, typename ALLCHAR>
    void replaceChar(ALLSTDSTR& str,ALLCHAR originalChar,ALLCHAR newChar){
        for(unsigned short c{0};c<str.size();++c){
            if(str[c]==originalChar)str[c] = newChar;
        }
    }

    //Helper Functions
    void mergeStr8IntoStrW(std::u8string& str8,std::wstring& strW,const unsigned short& indexToLastDir);
    std::wstring str8ToStrW(const std::u8string& str8);
    std::u8string mergeTargetRootWithSourceExtension(
        const unsigned short& sourceSize, std::u8string source,
        const std::u8string& target
    );
    unsigned short returnIndexLastDirW(const std::wstring& strW);
    unsigned short returnIndexLastDirU8(const std::u8string& strU8);
    std::u8string returnLastDir(std::u8string dirPath);
    unsigned short returnStartIndexFromSource(std::u8string dirPath);
    enum CONSOLEMODE{utf8,utf16};
    void setConsoleMode(CONSOLEMODE mode);
    std::u8string returnLastPath(std::u8string& str8);
    unsigned short startIndexOfLastPath(std::u8string& str8);
    std::u8string returnPathFromIndex(const std::u8string& str8, const unsigned short& index);
}