/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */


#include "EmptyHeader.h"

#ifndef _WIN32
#include <netdb.h>
#endif

#ifdef RAKNET_SOCKET_2_INLINE_FUNCTIONS

#ifndef RAKNETSOCKET2_BERKLEY_NATIVE_CLIENT_CPP
#define RAKNETSOCKET2_BERKLEY_NATIVE_CLIENT_CPP

// Every platform except windows store 8 and native client supports Berkley sockets
#if !defined(WINDOWS_STORE_RT)

#include "Itoa.h"

// Shared on most platforms, but excluded from the listed


void DomainNameToIP_Berkley_IPV4And6(const char* domainName, char ip[65]) {
#if RAKNET_SUPPORT_IPV6 == 1
    // 确保ip数组被正确初始化
    const size_t IP_BUFFER_SIZE = 65;
    memset(ip, 0, IP_BUFFER_SIZE);

    if (domainName == nullptr || domainName[0] == '\0') { return; }

    struct addrinfo hints, *res = nullptr, *p = nullptr;
    int             status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC; // 支持IPv4和IPv6
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_ADDRCONFIG; // 只返回当前系统支持的地址类型

    status = getaddrinfo(domainName, NULL, &hints, &res);
    if (status != 0 || res == nullptr) { return; }

    // 遍历所有返回的地址
    for (p = res; p != nullptr; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            // IPv4处理
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
            inet_ntop(AF_INET, &(ipv4->sin_addr), ip, IP_BUFFER_SIZE);
            break; // 使用第一个IPv4地址
        }
#if RAKNET_SUPPORT_IPV6 == 1
        else if (p->ai_family == AF_INET6) {
            // IPv6处理
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
            inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip, IP_BUFFER_SIZE);
            break; // 使用第一个IPv6地址
        }
#endif
    }

    freeaddrinfo(res);
#else
    // 不支持IPv6的情况
    (void)domainName;
    memset(ip, 0, 65);
#endif
}


void DomainNameToIP_Berkley_IPV4(const char* domainName, char ip[65]) {
    static struct in_addr addr;
    memset(&addr, 0, sizeof(in_addr));

    // Use inet_addr instead? What is the difference?
    struct hostent* phe = gethostbyname(domainName);

    if (phe == 0 || phe->h_addr_list[0] == 0) {
        // cerr << "Yow! Bad host lookup." << endl;
        memset(ip, 0, 65 * sizeof(char));
        return;
    }

    if (phe->h_addr_list[0] == 0) {
        memset(ip, 0, 65 * sizeof(char));
        return;
    }

    memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));
    strcpy(ip, inet_ntoa(addr));
}


void DomainNameToIP_Berkley(const char* domainName, char ip[65]) {
#if RAKNET_SUPPORT_IPV6 == 1
    return DomainNameToIP_Berkley_IPV4And6(domainName, ip);
#else
    return DomainNameToIP_Berkley_IPV4(domainName, ip);
#endif
}


#endif // !defined(WINDOWS_STORE_RT) && !defined(__native_client__)

#endif // file header

#endif // #ifdef RAKNET_SOCKET_2_INLINE_FUNCTIONS
