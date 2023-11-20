#include "ShaiFtp.h"
//Exceptions thrown on construction
//20 - login failure

//Other Exceptions
//21 - Failed to enter passive mode 


/////////////////////////////CUSTOM-HELPER-STRUCTS//////////////////////////////
SHAIFTP::U8STRING::U8STRING():str{std::u8string(100,u8'\0')}{}
//SHAIFTP::U8STRING::U8STRING(U8STRING& u8str):str{u8str.str}{}
SHAIFTP::U8STRING::U8STRING(const std::u8string& str8):str{str8}{}

unsigned int SHAIFTP::U8STRING::resize(){
    std::cout<<"String size: "<<str.size();
    unsigned int subtract{static_cast<unsigned int>(str.size()-1)};
    while(str[subtract]=='\0')--subtract;
    str.resize(subtract+1);
    std::cout<<" ; String size: "<<str.size()<<std::endl;
    return 1;
}

std::ostream& operator<<(std::ostream &os, SHAIFTP::U8STRING& str){
    os<<reinterpret_cast<char*>(str.str.data()); return os;
}

SHAIFTP::DIRCHILDITEM::DIRCHILDITEM():
path{std::u8string(100,u8'\0')}{}
/////////////////////////////CUSTOM-HELPER-STRUCTS//////////////////////////////


/////////////////////////////FTPCLIENTCONRTOL//////////////////////////////
SHAIFTP::FTPCLIENTCONTROL::FTPCLIENTCONTROL(
    SHAISOCK::WINSOCKVERSION winSockVersion,
    SHAISOCK::ADDRESSFAMILY addrFamily,
    SHAISOCK::SOCKETTYPE sockType,
    SHAISOCK::SOCKETPROTOCOL socProtocol,
    std::wstring ipAddress,
    unsigned int port,
    unsigned int recBuffSize,
    unsigned int sendBuffSize
):SHAISOCK::CLIENTSOCK(
    winSockVersion,
    addrFamily,
    sockType,
    socProtocol,
    ipAddress,
    port,
    recBuffSize,
    sendBuffSize
),loggedIn{0}{
    #ifdef DEBUG 
    std::cout<<"Constructed FTPCLIENTControlSock"<<std::endl;
    #endif    
}
SHAIFTP::FTPCLIENTCONTROL::FTPCLIENTCONTROL(
    std::wstring serverDomain,
    SHAISOCK::WINSOCKVERSION winSockVersion,
    SHAISOCK::ADDRESSFAMILY addrFamily,
    SHAISOCK::SOCKETTYPE sockType,
    SHAISOCK::SOCKETPROTOCOL socProtocol,
    unsigned int port,
    unsigned int recBuffSize,
    unsigned int sendBuffSize
):SHAISOCK::CLIENTSOCK(
    serverDomain,
    winSockVersion,
    addrFamily,
    sockType,
    socProtocol,
    port,
    recBuffSize,
    sendBuffSize
),loggedIn{0}{
    #ifdef DEBUG 
    std::cout<<"Constructed FTPCLIENTControlSock"<<std::endl;
    #endif
}
SHAIFTP::FTPCLIENTCONTROL::FTPCLIENTCONTROL(
    SHAISOCK::WINSOCKVERSION winSockVersion,
    SHAISOCK::ADDRESSFAMILY addrFamily,
    SHAISOCK::SOCKETTYPE sockType,
    SHAISOCK::SOCKETPROTOCOL socProtocol,
    std::wstring ipAddress,
    unsigned int port,
    std::u8string user,
    std::u8string pass,
    unsigned int recBuffSize,
    unsigned int sendBuffSize
):SHAISOCK::CLIENTSOCK(
    winSockVersion,
    addrFamily,
    sockType,
    socProtocol,
    ipAddress,
    port,
    recBuffSize,
    sendBuffSize
),loggedIn{0},user{user},pass{pass}{
    #ifdef DEBUG 
    std::cout<<"Constructed FTPCLIENTControlSock"<<std::endl;
    #endif    
}
SHAIFTP::FTPCLIENTCONTROL::FTPCLIENTCONTROL(
    std::wstring serverDomain,
    SHAISOCK::WINSOCKVERSION winSockVersion,
    SHAISOCK::ADDRESSFAMILY addrFamily,
    SHAISOCK::SOCKETTYPE sockType,
    SHAISOCK::SOCKETPROTOCOL socProtocol,
    unsigned int port,
    std::u8string user,
    std::u8string pass,
    unsigned int recBuffSize,
    unsigned int sendBuffSize
):SHAISOCK::CLIENTSOCK(
    serverDomain,
    winSockVersion,
    addrFamily,
    sockType,
    socProtocol,
    port,
    recBuffSize,
    sendBuffSize
),loggedIn{0},user{user},pass{pass}{
    #ifdef DEBUG 
    std::cout<<"Constructed FTPCLIENTControlSock"<<std::endl;
    #endif
}
SHAIFTP::FTPCLIENTCONTROL::~FTPCLIENTCONTROL(){
    quit();
    #ifdef DEBUG
    std::cout<<"Destructed FTPCLIENTControlSock"<<std::endl;
    #endif   
}

void SHAIFTP::FTPCLIENTCONTROL::setFtpCommand(std::u8string command){
    unsigned short index{0};
    while(index<command.length()){
        sendBuff[index] = command[index];++index;
    }
    sendBuff[index]='\r';sendBuff[index+1]='\n';
    ftpCommandLength = (command.length()+2);
    #ifdef DEBUG
    std::cout<<sendBuff.get()<<"Command Length: "<<ftpCommandLength<<std::endl;
    #endif
}

unsigned short SHAIFTP::FTPCLIENTCONTROL::getFtpCommandLength(){return ftpCommandLength;}

