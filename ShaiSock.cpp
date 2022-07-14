#include "ShaiSock.h"

//Exceptions thrown on construction
//1 - version error
//2 - Winsock failed to initialize
//3 - Failed to create socket
//4 - inetptow error - error with ipaddress conversion to binary
//5 - Socket connect error
//6 - DNS resolve error
//7 - DNS resolve error


/////////////////////////////BASE INITSOCK//////////////////////////////
SHAISOCK::INITSOCK::INITSOCK(WINSOCKVERSION winSockVersion){
    switch (winSockVersion){
        case TwoTwo:{version[0]=2;version[1]=2;}break;
        case TwoOne:{version[0]=2;version[1]=1;}break;
        case Two:{version[0]=2;version[1]=0;}break;
        case OneOne:{version[0]=1;version[1]=1;}break;
        case One:{version[0]=1;version[1]=0;}break;
        
        default:throw 1;break;
    }
    if(!checkVersion())throw 1;
    if((WSAStartup((MAKEWORD(version[0],version[1])),&wsaData))!=0)throw 2;
    #ifdef DEBUG
    std::cout<<"Constructed Initsock"<<std::endl;
    #endif
}

SHAISOCK::INITSOCK::~INITSOCK(){
    WSACleanup();
    #ifdef DEBUG
    std::cout<<"Destructed Initsock"<<std::endl;
    #endif
}

bool SHAISOCK::INITSOCK::checkVersion(){
    if(version[0]==2 && version[1]==2)return 1;
    else if(
        (version[0]==2 && version[1]==1) ||
        (version[0]==2 && version[1]==0)
    )return 1;
    else if(
        (version[0]==1 && version[1]==1) ||
        (version[0]==1 && version[1]==0)
    )return 1;
    else return 0;
}

float SHAISOCK::INITSOCK::getVersion(){
    if(version[0]==2&&version[1]==2)return 2.2;
    else if(version[0]==2&&version[1]==2)return 2.1;
    else if(version[0]==2&&version[1]==0)return 2.0;
    else if(version[0]==1&&version[1]==1)return 1.1;
    else if(version[0]==1&&version[1]==0)return 1.0;
    else return -1.0;
}
/////////////////////////////BASE INITSOCK//////////////////////////////


/////////////////////////////BASIC CLIENTOSCK//////////////////////////////
SHAISOCK::CLIENTSOCK::CLIENTSOCK(
    WINSOCKVERSION winSockVersion,
    ADDRESSFAMILY addrFamily,
    SOCKETTYPE sockType,
    SOCKETPROTOCOL socProtocol,
    std::wstring ipAddress,
    unsigned int port,
    unsigned int recBuffSize,
    unsigned int sendBuffSize
):SHAISOCK::INITSOCK(winSockVersion),sockAddr{},ipAddress{ipAddress},
recBuffSize{recBuffSize},sendBuffSize{sendBuffSize},
recBuff{new char[recBuffSize]{}},sendBuff{new char[sendBuffSize]{}}{
    sock = socket(addrFamily,sockType,socProtocol);
    if(sock==INVALID_SOCKET)throw 3;
    sockAddr.sin_family = addrFamily;
    sockAddr.sin_port = htons(port);
    int returnVal{InetPtonW(addrFamily,this->ipAddress.data(),&sockAddr.sin_addr)};
    if(returnVal!=1)throw 4;
    returnVal = connect(sock,reinterpret_cast<sockaddr*>(&sockAddr),sizeof(sockAddr));
    if(returnVal!=0)throw 5;
    #ifdef DEBUG
    std::cout<<"Constructed ClientSock\nSock connected"<<std::endl;
    #endif
}
SHAISOCK::CLIENTSOCK::CLIENTSOCK(
    std::wstring serverDomain,
    WINSOCKVERSION winSockVersion,
    ADDRESSFAMILY addrFamily,
    SOCKETTYPE sockType,
    SOCKETPROTOCOL socProtocol,
    unsigned int port,
    unsigned int recBuffSize,
    unsigned int sendBuffSize
):SHAISOCK::INITSOCK(winSockVersion),recBuffSize{recBuffSize},sendBuffSize{sendBuffSize},
recBuff{new char[recBuffSize]{}},sendBuff{new char[sendBuffSize]{}}{
    ipAddress = resolveToIPv4(serverDomain);
    #ifdef DEBUG
    std::wcout<<L"IP Address to convert: "<<ipAddress<<std::endl;
    #endif
    sock = socket(addrFamily,sockType,socProtocol);
    if(sock==INVALID_SOCKET)throw 3;
    sockAddr.sin_family = addrFamily;
    sockAddr.sin_port = htons(port);
    int returnVal{InetPtonW(addrFamily,this->ipAddress.data(),&sockAddr.sin_addr)};
    if(returnVal!=1)throw 4;
    returnVal = connect(sock,reinterpret_cast<sockaddr*>(&sockAddr),sizeof(sockAddr));
    if(returnVal!=0)throw 5;
    #ifdef DEBUG
    std::cout<<"Constructed ClientSock\nSock connected"<<std::endl;
    #endif
}

