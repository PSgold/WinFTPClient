Basic Usage:
1. Create ftpClient object: 
    SHAIFTP::FTPCLIENT ftpClient{
        serverName[domain or ip],
        user[u8 string],
        pass[u8string],
        SHAIFTP::DOMAINFLAG::domainFlag[added only if serverName is domain and not ip address]
    };
2. Download file:
    ftpClient.getFile(source[u8 string],destination[wstring]);

3. Upload file:
    ftpClient.putFile(source[wstring],destination[u8 string]);

4. Download folder:
	ftpClient.getDir(source[u8 string],destination[u8string]);