void SHAIFTP::FTPCLIENTCONTROL::login(bool anonPass){
    if(user!= u8"anonymous" || (user==u8"anonymous" && anonPass)){
        std::u8string command{u8"USER "+user};
        this->setFtpCommand(command);
        this->sockSend(ftpCommandLength);
        this->sockRec();
        this->printRecBuff();
        this->resetRecBuff();
        command = u8"PASS "+pass;
        this->setFtpCommand(command);
        this->sockSend(ftpCommandLength);
    }
    else{
        std::u8string command{u8"USER anonymous"};
        this->setFtpCommand(command);
        this->sockSend(ftpCommandLength);
    }
    this->sockRec();
    this->printRecBuff();
    writeResponseCode(recBuff.get());
    this->resetRecBuff();
    #ifdef DEBUG
    std::cout.write(responseCode,3);
    std::cout<<std::endl;
    #endif
    char code[]{"230"};
    if(!checkReponseCode(code)){
        exception.code=20;exception.errorStr="Login error";
        throw exception;
    }
}

unsigned int SHAIFTP::FTPCLIENTCONTROL::enterPASV(){
    resetRecBuff();
    setFtpCommand(u8"PASV");
    sockSend(ftpCommandLength);
    sockRec();
    
    #ifdef DEBUG
    printRecBuff();
    #endif
    #ifdef BUFFBYTES
    SHAISOCK::printCharArray(this->recBuff.get(),SHAISOCK::base10,100);
    #endif

    writeResponseCode(recBuff.get());
    std::string code{"227"};
    if(!checkReponseCode(code.data())){
        exception.code=21;exception.errorStr="Enter passive mode error";
        throw exception;
    }
    
    std::string portPart1(8,'\0');
    std::string portPart2(8,'\0');
    unsigned short part1Index{0};
    unsigned short part2Index{0};
    unsigned short commaNum{0};
    for(unsigned int c{26};c<recBuffSize;++c){
        if (commaNum<4){
            if (recBuff[c]==',')++commaNum;
            continue;
        }
        else if (commaNum==4){
            if(recBuff[c]==','){++commaNum;continue;}
            portPart1[part1Index] = recBuff[c];
            ++part1Index;
        }
        else{
            if(recBuff[c]==')')break;
            portPart2[part2Index] = recBuff[c];
            ++part2Index;
        }
    }
    
    unsigned long part1Int{std::stoul(portPart1)};
    unsigned long part2Int{std::stoul(portPart2)};
    unsigned int port{static_cast<unsigned short>(((part1Int*256)+part2Int))};
    #ifdef DEBUG
    std::cout<<"\nPort: "<<port<<std::endl;
    #endif
    resetRecBuff();
    return port;
}
bool SHAIFTP::FTPCLIENTCONTROL::enterBinaryMode(){
    resetRecBuff();
    std::u8string command {u8"TYPE I"};
    setFtpCommand(command);
    sockSend(ftpCommandLength);
    sockRec();
    writeResponseCode(getRecBuff());
    #ifdef DEBUG
    printRecBuff();
    #endif
    resetRecBuff();
    std::string code{"200"};
    return checkReponseCode(code.data());
}
bool SHAIFTP::FTPCLIENTCONTROL::retrieveFile(const std::u8string& pathFromServer){
    resetRecBuff();
    const std::u8string command{u8"RETR "+pathFromServer};
    setFtpCommand(command);
    sockSend(ftpCommandLength);
    sockRec();
    writeResponseCode(getRecBuff());
    std::string code{"150"};
    #ifdef DEBUG
    printRecBuff();
    #endif
    resetRecBuff();
    return checkReponseCode(code.data());
}
bool SHAIFTP::FTPCLIENTCONTROL::storeFile(const std::u8string& serverPath){
    resetRecBuff();
    const std::u8string command{(u8"STOR "+serverPath)};
    setFtpCommand(command);
    sockSend(ftpCommandLength);
    sockRec();
    writeResponseCode(recBuff.get());
    std::string code{"150"};
    #ifdef DEBUG
    printRecBuff();
    #endif
    resetRecBuff();
    return checkReponseCode(code.data());
}

bool SHAIFTP::FTPCLIENTCONTROL::mkDir(std::u8string& dirPath){
    resetRecBuff();
    const std::u8string command{u8"MKD "+dirPath};
    setFtpCommand(command);
    sockSend(ftpCommandLength);
    sockRec();
    writeResponseCode(recBuff.get());
    std::string code{"257"};
    #ifdef DEBUG
    printRecBuff();
    #endif
    resetRecBuff();
    return checkReponseCode(code.data());
}

bool SHAIFTP::FTPCLIENTCONTROL::list(const std::u8string dirPath){
    std::cout<<"TESTING BUFF OUT: "<<reinterpret_cast<const char*>(dirPath.data())<<'\n';
    resetRecBuff();
    //std::u8string command{u8"LIST "+dirPath};
    std::u8string command{u8"NLST "+dirPath};
    setFtpCommand(command);
    sockSend(ftpCommandLength);
    sockRec();
    writeResponseCode(recBuff.get());
    #ifdef DEBUG
    printRecBuff();
    #endif
    resetRecBuff();
}

bool SHAIFTP::FTPCLIENTCONTROL::listFull(const std::u8string dirPath){
    resetRecBuff();
    std::u8string command{u8"LIST "+dirPath};
    //std::u8string command{u8"NLST "+dirPath};
    setFtpCommand(command);
    sockSend(ftpCommandLength);
    sockRec();
    writeResponseCode(recBuff.get());
    #ifdef DEBUG
    printRecBuff();
    #endif
    resetRecBuff();
}

void SHAIFTP::FTPCLIENTCONTROL::writeListToArray(){
    std::unique_ptr<char[]>buffer{new char[MAX_PATH]{}};
    for(unsigned short c{0};recBuff[c]!='\r';++c){
        if(c>MAXCHILDITEMS)break;
        while(recBuff[c]!='\n'){}
    }
}

void SHAIFTP::FTPCLIENTCONTROL::features(){
    resetRecBuff();
    std::u8string command{u8"FEAT"};
    setFtpCommand(command);
    sockSend();
    sockRec();
    #ifdef DEBUG
    printRecBuff();
    #endif
    resetRecBuff();
}

bool SHAIFTP::FTPCLIENTCONTROL::quit(){
    resetRecBuff();
    std::u8string command{u8"QUIT"};
    setFtpCommand(command);
    sockSend(getFtpCommandLength());
    sockRec();
    writeResponseCode(recBuff.get());
    #ifdef DEBUG
    printRecBuff();
    #endif
    std::string code{"221"};
    return checkReponseCode(code.data());
}