SHAISOCK::CLIENTSOCK::~CLIENTSOCK(){
    int returnVal{shutdown(sock,SD_BOTH)};
    #ifdef DEBUG
    std::cout<<"ShutdownSocket return: "<<returnVal<<std::endl;
    #endif
    returnVal = closesocket(sock);
    #ifdef DEBUG
    std::cout<<"CloseSocket return: "<<returnVal<<"\nDestructed ClientSock"<<std::endl;
    #endif
}

void SHAISOCK::CLIENTSOCK::sockSend(unsigned int buffSize){
    int result{0};
    if (buffSize==0){result = send(sock,sendBuff.get(),sendBuffSize,0);}
    else{result = send(sock,sendBuff.get(),buffSize,0);}
    #ifdef BUFFBYTES
    std::cout<<"Send Result: "<<result<<'\n';
    if(buffSize==0)SHAISOCK::printCharArray(sendBuff.get(),base10,sendBuffSize);
    else SHAISOCK::printCharArray(sendBuff.get(),base10,buffSize);
    std::cout<<std::endl;
    #endif
    this->resetSendBuff();
}
void SHAISOCK::CLIENTSOCK::sockRec(){
    #ifdef DEBUG
    std::cout<<"Receiving"<<std::endl;
    #endif
    recv(sock,recBuff.get(),recBuffSize,0);
}

void SHAISOCK::CLIENTSOCK::resetRecBuff(){
    for(unsigned int c{0};c<recBuffSize;++c)recBuff[c] = '\0';
}
void SHAISOCK::CLIENTSOCK::resetSendBuff(){
    for(unsigned int c{0};c<sendBuffSize;++c)sendBuff.get()[c] = '\0';
}

void SHAISOCK::CLIENTSOCK::printRecBuff(){
    #ifdef DEBUG
    std::cout<<"Printing"<<std::endl;
    #endif
    std::cout<<recBuff.get()<<std::endl;
}
void SHAISOCK::CLIENTSOCK::printSendBuff(){std::cout<<sendBuff.get()<<std::endl;}
bool SHAISOCK::CLIENTSOCK::setRecTimeOut(unsigned int timeout){
    timeout = (timeout*1000);
    char* timeoutBuff{reinterpret_cast<char*>(&timeout)};
    int result{setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,timeoutBuff,sizeof(unsigned int))};
    if(result==0)return 1;
    else return 0;
}
std::wstring SHAISOCK::CLIENTSOCK::getIpAddress(){return this->ipAddress;}
/////////////////////////////BASIC CLIENTOSCK//////////////////////////////





/////////////////////////////HELPER FUNCTIONS//////////////////////////////
#ifdef DEBUG
void SHAISOCK::printCharArray(const char* const buff,const printType type,const unsigned int size){
        switch(type){
            case character:{
                for(unsigned int c{0};c<size;++c){std::cout<<buff[c];}
                std::cout.flush();
            }break;
            case base10:{
                for(unsigned int c{0};c<size;++c){std::cout<<static_cast<unsigned short>((*(reinterpret_cast<const unsigned char*>((buff+c)))))<<' ';}
                std::cout.flush();
            }break;
        }
}
#endif
std::wstring SHAISOCK::resolveToIPv4(const std::wstring& domainName){
	addrinfoW hints{};
	addrinfoW* addrResult{nullptr};
	hints.ai_family = SHAISOCK_AF_UNSPEC;
	hints.ai_socktype = SHAISOCK_SOCK_STREAM;
	hints.ai_protocol = SHAISOCK_IPPROTO_TCP;
	int resolveResult{
		GetAddrInfoW(
			domainName.data(),
			NULL,
			&hints,
			&addrResult
		)
	};
    if(resolveResult!=0){
        std::cout<<"Winsock Resolve Error: "<<resolveResult<<std::endl;
        throw 6;
    }
	if(addrResult->ai_family == SHAISOCK_AF_INET){
        unsigned short length{static_cast<unsigned short>(addrResult->ai_addrlen)};
        const unsigned short buffSize{17};
        char serverAddr[buffSize]{};
        std::wstring serverAddrW(buffSize,L'\0');
        sockaddr_in* sockAddrInPtr{reinterpret_cast<sockaddr_in*>(addrResult->ai_addr)};
        inet_ntop(AF_INET,&(sockAddrInPtr->sin_addr),serverAddr,16);
        for(unsigned short c{0};c<buffSize;++c){
            serverAddrW[c] = serverAddr[c];
        }
        return serverAddrW;
    }
    else throw 7;
}