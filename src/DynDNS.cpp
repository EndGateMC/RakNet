/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_DynDNS == 1 && _RAKNET_SUPPORT_TCPInterface == 1

#include "Base64Encoder.h"
#include "DynDNS.h"
#include "GetTime.h"
#include "RakNetSocket2.h"
#include "TCPInterface.h"

using namespace RakNet;

struct DynDnsResult {
    const char*      description;
    const char*      code;
    DynDnsResultCode resultCode;
};

DynDnsResult resultTable[13] = {
    // See http://www.dyndns.com/developers/specs/flow.pdf
    {"DNS update success.\nPlease wait up to 60 seconds for the change to take effect.\n",
     "good",                                                                                           RC_SUCCESS    }, // Even with success, it takes time for the cache to update!
    {"No change",                                                                          "nochg",    RC_NO_CHANGE  },
    {"Host has been blocked. You will need to contact DynDNS to reenable.",                "abuse",    RC_ABUSE      },
    {"Useragent is blocked",                                                               "badagent", RC_BAD_AGENT  },
    {"Username/password pair bad",                                                         "badauth",  RC_BAD_AUTH   },
    {"Bad system parameter",                                                               "badsys",   RC_BAD_SYS    },
    {"DNS inconsistency",                                                                  "dnserr",   RC_DNS_ERROR  },
    {"Paid account feature",                                                               "!donator", RC_NOT_DONATOR},
    {"No such host in system",                                                             "nohost",   RC_NO_HOST    },
    {"Invalid hostname format",                                                            "notfqdn",  RC_NOT_FQDN   },
    {"Serious error",                                                                      "numhost",  RC_NUM_HOST   },
    {"This host exists, but does not belong to you",                                       "!yours",   RC_NOT_YOURS  },
    {"911",                                                                                "911",      RC_911        }
};
DynDNS::DynDNS() {
    connectPhase = CP_IDLE;
    tcp          = 0;
}
DynDNS::~DynDNS() {
    if (tcp) RakNet::OP_DELETE(tcp, _FILE_AND_LINE_);
}
void DynDNS::Stop(void) {
    tcp->Stop();
    connectPhase = CP_IDLE;
    RakNet::OP_DELETE(tcp, _FILE_AND_LINE_);
    tcp = 0;
}