void SHAIFTP::FTPCLIENTCONTROL::writeResponseCode(char* responseCodeToWrite){
    for(unsigned short c{0};c<3;++c)responseCode[c]=responseCodeToWrite[c];
}
bool SHAIFTP::FTPCLIENTCONTROL::checkReponseCode(char* codeToCheck){
    bool isEqual{1};
    for(unsigned short c{0};c<3;++c){if(responseCode[c]!=codeToCheck[c]){isEqual=0;break;}}
    return isEqual;
}
char* SHAIFTP::FTPCLIENTCONTROL::getResponseCode(){return responseCode;}

bool SHAIFTP::FTPCLIENTCONTROL::confirmFileSizeTransfer(const std::u8string& serverPath, const std::wstring& localFilePath){
    resetRecBuff();
    unsigned long long localFileSize{};
    try{localFileSize = std::filesystem::file_size(localFilePath);}
    catch(std::filesystem::filesystem_error& e) {return 0;}
    std::u8string command {u8"SIZE "+serverPath};
    setFtpCommand(command);
    sockSend(ftpCommandLength);
    sockRec();
    writeResponseCode(recBuff.get());
    char serverFileSize[21]{};unsigned short index{0};
    for(unsigned short c{4};((recBuff.get()[c]!='\r')&&(recBuff.get()[c]!='\n'));++c){
        serverFileSize[index]=recBuff.get()[c];++index;
    }
    std::string localFileSizeStr{std::to_string(localFileSize)};
    #ifdef DEBUG
    std::cout<<"Server file size "<<serverFileSize<<"\nLocal file size  "<<localFileSizeStr<<std::endl;
    #endif
    resetRecBuff();
    return SHAIFTP::FTPCLIENTCONTROL::compareStrToCharStr(localFileSizeStr,serverFileSize);
}
bool SHAIFTP::FTPCLIENTCONTROL::compareStrToCharStr(std::string str,char* charStr){
    unsigned short index{0};
    while(index<str.size()){
        if(str[index]!=charStr[index])return 0;
        ++index;
    }
    if(charStr[index]=='\0')return 1;
    else return 0;
}

char* SHAIFTP::FTPCLIENTCONTROL::getRecBuff(){return recBuff.get();}
/////////////////////////////FTPCLIENTCONRTOL//////////////////////////////


/////////////////////////////FTPCLIENTDATA//////////////////////////////
SHAIFTP::FTPCLIENTDATA::FTPCLIENTDATA(
    SHAISOCK::WINSOCKVERSION winSockVersion,
    SHAISOCK::ADDRESSFAMILY addrFamily,
    SHAISOCK::SOCKETTYPE sockType,
    SHAISOCK::SOCKETPROTOCOL socProtocol,
    const std::wstring ipAddress,
    unsigned int port,
    const std::wstring& localPath,
    unsigned int recBuffSize,
    unsigned int sendBuffSize
):SHAISOCK::CLIENTSOCK(
    winSockVersion,
    addrFamily,
    sockType,
    socProtocol,
    ipAddress,
    port,
    recBuffSize,
    sendBuffSize
),localPath{localPath}{
    #ifdef DEBUG 
    std::cout<<"Constructed FTPCLIENTDataSock"<<std::endl;
    #endif
}
SHAIFTP::FTPCLIENTDATA::~FTPCLIENTDATA(){
    #ifdef DEBUG 
    std::cout<<"Destructed FTPCLIENTDataSock"<<std::endl;
    #endif
}

void SHAIFTP::FTPCLIENTDATA::close(){this->~FTPCLIENTDATA();}
void SHAIFTP::FTPCLIENTDATA::setLocalPath(std::wstring& localPath){
    this->localPath = localPath;
}

bool SHAIFTP::FTPCLIENTDATA::getFile(){
    #ifdef DEBUG
    std::cout<<"Downloading file"<<std::endl;
    #endif
    std::ofstream oFileStream{localPath,std::ios::binary};
    if(!oFileStream.is_open()){
        #ifdef DEBUG
        std::cout<<"Failed to open local file stream"<<std::endl;
        #endif
        return 0;
    }
    int bytesRec;
    do{
        bytesRec = recv(sock,recBuff.get(),recBuffSize,0);
        oFileStream.write(recBuff.get(),bytesRec);
    } 
    while (bytesRec>=1);
    oFileStream.close();
    this->resetRecBuff();
    return 1;
}

bool SHAIFTP::FTPCLIENTDATA::putFile(){
    #ifdef DEBUG
    std::cout<<"Uploading file"<<std::endl;
    #endif
    std::ifstream iFileStream{localPath,std::ios::binary|std::ios::ate};
    if(!iFileStream.is_open()) {
        #ifdef DEBUG
        std::cout<<"Failed to open local file"<<std::endl;
        #endif
        return 0;
    }
    unsigned long long fileSize{static_cast<unsigned long long>(iFileStream.tellg())};
    #ifdef DEBUG
    std::cout<<"File size: "<<fileSize<<std::endl;
    #endif
    iFileStream.seekg(std::ios::beg,std::ios::beg);
    #ifdef DEBUG
    std::cout<<"Reset position to: "<<iFileStream.tellg()<<std::endl;
    #endif
    unsigned long long bytesRead{0};
    while(bytesRead<fileSize){
        iFileStream.read(sendBuff.get(),sendBuffSize);
        unsigned int charsRead{static_cast<unsigned int>(iFileStream.gcount())};    
        sockSend(charsRead);
        bytesRead+=charsRead;
    }
    #ifdef DEBUG
    std::cout<<"Bytes read: "<<bytesRead<<std::endl;
    #endif
    iFileStream.close();
    return 1;
}

