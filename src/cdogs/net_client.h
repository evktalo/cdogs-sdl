/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.

    Copyright (c) 2014, Cong Xu
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __NET_CLIENT
#define __NET_CLIENT

#include <time.h>

#include "net_util.h"

typedef enum
{
	NET_STATE_STARTING = 0,
	NET_STATE_LOADED,
	NET_STATE_ERROR
} NetClientState;

typedef struct
{
	ENetHost *client;
	ENetPeer *peer;
	NetClientState State;
} NetClient;

extern NetClient gNetClient;

void NetClientInit(NetClient *n);
void NetClientTerminate(NetClient *n);

// Attempt to connect to a server
void NetClientConnect(NetClient *n, const ENetAddress addr);
bool NetClientTryLoadCampaignDef(NetClient *n, NetMsgCampaignDef *def);
void NetClientPoll(NetClient *n);
// Send a command to the server
void NetClientSend(NetClient *n, int cmd);

bool NetClientIsConnected(const NetClient *n);

#endif