// newIPAddress is optional - if left out, DynDNS will use whatever it receives
void DynDNS::UpdateHostIPAsynch(const char* dnsHost, const char* newIPAddress, const char* usernameAndPassword) {
    myIPStr[0] = 0;

    if (tcp == 0) tcp = RakNet::OP_NEW<TCPInterface>(_FILE_AND_LINE_);
    connectPhase = CP_IDLE;
    host         = dnsHost;

    if (tcp->Start(0, 1) == false) {
        SetCompleted(RC_TCP_FAILED_TO_START, "TCP failed to start");
        return;
    }

    connectPhase = CP_CONNECTING_TO_CHECKIP;
    tcp->Connect("checkip.dyndns.org", 80, false);

    // See https://www.dyndns.com/developers/specs/syntax.html
    getString  = "GET /nic/update?hostname=";
    getString += dnsHost;
    if (newIPAddress) {
        getString += "&myip=";
        getString += newIPAddress;
    }
    getString += "&wildcard=NOCHG&mx=NOCHG&backmx=NOCHG HTTP/1.0\n";
    getString += "Host: members.dyndns.org\n";
    getString += "Authorization: Basic ";
    char outputData[512];
    Base64Encoding((const unsigned char*)usernameAndPassword, (int)strlen(usernameAndPassword), outputData);
    getString += outputData;
    getString += "User-Agent: Jenkins Software LLC - PC - 1.0\n\n";
}
void DynDNS::Update(void) {
    if (connectPhase == CP_IDLE) return;

    serverAddress = tcp->HasFailedConnectionAttempt();
    if (serverAddress != UNASSIGNED_SYSTEM_ADDRESS) {
        SetCompleted(RC_TCP_DID_NOT_CONNECT, "Could not connect to DynDNS");
        return;
    }

    serverAddress = tcp->HasCompletedConnectionAttempt();
    if (serverAddress != UNASSIGNED_SYSTEM_ADDRESS) {
        if (connectPhase == CP_CONNECTING_TO_CHECKIP) {
            checkIpAddress = serverAddress;
            connectPhase   = CP_WAITING_FOR_CHECKIP_RESPONSE;
            tcp->Send(
                "GET\n\n",
                (unsigned int)strlen("GET\n\n"),
                serverAddress,
                false
            ); // Needs 2 newlines! This is not documented and wasted a lot of my time
        } else {
            connectPhase = CP_WAITING_FOR_DYNDNS_RESPONSE;
            tcp->Send(getString.C_String(), (unsigned int)getString.GetLength(), serverAddress, false);
        }
        phaseTimeout = RakNet::GetTime() + 1000;
    }

    if (connectPhase == CP_WAITING_FOR_CHECKIP_RESPONSE && RakNet::GetTime() > phaseTimeout) {
        connectPhase = CP_CONNECTING_TO_DYNDNS;
        tcp->CloseConnection(checkIpAddress);
        tcp->Connect("members.dyndns.org", 80, false);
    } else if (connectPhase == CP_WAITING_FOR_DYNDNS_RESPONSE && RakNet::GetTime() > phaseTimeout) {
        SetCompleted(RC_DYNDNS_TIMEOUT, "DynDNS did not respond");
        return;
    }

    Packet* packet = tcp->Receive();
    if (packet) {
        if (connectPhase == CP_WAITING_FOR_DYNDNS_RESPONSE) {
            unsigned int i;

            char* _result;
            _result = strstr((char*)packet->data, "Connection: close");
            if (_result != 0) {
                _result += strlen("Connection: close");
                while (*_result && ((*_result == '\r') || (*_result == '\n') || (*_result == ' '))) _result++;
                for (i = 0; i < 13; i++) {
                    if (strncmp(resultTable[i].code, _result, strlen(resultTable[i].code)) == 0) {
                        if (resultTable[i].resultCode == RC_SUCCESS) {
                            // Read my external IP into myIPStr
                            // Advance until we hit a number
                            while (*_result && ((*_result < '0') || (*_result > '9'))) _result++;
                            if (*_result) {
                                SystemAddress parser;
                                parser.FromString(_result);
                                parser.ToString(false, myIPStr);
                            }
                        }
                        tcp->DeallocatePacket(packet);
                        SetCompleted(resultTable[i].resultCode, resultTable[i].description);
                        break;
                    }
                }
                if (i == 13) {
                    tcp->DeallocatePacket(packet);
                    SetCompleted(RC_UNKNOWN_RESULT, "DynDNS returned unknown result");
                }
            } else {
                tcp->DeallocatePacket(packet);
                SetCompleted(RC_PARSING_FAILURE, "Parsing failure on returned string from DynDNS");
            }

            return;
        } else {
            /*
            HTTP/1.1 200 OK
            Content-Type: text/html
            Server: DynDNS-CheckIP/1.0
            Connection: close
            Cache-Control: no-cache
            Pragma: no-cache
            Content-Length: 105

            <html><head><title>Current IP Check</title></head><body>Current IP Address: 98.1
            89.219.22</body></html>


            Connection to host lost.
            */

            char* _result;
            _result = strstr((char*)packet->data, "Current IP Address: ");
            if (_result != 0) {
                _result += strlen("Current IP Address: ");
                SystemAddress myIp;
                myIp.FromString(_result);
                myIp.ToString(false, myIPStr);

                char existingHost[65];
                existingHost[0] = 0;
                // Resolve DNS we are setting. If equal to current then abort
                RakNetSocket2::DomainNameToIP(host.C_String(), existingHost);
                if (existingHost && strcmp(existingHost, myIPStr) == 0) {
                    // DynDNS considers setting the IP to what it is already set abuse
                    tcp->DeallocatePacket(packet);
                    SetCompleted(RC_DNS_ALREADY_SET, "No action needed");
                    return;
                }
            }

            tcp->DeallocatePacket(packet);
            tcp->CloseConnection(packet->systemAddress);

            connectPhase = CP_CONNECTING_TO_DYNDNS;
            tcp->Connect("members.dyndns.org", 80, false);
        }
    }

    if (tcp->HasLostConnection() != UNASSIGNED_SYSTEM_ADDRESS) {
        if (connectPhase == CP_WAITING_FOR_DYNDNS_RESPONSE) {
            SetCompleted(RC_CONNECTION_LOST_WITHOUT_RESPONSE, "Connection lost to DynDNS during GET operation");
        }
    }
}


#endif // _RAKNET_SUPPORT_DynDNS