bool SHAIFTP::FTPCLIENTDATA::getChildItems(
    std::vector<SHAIFTP::U8STRING>& childItemArray,
    SHAIFTP::FTPCLIENTCONTROL& ftpClientControl
){
    #ifdef DEBUG
    std::cout<<"Getting child items"<<std::endl;
    #endif
    int bytesRec;
    unsigned short arrayIndex{0};
    bool pass1{1};
    do{
        bytesRec = recv(sock,recBuff.get(),recBuffSize,0);
        std::cout<<"Bytes Received: "<<bytesRec<<std::endl;
        if((bytesRec<1)&&(pass1)){
            std::cout<<"PASS ONE"<<std::endl;
            ftpClientControl.sockRec();
            return 0;
        }
        if(bytesRec<1)break;
        addChildItems(recBuff.get(),childItemArray,arrayIndex);
        #ifdef DEBUG
        printRecBuff();
        #endif
        #ifdef BUFFBYTES
        SHAISOCK::printCharArray(recBuff.get(),SHAISOCK::base10,recBuffSize);
        #endif
        resetRecBuff();
        pass1=0;
    }
    while (1);
    resetRecBuff();
    childItemArray.resize(arrayIndex);
    ftpClientControl.sockRec();
    ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
    #ifdef DEBUG
    ftpClientControl.printRecBuff();
    #endif
    std::string code{"226"};
    return ftpClientControl.checkReponseCode(code.data());
}

void SHAIFTP::FTPCLIENTDATA::addChildItems(
    char* buff,
    std::vector<SHAIFTP::U8STRING>& childItemArray,
    unsigned short& childItemArrayIndex
){
    for(unsigned c{0};buff[c]!='\0'&&c<recBuffSize;c+=2){
        SHAIFTP::U8STRING childItem;
        unsigned short childItemPathIndex{0};
        while(buff[c]!='\r'&&buff[c]!='\n'){
            if(childItemPathIndex<100){
                childItem.str.data()[childItemPathIndex]=buff[c];
                ++childItemPathIndex;++c;
            }
            else{
                childItem.str.push_back(buff[c]);
                ++c;
            }
        }
        if(childItemArrayIndex<50){childItemArray[childItemArrayIndex] = childItem;++childItemArrayIndex;}
        else {childItemArray.push_back(childItem);++childItemArrayIndex;}
    }
}

bool SHAIFTP::FTPCLIENTDATA::getChildItemsFull(
    std::vector<SHAIFTP::U8STRING>& childItemArray,
    std::vector<SHAIFTP::U8STRING>& dirChildItemArray,
    std::vector<SHAIFTP::U8STRING>& fileChildItemArray,
    unsigned int& dirCiaIndexEmpty,
    unsigned int& fileCiaIndex,
    unsigned short& startIndexOfDir,
    unsigned short& indexOfDestDir,
    const std::u8string& destination
){
   #ifdef DEBUG
    std::cout<<"Getting directory child items full list"<<std::endl;
    #endif

    int bytesRec;
    unsigned int ciaIndex{0};
    do{
        bytesRec = recv(sock,recBuff.get(),recBuffSize,0);
        if(bytesRec<1)break;
        printRecBuff();
        #ifdef BUFFBYTES
        SHAISOCK::printCharArray(recBuff.get(),SHAISOCK::base10,recBuffSize);
        #endif
        bool afterNewLine{0};
        for(unsigned int c{0};c<recBuffSize;++c){
            if(recBuff[c]==0)break;
            if(c==0 || afterNewLine==1){
                if (recBuff[c]=='d'){
                    if(dirCiaIndexEmpty<25){
                        dirChildItemArray[dirCiaIndexEmpty].str = 
                        childItemArray[ciaIndex].str;
                        std::cout<<reinterpret_cast<char*>(dirChildItemArray[dirCiaIndexEmpty].str.data())<<std::endl;
                    }
                    else{
                        dirChildItemArray.push_back(childItemArray[ciaIndex].str);
                    }
                    ++dirCiaIndexEmpty;
                }
                else if(recBuff[c]=='-'){
                    if(fileCiaIndex<50){
                        fileChildItemArray[fileCiaIndex].str = childItemArray[ciaIndex].str;
                        fileChildItemArray[fileCiaIndex].resize();
                    }
                    else{
                        fileChildItemArray.push_back(childItemArray[ciaIndex]);
                        fileChildItemArray[fileCiaIndex].resize();
                    }
                    ++fileCiaIndex;
                }
                ++ciaIndex;
            }
            if(recBuff[c]=='\n')afterNewLine=1;
            else if(recBuff[c]!='\n')afterNewLine=0;
        }
        resetRecBuff();
    }
    while (1);
    
    resetRecBuff();
    return 1;       
}

void SHAIFTP::FTPCLIENTDATA::printRecBuffBase10(){
    SHAISOCK::printCharArray(this->recBuff.get(),SHAISOCK::base10,recBuffSize);
}
/////////////////////////////FTPCLIENTDATA//////////////////////////////


