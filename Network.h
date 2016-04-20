#pragma once
#ifndef NETWORK_H
#define NETWORK_H

#include "stdafx.h" 
#include "Globals.h"

using namespace std;
using namespace sf; 

struct NetworkPacket {
	string type = "NOTHING";
	string ip;
	string message = "";
	int playerID = 0;
	int dataSize = 0;
	vector<float> data;
};

struct PlayerInfo {
	int id = 0;
	string ip = "";
	unsigned int port = 0;
	bool update = false;
	vector<float> data;
	bool dead = false;
	time_t lastMessage;
};

class Network {
private:
	vector<PlayerInfo> connectedPlayerInfo;
	UdpSocket sendSocket;
	UdpSocket receiveSocket;
	vector<NetworkPacket*> receivedPackets;
	//NetworkPacket* sentMessage;
	vector<NetworkPacket*> sentMessages;
	time_t timer; 
public:
	queue<string> textQueue;
	bool restart = false;

	Network();
	static Network* GetInstance();
	void Update();
	void ReceiveMessages();
	void HandleMessage();
	void ProcessMessageData(NetworkPacket*);
	void SendPacket(IpAddress, NetworkPacket*);
	void SendPacketThread();
	void CheckDisconnect();
	void CheckWinner();
	void StartGame();
	vector<PlayerInfo> getPlayers();
	int NumberOfPlayers();
	void SendPlayerData();
	void SendPlayerMessages();
};

#endif