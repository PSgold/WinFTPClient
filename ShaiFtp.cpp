#include "ShaiFtp.h"
//Exceptions thrown on construction
//8 - login failure

//Other Exceptions
//9 - Failed to enter passive mode 

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

bool SHAIFTP::FTPCLIENTCONTROL::login(){
    if(user!= u8"anonymous"){
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
    return checkReponseCode(code);
}

unsigned int SHAIFTP::FTPCLIENTCONTROL::enterPASV(){
    this->setFtpCommand(u8"PASV");
    this->resetRecBuff();
    this->sockSend(ftpCommandLength);
    this->sockRec();
    writeResponseCode(recBuff.get());
    std::string code{"227"};
    if(!checkReponseCode(code.data()))throw 9;
    #ifdef DEBUG
    printRecBuff();
    #endif
    
    #ifdef BUFFBYTES
    SHAISOCK::printCharArray(this->recBuff.get(),SHAISOCK::base10,100);
    #endif
    
    std::string portPart1(8,'\0');
    std::string portPart2(8,'\0');
    unsigned short part1Index{0};
    unsigned short part2Index{0};
    unsigned short commaNum{0};
    for(unsigned int c{26};c<this->recBuffSize;++c){
        if (commaNum<4){
            if (this->recBuff[c]==',')++commaNum;
            continue;
        }
        else if (commaNum==4){
            if(this->recBuff[c]==','){++commaNum;continue;}
            portPart1[part1Index] = this->recBuff[c];
            ++part1Index;
        }
        else{
            if(this->recBuff[c]==')')break;
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
    this->resetRecBuff();
    return port;
}

char* SHAIFTP::FTPCLIENTCONTROL::getRecBuff(){return recBuff.get();}
unsigned short SHAIFTP::FTPCLIENTCONTROL::getFtpCommandLength(){return ftpCommandLength;}

void SHAIFTP::FTPCLIENTCONTROL::writeResponseCode(char* responseCodeToWrite){
    for(unsigned short c{0};c<3;++c)responseCode[c]=responseCodeToWrite[c];
}
bool SHAIFTP::FTPCLIENTCONTROL::checkReponseCode(char* codeToCheck){
    bool isEqual{1};
    for(unsigned short c{0};c<3;++c){if(responseCode[c]!=codeToCheck[c]){isEqual=0;break;}}
    return isEqual;
}
char* SHAIFTP::FTPCLIENTCONTROL::getResponseCode(){return responseCode;}
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
    //if(this->downloadToPath.back()!=L'\\')this->downloadToPath.push_back(L'\\');
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
    //if(this->downloadToPath.back()!=L'\\')this->downloadToPath.push_back(L'\\');
}

void SHAIFTP::FTPCLIENTDATA::getFile(){
    #ifdef DEBUG
    std::cout<<"Downloading file"<<std::endl;
    #endif
    std::ofstream oFileStream{localPath,std::ios::binary};
    int bytesRec;
    do{
        bytesRec = recv(sock,recBuff.get(),recBuffSize,0);
        oFileStream.write(recBuff.get(),bytesRec);
    } 
    while (bytesRec>=1);
    oFileStream.close();
    this->resetRecBuff();
}

void SHAIFTP::FTPCLIENTDATA::putFile(){
    #ifdef DEBUG
    std::cout<<"Uploading file"<<std::endl;
    #endif
    std::ifstream iFileStream{localPath,std::ios::binary|std::ios::ate};
    unsigned long long fileSize{static_cast<unsigned long long>(iFileStream.tellg())};
    #ifdef DEBUG
    std::cout<<"File size: "<< fileSize<<std::endl;
    #endif
    iFileStream.seekg(std::ios::beg,std::ios::beg);
    #ifdef DEBUG
    std::cout<<"Reset position to: "<<iFileStream.tellg()<<std::endl;
    #endif
    unsigned long long bytesRead{0};
    while(bytesRead<fileSize){
        iFileStream.read(sendBuff.get(),sendBuffSize);
        //SHAISOCK::printCharArray(sendBuff.get(),SHAISOCK::printType::base10,33);
        //std::cout<<std::endl;
        unsigned int charsRead{static_cast<unsigned int>(iFileStream.gcount())};
        //std::cout<<"Chars read: "<<charsRead<<std::endl;    
        sockSend(charsRead);
        bytesRead+=charsRead;
    }
    #ifdef DEBUG
    std::cout<<"Bytes read: "<<bytesRead<<std::endl;
    #endif
    iFileStream.close();
    //this->close();
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
    #ifdef DEBUG
    std::cout<<"Constructed FTPCLIENT"<<std::endl;
    ftpClientControl.sockRec();
    ftpClientControl.printRecBuff();
    ftpClientControl.resetRecBuff();
    #endif
    if(!ftpClientControl.login())throw 8;
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
    if(!ftpClientControl.login())throw 8;
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
    #ifdef DEBUG
    std::cout<<"Constructed FTPCLIENT"<<std::endl;
    ftpClientControl.sockRec();
    ftpClientControl.printRecBuff();
    ftpClientControl.resetRecBuff();
    #endif
    if(!ftpClientControl.login())throw 8;
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
    #ifdef DEBUG
    std::cout<<"Constructed FTPCLIENT"<<std::endl;
    ftpClientControl.sockRec();
    ftpClientControl.printRecBuff();
    ftpClientControl.resetRecBuff();
    #endif
    if(!ftpClientControl.login())throw 8;
}
SHAIFTP::FTPCLIENT::~FTPCLIENT(){
    std::u8string command{u8"QUIT"};
    ftpClientControl.setFtpCommand(command);
    ftpClientControl.sockSend(ftpClientControl.getFtpCommandLength());
    ftpClientControl.sockRec();
    ftpClientControl.printRecBuff();
    #ifdef DEBUG
    std::cout<<"Destructed FTPCLIENT"<<std::endl;
    #endif
}

//Errors return -1 and -2
//Returns 1 on success
short SHAIFTP::FTPCLIENT::getFile(
    const std::u8string pathFromServer,
    const std::wstring localTargetPath
){  
    std::u8string command = u8"TYPE I";
    ftpClientControl.setFtpCommand(command);
    ftpClientControl.sockSend(ftpClientControl.getFtpCommandLength());
    ftpClientControl.sockRec();
    ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
    ftpClientControl.printRecBuff();
    ftpClientControl.resetRecBuff();

    unsigned int port{ftpClientControl.enterPASV()};
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
    command = u8"RETR "+pathFromServer;
    ftpClientControl.setFtpCommand(command);
    ftpClientControl.sockSend(ftpClientControl.getFtpCommandLength());
    ftpClientControl.sockRec();
    ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
    std::string code{"150"};
    #ifdef DEBUG
    ftpClientControl.printRecBuff();
    #endif
    ftpClientControl.resetRecBuff();
    if(!ftpClientControl.checkReponseCode(code.data()))return -1;
    
    //download file
    ftpClientData.getFile();
    
    //check command to for successful transfer
    ftpClientControl.sockRec();
    ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
    #ifdef DEBUG
    ftpClientControl.printRecBuff();
    #endif
    ftpClientControl.resetRecBuff();
    code = "226";
    if(!ftpClientControl.checkReponseCode(code.data()))return -2;
    return 1;
}

short SHAIFTP::FTPCLIENT::putFile(
    const std::wstring localPath, 
    const std::u8string serverPath
){
    //ftpclientdata in its own scope to destruct at correct time
    {   
        std::u8string command = u8"TYPE I";
        ftpClientControl.setFtpCommand(command);
        ftpClientControl.sockSend(ftpClientControl.getFtpCommandLength());
        ftpClientControl.sockRec();
        ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
        ftpClientControl.printRecBuff();
        ftpClientControl.resetRecBuff();
        
        unsigned int port{ftpClientControl.enterPASV()};
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
        
        command = (u8"STOR "+serverPath);
        ftpClientControl.setFtpCommand(command);
        ftpClientControl.sockSend(ftpClientControl.getFtpCommandLength());
        ftpClientControl.sockRec();
        ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
        ftpClientControl.printRecBuff();
        ftpClientControl.resetRecBuff();

        //Upload file
        ftpClientData.putFile();
    }
    
    ftpClientControl.sockRec();
    ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
    ftpClientControl.printRecBuff();
    ftpClientControl.resetRecBuff();

    std::u8string command{u8"SIZE "+serverPath};
    ftpClientControl.setFtpCommand(command);
    ftpClientControl.sockSend(ftpClientControl.getFtpCommandLength());
    ftpClientControl.sockRec();
    ftpClientControl.writeResponseCode(ftpClientControl.getRecBuff());
    ftpClientControl.printRecBuff();
    ftpClientControl.resetRecBuff();

    return 1;
}
/////////////////////////////FTPCLIENT//////////////////////////////