/////////////////////////////FTPCLIENT//////////////////////////////
SHAIFTP::FTPCLIENT::FTPCLIENT(const std::wstring serverIpAddress, unsigned int port):
version{SHAISOCK::WINSOCKVERSION::TwoTwo},
ftpClientControl{
    SHAISOCK::WINSOCKVERSION::TwoTwo,
    SHAISOCK::ADDRESSFAMILY::SHAISOCK_AF_INET,
    SHAISOCK::SOCKETTYPE::SHAISOCK_SOCK_STREAM,
    SHAISOCK::SOCKETPROTOCOL::SHAISOCK_IPPROTO_TCP,
    serverIpAddress,
    port,
    1024,
    50
}{
    ftpClientControl.sockRec();
    #ifdef DEBUG
    std::cout<<"Constructed FTPCLIENT"<<std::endl;
    ftpClientControl.printRecBuff();
    #endif
    ftpClientControl.resetRecBuff();
    ftpClientControl.login();
}
SHAIFTP::FTPCLIENT::FTPCLIENT(const std::wstring domain,DOMAINFLAG flag,unsigned int port):
version{SHAISOCK::WINSOCKVERSION::TwoTwo},
ftpClientControl{
    domain,
    SHAISOCK::WINSOCKVERSION::TwoTwo,
    SHAISOCK::ADDRESSFAMILY::SHAISOCK_AF_INET,
    SHAISOCK::SOCKETTYPE::SHAISOCK_SOCK_STREAM,
    SHAISOCK::SOCKETPROTOCOL::SHAISOCK_IPPROTO_TCP,
    port,
    1024,
    50
}{
    #ifdef DEBUG
    std::cout<<"Constructed FTPCLIENT"<<std::endl;
    ftpClientControl.sockRec();
    ftpClientControl.printRecBuff();
    ftpClientControl.resetRecBuff();
    #endif
    ftpClientControl.login();
}
SHAIFTP::FTPCLIENT::FTPCLIENT(
    const std::wstring serverIpAddress,
    std::u8string user,
    std::u8string pass, 
    unsigned int port
):version{SHAISOCK::WINSOCKVERSION::TwoTwo},
ftpClientControl{
    SHAISOCK::WINSOCKVERSION::TwoTwo,
    SHAISOCK::ADDRESSFAMILY::SHAISOCK_AF_INET,
    SHAISOCK::SOCKETTYPE::SHAISOCK_SOCK_STREAM,
    SHAISOCK::SOCKETPROTOCOL::SHAISOCK_IPPROTO_TCP,
    serverIpAddress,
    port,user,pass,1024,50
}{
    ftpClientControl.sockRec();
    #ifdef DEBUG
    std::cout<<"Constructed FTPCLIENT"<<std::endl;
    ftpClientControl.printRecBuff();
    #endif
    ftpClientControl.resetRecBuff();
    ftpClientControl.login(1);
}
SHAIFTP::FTPCLIENT::FTPCLIENT(
    const std::wstring domain,
    std::u8string user,
    std::u8string pass,
    DOMAINFLAG flag,
    unsigned int port
):version{SHAISOCK::WINSOCKVERSION::TwoTwo},
ftpClientControl{
    domain,
    SHAISOCK::WINSOCKVERSION::TwoTwo,
    SHAISOCK::ADDRESSFAMILY::SHAISOCK_AF_INET,
    SHAISOCK::SOCKETTYPE::SHAISOCK_SOCK_STREAM,
    SHAISOCK::SOCKETPROTOCOL::SHAISOCK_IPPROTO_TCP,
    port,user,pass,1024,50
}{
    ftpClientControl.sockRec();
    #ifdef DEBUG
    std::cout<<"Constructed FTPCLIENT"<<std::endl;
    ftpClientControl.printRecBuff();
    #endif
    ftpClientControl.resetRecBuff();
    ftpClientControl.login(1);
}
SHAIFTP::FTPCLIENT::~FTPCLIENT(){
    /* std::u8string command{u8"QUIT"};
    ftpClientControl.setFtpCommand(command);
    ftpClientControl.sockSend(ftpClientControl.getFtpCommandLength());
    ftpClientControl.sockRec();
    ftpClientControl.printRecBuff(); */
    #ifdef DEBUG
    std::cout<<"Destructed FTPCLIENT"<<std::endl;
    #endif
}

//Errors
//-1 - Failed to enter Binary mode
//-2 - Failed to enter PASV
//-3 - Failed to retrieve file
//-4 - Failed to complete file transfer
//-5 - Failed file size comparison
//Success
//1
short SHAIFTP::FTPCLIENT::getFile(
    const std::u8string pathFromServer,
    const std::wstring localTargetPath
){ 
    //enter binary mode
    if(!ftpClientControl.enterBinaryMode())return -1;

    //enter passive mode
    unsigned int port;
    try{port = ftpClientControl.enterPASV();}
    catch(const SHAISOCK::EXCEPTION& error){
        #ifdef DEBUG
        std::cerr <<"Exception thrown: "<<error.code<<" ; "<<error.errorStr<<std::endl;
        #endif
        return -2;
    }

    
    //create data socket connection
    FTPCLIENTDATA ftpClientData{
        version,
        SHAISOCK::ADDRESSFAMILY::SHAISOCK_AF_INET,
        SHAISOCK::SOCKETTYPE::SHAISOCK_SOCK_STREAM,
        SHAISOCK::SOCKETPROTOCOL::SHAISOCK_IPPROTO_TCP,
        ftpClientControl.getIpAddress(),
        port,
        localTargetPath,
        16777216,
        8,
    };

    
    //command to retrieve file
    if(!ftpClientControl.retrieveFile(pathFromServer))return -3;
    
    //download file
    ftpClientData.getFile();
    
    //check for successful transfer
    ftpClientControl.sockRec();
    ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
    #ifdef DEBUG
    ftpClientControl.printRecBuff();
    #endif
    ftpClientControl.resetRecBuff();
    std::string code{"226"};
    if(!ftpClientControl.checkReponseCode(code.data()))return -4;

    if(!ftpClientControl.confirmFileSizeTransfer(pathFromServer,localTargetPath)){
        #ifdef DEBUG
        std::cout<<"File sizes don't match"<<std::endl;
        #endif    
        return -5;
    }
    #ifdef DEBUG
    std::cout<<"File sizes match"<<std::endl;
    #endif
    return 1;
}

//Errors
//-1 - Failed to enter Binary mode
//-2 - Failed to enter PASV
//-3 - Failed to prepare for opening data connection
//-4 - Failed to open local file
//-5 - Failed to complete file transfer
//-6 - Failed file size comparison
//Success
//1
short SHAIFTP::FTPCLIENT::putFile(
    const std::wstring localPath, 
    const std::u8string serverPath
){
    //std::string code{"150"};
    bool success{1};
    //ftpclientdata in its own scope to destruct at correct time
    {   
        //enter binary mode
        if(!ftpClientControl.enterBinaryMode())return -1;
        
        //enter passive mode
        unsigned int port;
        try{port = ftpClientControl.enterPASV();}
        catch(const SHAISOCK::EXCEPTION& error){
            #ifdef DEBUG
            std::cerr <<"Exception thrown: "<<error.code<<" ; "<<error.errorStr<<std::endl;
            #endif
            return -2;
        }


        //create data socket connection
        FTPCLIENTDATA ftpClientData{
            version,
            SHAISOCK::ADDRESSFAMILY::SHAISOCK_AF_INET,
            SHAISOCK::SOCKETTYPE::SHAISOCK_SOCK_STREAM,
            SHAISOCK::SOCKETPROTOCOL::SHAISOCK_IPPROTO_TCP,
            ftpClientControl.getIpAddress(),
            port,
            localPath,
            8,
            16777216
        };


        //command to store file
        if(!ftpClientControl.storeFile(serverPath))return -3;

        //Upload file
        success = ftpClientData.putFile();
    }
    //checking for open local file error
    if(!success) return -4;
    
    //check for successful transfer
    ftpClientControl.sockRec();
    ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
    std::string code{"226"};
    if(!ftpClientControl.checkReponseCode(code.data()))return -5;
    ftpClientControl.printRecBuff();
    if(!ftpClientControl.confirmFileSizeTransfer(serverPath,localPath)){
        #ifdef DEBUG
        std::cout<<"File sizes don't match"<<std::endl;
        #endif    
        return -6;
    }
    #ifdef DEBUG
    std::cout<<"File sizes match"<<std::endl;
    #endif

    return 1;
}

