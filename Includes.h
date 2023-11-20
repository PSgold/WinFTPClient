#pragma once

#define _WIN32_DCOM
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN
#define NOCOMM
#define NOMINMAX
#define DEBUG
//#define DEBUG2
#define LOG
#define MAXCHILDITEMS 500
//#define BUFFBYTES

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include "Windows.h"
#include "WinSock2.h"
#include "ws2tcpip.h"
#include "WinSock2.h"
#include "ws2tcpip.h"
#include "LM.h"
#include "io.h"
#include "fcntl.h"