//Errors
//0 - Failed to get all files
//-1 - Failed to enter Binary mode
//-2 - Failed to enter PASV
//-3 - Failed to get child items
//-4 - Failed to get some files
//Success
//1
short SHAIFTP::FTPCLIENT::getDir(const std::u8string dirPath, std::u8string destination){
    //enter binary mode
    std::wstring dirPathW{str8ToStrW(dirPath)};
    unsigned short indexToLastDir{returnIndexLastDirW(dirPathW)};
    unsigned short indexToDestLastDirU8{returnIndexLastDirU8(dirPath)};

    #ifdef DEBUG
    std::cout<< "Server Path U8: "<<reinterpret_cast<const char*> (dirPath.data())<<'\n';
    std::wcout<<L"Server Path W: "<<dirPathW<<L" ; Last Dir in Path: "
    <<dirPathW.substr(indexToLastDir,dirPathW.size()-indexToLastDir)<<std::endl;
    #endif
    if(!ftpClientControl.enterBinaryMode())return -1;
    if (destination.back()==u8'\\')destination.pop_back();
    replaceChar<std::u8string&,char8_t>(destination,u8'\\',u8'/');
    unsigned short startIndexOfDir{returnStartIndexFromSource(dirPath)};
    std::cout<<"start index of dir: "<<startIndexOfDir<<std::endl;
    std::u8string destFirstDirPath{(destination + (returnLastDir(dirPath)))};
    replaceChar<std::u8string&,char8_t>(destFirstDirPath,u8'\\',u8'/');
    
    //Variables - Populate directory and file arrays
    std::vector<SHAIFTP::U8STRING>dirChildItemArray{25};
    std::vector<SHAIFTP::U8STRING>fileChildItemArray{50};
    unsigned int dirCiaIndex{0};
    unsigned int dirCiaIndexEmpty{0};
    unsigned int fileCiaIndex{0};
    bool pass1{1};
    std::u8string dirPathLoop{dirPath};
    unsigned int port;
    unsigned short loop{0};
    
    //Populate directory and file array loop
    //dirChildItemArray[dirCiaIndex]
    do{
        std::vector<SHAIFTP::U8STRING>childItemArray{50};
        if(!pass1){
            dirChildItemArray[dirCiaIndex].resize();
            //dirPathLoop = returnPathFromIndex((dirChildItemArray[dirCiaIndex].str),destination.size());
            dirPathLoop = dirChildItemArray[dirCiaIndex].str;
        }
        
        //enter passive mode
        try{port = ftpClientControl.enterPASV();}
        catch(const SHAISOCK::EXCEPTION& error){
            #ifdef DEBUG
            std::cerr <<"Exception thrown: "<<error.code<<" ; "<<error.errorStr<<std::endl;
            #endif
            return -2;
        }
        
        //create data socket connection to get basic list of child items
        std::wstring localTargetPath{};
        FTPCLIENTDATA ftpClientDataList{
            version,
            SHAISOCK::ADDRESSFAMILY::SHAISOCK_AF_INET,
            SHAISOCK::SOCKETTYPE::SHAISOCK_SOCK_STREAM,
            SHAISOCK::SOCKETPROTOCOL::SHAISOCK_IPPROTO_TCP,
            ftpClientControl.getIpAddress(),
            port,
            localTargetPath
        };
        ftpClientControl.list(dirPathLoop);
        #ifdef DEBUG
        std::cout<<"\nAFTER CALLL LIST"<<std::endl;
        #endif
        if(!ftpClientDataList.getChildItems(childItemArray,ftpClientControl)){
            if(pass1){pass1=0;++loop;continue;}
            ++loop;++dirCiaIndex;continue;
        }
        
        //enter passive mode
        try{port = ftpClientControl.enterPASV();}
        catch(const SHAISOCK::EXCEPTION& error){
            #ifdef DEBUG
            std::cerr <<"Exception thrown: "<<error.code<<" ; "<<error.errorStr<<std::endl;
            #endif
            return -2;
        }
        
        //create data socket connection to get detailed list of child items
        FTPCLIENTDATA ftpClientDataListFull{
            version,
            SHAISOCK::ADDRESSFAMILY::SHAISOCK_AF_INET,
            SHAISOCK::SOCKETTYPE::SHAISOCK_SOCK_STREAM,
            SHAISOCK::SOCKETPROTOCOL::SHAISOCK_IPPROTO_TCP,
            ftpClientControl.getIpAddress(),
            port,
            localTargetPath
        };
        ftpClientControl.listFull(dirPathLoop);

        #ifdef DEBUG
        std::cout<<"Printing ChildItemArray:\n";
        for (int c = 0; c < childItemArray.size(); c++){
            std::cout<<childItemArray[c]<<" ; ";
        }
        std::cout<<"\n\n";
        #endif

        ftpClientDataListFull.getChildItemsFull(
            childItemArray,dirChildItemArray,fileChildItemArray,
            dirCiaIndexEmpty,fileCiaIndex,startIndexOfDir,indexToDestLastDirU8,destination
        );

        ftpClientControl.sockRec();
        ftpClientControl.resetRecBuff();
        
        //set loop variables flow control
        if(!pass1)++dirCiaIndex;
        if(pass1)pass1=0;
        ++loop;
    }
    while (dirCiaIndex < dirCiaIndexEmpty);

    dirChildItemArray.resize(dirCiaIndex);
    fileChildItemArray.resize(fileCiaIndex);
    
    ////////////////Print the arrays for troubleshooting/////////////////////////
    #ifdef DEBUG
    std::cout<<"Printing DirChildItemArray:\n";
    for (int c = 0; c < dirChildItemArray.size(); c++){
        std::cout<<dirChildItemArray[c]<<" ; ";
    }
    std::cout<<"\n\n";

    std::cout<<"Printing fileChildItemArray:\n";
    for (int c = 0; c < fileChildItemArray.size(); c++){
        std::cout<<fileChildItemArray[c]<<" ; ";
    }
    std::cout<<'\n';
    #endif
    ////////////////Print the arrays for troubleshooting/////////////////////////
    if (destination.back()==u8'\\')destination.pop_back();
    std::u8string localDirPath{destination+returnLastDir(dirPath)};
    #ifdef DEBUG2
    std::cout<<"\n\nPrinting Directories:\n";
    std::cout<<reinterpret_cast<char*>(localDirPath.data())<<" ; ";
    for (unsigned short c{0};c<dirChildItemArray.size();++c)std::cout<<dirChildItemArray[c]<<" ; ";
    std::cout<<"\n\nPrinting Files:\n";
    for (unsigned short c{0};c<fileChildItemArray.size();++c)std::cout<<fileChildItemArray[c]<<" ; ";
    std::cout<<"\n\n"<<std::flush;
    #endif
    
    //Create local directory tree
    std::filesystem::create_directory(localDirPath);
    for(unsigned int c{0};c<dirChildItemArray.size();++c){
        localDirPath = (destination + dirChildItemArray[c].str.substr(
            indexToDestLastDirU8,
            (dirChildItemArray[c].str.size() - indexToDestLastDirU8)
        ));
    }
    
    //Download files looping through file array
    unsigned int failureCount{0};
    short result;
    for(unsigned int c{0};c<fileChildItemArray.size();++c){
        std::cout<<reinterpret_cast<char*>(destination.data())<<'\n';
        std::wstring filePath{str8ToStrW(destination)};
        std::wcout<<filePath<<L'\n';
        mergeStr8IntoStrW(fileChildItemArray[c].str,filePath,indexToLastDir);
        std::wcout<<L"After Merge: "<<filePath<<L'\n';
        result = getFile(fileChildItemArray[c].str, filePath);
        if (result<0){
            ++failureCount;
            std::cout<<"Error code: "<<result<<";"<<reinterpret_cast<char*>(fileChildItemArray[c].str.data())<<std::endl;
        }
    }
    
    if(failureCount == fileChildItemArray.size())return 0;
    if(failureCount>0)return -4;
    return 1;
}

short SHAIFTP::FTPCLIENT::putDir(std::u8string& localDir, std::u8string targetPath){
    //remove trailing slash from paths
    if (localDir.back()==u8'\\'||localDir.back()==u8'/')localDir.pop_back();
    if(targetPath.back()!=u8'\\'&&targetPath.back()!=u8'/')targetPath.push_back(u8'/');
    #ifdef DEBUG
    std::cout<<"Local Dir: "<<reinterpret_cast<char*>(localDir.data())<<std::endl;
    #endif
    
    //set target path + local directory and remove trailing slash
    targetPath = targetPath+returnLastPath(localDir);
    SHAIFTP::replaceChar<std::u8string&,char8_t>(targetPath,u8'\\',u8'/');
    #ifdef DEBUG
    std::cout<<"Target path: "<<reinterpret_cast<char*>(targetPath.data())<<std::endl;
    #endif

    //Declare the Arrays, indexes etc.
    unsigned short dirArrayStartSize{25};
    unsigned short fileArrayStartSize{50};
    std::vector<SHAIFTP::U8STRING>dirArray{dirArrayStartSize};
    std::vector<SHAIFTP::U8STRING>fileArray{fileArrayStartSize};
    std::vector<std::wstring>localfileArray{fileArrayStartSize};
    unsigned dirArrayIndex{0};//fileVec current index
    unsigned fileArrayIndex{0};//fileVec current index
    dirArray[dirArrayIndex].str = targetPath;
    ++dirArrayIndex;
	
	//Populate Directory and File Arrays loop
    std::filesystem::recursive_directory_iterator sourceDirIt(localDir);//iterartor to source obj that will iterate to all files and dirs in source recursively
    for (sourceDirIt; sourceDirIt != std::filesystem::end(sourceDirIt); ++sourceDirIt) {
		//If item is a directory
		if (std::filesystem::is_directory((*sourceDirIt).path())) {
			if(dirArrayIndex<dirArrayStartSize){
                dirArray[dirArrayIndex].str = mergeTargetRootWithSourceExtension(
                    localDir.size(),sourceDirIt->path().u8string(),targetPath
                );
                SHAIFTP::replaceChar<std::u8string&,char8_t>(dirArray[dirArrayIndex].str,u8'\\',u8'/');
                ++dirArrayIndex;
            }
            else {
                dirArray.push_back(
                    mergeTargetRootWithSourceExtension(
                        localDir.size(),sourceDirIt->path().u8string(),targetPath
                    )
                );
                SHAIFTP::replaceChar<std::u8string&,char8_t>(dirArray[dirArrayIndex].str,u8'\\',u8'/');
                ++dirArrayIndex;
            }
		}
        else if(std::filesystem::is_regular_file((*sourceDirIt).path())){
            if(fileArrayIndex<fileArrayStartSize){
                localfileArray[fileArrayIndex] = sourceDirIt->path().wstring();
                fileArray[fileArrayIndex].str = mergeTargetRootWithSourceExtension(
                    localDir.size(),sourceDirIt->path().u8string(),targetPath
                );
                SHAIFTP::replaceChar<std::u8string&,char8_t>(fileArray[fileArrayIndex].str,u8'\\',u8'/');
                ++fileArrayIndex;
            }
            else{
                localfileArray.push_back(sourceDirIt->path().wstring());
                fileArray.push_back(
                    mergeTargetRootWithSourceExtension(
                        localDir.size(),sourceDirIt->path().u8string(),targetPath
                    )
                );
                SHAIFTP::replaceChar<std::u8string&,char8_t>(fileArray[fileArrayIndex].str,u8'\\',u8'/');
                ++fileArrayIndex;
            }
        }
    }
    
    //Resize the arrays
    dirArray.resize(dirArrayIndex);
    localfileArray.resize(fileArrayIndex);
    fileArray.resize(fileArrayIndex);
    //Create Directories recursively on FTP server
    bool result{0};
    for(unsigned int c{0};c<dirArray.size();++c){
        result = ftpClientControl.mkDir(dirArray[c].str);
        #ifdef DEBUG
        if(result)std::cout<<"Successfully created dir: "
        <<reinterpret_cast<char*>(dirArray[c].str.data())<<'\n';
        else std::cout<<"Failed created dir: "
        <<reinterpret_cast<char*>(dirArray[c].str.data())<<'\n';
        #endif
    }
    //Create files on FTP server
    short result2{};
    for(unsigned int c{0};c<fileArray.size();++c){
        result2 = putFile(localfileArray[c],fileArray[c].str);
        #ifdef DEBUG
        if(result2==1)std::cout<<"Successfully created file: "
        <<reinterpret_cast<char*>(fileArray[c].str.data())<<'\n';
        else std::cout<<"Failed to create file: "
        <<reinterpret_cast<char*>(fileArray[c].str.data())<<'\n';
        #endif
    }
    #ifdef DEBUG
    std::cout.flush();
    #endif
    return 1;
}

short SHAIFTP::FTPCLIENT::getFeatures(){
    ftpClientControl.features();
}

short SHAIFTP::FTPCLIENT::makeDir(std::u8string& dirPath){
    if(!(ftpClientControl.mkDir(dirPath))) return -1;
    return 1;
}
/////////////////////////////FTPCLIENT//////////////////////////////

/////////////////////////////HELPER FUNCTIONS//////////////////////////////
void SHAIFTP::mergeStr8IntoStrW(std::u8string& str8,std::wstring& strW, const unsigned short& indexToLastDir){
	int wBufferNeeded{MultiByteToWideChar(CP_UTF8,0,reinterpret_cast<char*>(str8.data()),str8.size(),0,0)};
	//std::unique_ptr<wchar_t[]>buffW{new wchar_t[wBufferNeeded+1]{}};
    std::wstring buffW(wBufferNeeded,0);
	MultiByteToWideChar(CP_UTF8,0,reinterpret_cast<char*>(str8.data()),str8.size(),buffW.data(),wBufferNeeded);
    std::wstring buffW2{buffW.substr(indexToLastDir, buffW.size()-indexToLastDir)};
    for(unsigned short c{0};c<wBufferNeeded;++c)strW.push_back(buffW2[c]);
}
std::wstring SHAIFTP::str8ToStrW(const std::u8string& str8){
    int wBufferNeeded{MultiByteToWideChar(CP_UTF8,0,reinterpret_cast<const char*>(str8.data()),str8.size(),0,0)};
    std::wstring buffW(wBufferNeeded,L'\0');
    MultiByteToWideChar(CP_UTF8,0,reinterpret_cast<const char*>(str8.data()),str8.size(),buffW.data(),wBufferNeeded);
    return buffW;
}
std::u8string SHAIFTP::mergeTargetRootWithSourceExtension(
        const unsigned short& sourceSize, std::u8string source,
        const std::u8string& target 
){   
    return (target+source.substr(sourceSize,source.size()));
}
unsigned short SHAIFTP::returnIndexLastDirW(const std::wstring& strW){
	unsigned short index{static_cast<unsigned short>(strW.size())};
	while(strW[index]!=L'/')--index;
	return index;
}

unsigned short SHAIFTP::returnIndexLastDirU8(const std::u8string& strU8){
	unsigned short index{static_cast<unsigned short>(strU8.size())};
	while(strU8.data()[index]!='/')--index;
	return index;
}

std::u8string SHAIFTP::returnLastDir(std::u8string dirPath){
    if(dirPath.back()==u8'/'||dirPath.back()==u8'\\')dirPath.pop_back();
    short index{static_cast<short>(dirPath.size())};
    while (dirPath[index]!=u8'/'&&dirPath[index]!=u8'\\'&&(index>(-1)))--index;
    return dirPath.substr(index,dirPath.size());
}

unsigned short SHAIFTP::returnStartIndexFromSource(std::u8string dirPath){
    if(dirPath.back()==u8'/'||dirPath.back()==u8'\\')dirPath.pop_back();
    short startIndex{static_cast<short>(dirPath.size())};
    while (dirPath[startIndex]!=u8'/'&&dirPath[startIndex]!=u8'\\'&&(startIndex>(-1)))--startIndex;
    return startIndex;
}

void SHAIFTP::setConsoleMode(CONSOLEMODE mode){
	if (mode == CONSOLEMODE::utf8)_setmode(_fileno(stdout), _O_TEXT);
	else if(mode == CONSOLEMODE::utf16)_setmode(_fileno(stdout), _O_U16TEXT);
}

std::u8string SHAIFTP::returnLastPath(std::u8string& str8){
    unsigned short lastPathStartIndex{startIndexOfLastPath(str8)};
    //std::cout<<"Test index: "<<lastPathStartIndex<<std::endl;
    std::u8string returnStr8{str8.substr(lastPathStartIndex,(str8.size()-lastPathStartIndex))};
    std::cout<<"Last Part String: "<<reinterpret_cast<char*>(returnStr8.data())<<std::endl;
    return returnStr8;
}

unsigned short SHAIFTP::startIndexOfLastPath(std::u8string& str8){
    unsigned short index{static_cast<unsigned short>(str8.size()-1)};
    while(str8[index]!= u8'\\' && str8[index]!= u8'/')--index;
    return (index+1);
}

std::u8string SHAIFTP::returnPathFromIndex(const std::u8string& str8, const unsigned short& index){
    std::cout<<"TroubleShooting 1: "<<reinterpret_cast<const char*> (str8.data())<<" ; "<<index<<std::endl;
    std::u8string returnStr8{str8.substr(index,(str8.size()-index))};
    return returnStr8;
}
/////////////////////////////HELPER FUNCTIONS//////////////////////////////