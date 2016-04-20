#include "Network.h"

static bool instanceFlag = false;
static Network* instance = NULL;

int ThreadedReceive(void* param) {
	static_cast<Network*>(param)->ReceiveMessages();
	return 1;
}

int ThreadedSend(void* param) {
	static_cast<Network*>(param)->SendPacketThread();
	return 0;
}

sf::Packet& operator << (sf::Packet& packet, const NetworkPacket& p)
{
	int size = p.data.size();
	packet << p.type << p.ip << p.message << p.playerID << size;
	for (int i = 0; i < size; i++) {
		packet << p.data[i];
	}
	return packet;
}

sf::Packet& operator >> (sf::Packet& packet, NetworkPacket& p)
{
	packet >> p.type >> p.ip >> p.message >> p.playerID >> p.dataSize;
	int size = p.dataSize;
	for (int i = 0; i < size; i++) {
		float dataPiece;
		packet >> dataPiece;
		p.data.push_back(dataPiece);
	}
	return packet;
}

Network::Network()
{
	if (receiveSocket.bind(54000) != sf::Socket::Done)
	{
		cout << "Could not bind to specified port" << endl;
	}
	receiveSocket.setBlocking(true);

	if (sendSocket.bind(55000) != sf::Socket::Done)
	{
		cout << "Could not bind to specified port" << endl;
	}
	sendSocket.setBlocking(true);

	Parameter p;
	p.param = this;

	ThreadPool::GetInstance()->AddTask(ThreadedReceive, p);
}

Network * Network::GetInstance()
{
	if (!instanceFlag) {
		instance = new Network();
		instanceFlag = true;
	}
	return instance;
}

void Network::Update()
{
	ReceiveMessages();
}

void Network::ReceiveMessages()
{
	//cout << "Receiving Message" << endl;
	sf::Packet packet;
	IpAddress sender;
	unsigned short port;
	receiveSocket.receive(packet, sender, port);
	NetworkPacket* np = new NetworkPacket();
	packet >> *np;
	receivedPackets.push_back(np);
	HandleMessage();
}

void Network::HandleMessage()
{
	if (receivedPackets.size() > 0) {
		//cout << "Handling Message" << endl; 
		for (int i = 0; i < receivedPackets.size(); i++) {
			ProcessMessageData(receivedPackets[i]);
			delete receivedPackets[i];
			receivedPackets.erase(receivedPackets.begin() + i);
			i--;
		}
	}
}

void Network::ProcessMessageData(NetworkPacket* np)
{
	string messageType;
	unsigned int port = 8080;
	messageType = np->type;
	if (messageType != "NOTHING") {
		if (messageType == "Connection") {
			int size = connectedPlayerInfo.size();
			bool addPlayer = true;
			for (int i = 0; i < size; i++) {
				if (connectedPlayerInfo[i].id == np->playerID)
					addPlayer = false;
			}

			if (addPlayer) {
				PlayerInfo temp;
				temp.id = np->playerID;
				temp.ip = np->ip;
				temp.port = port;
				temp.lastMessage = time_t(timer);
				connectedPlayerInfo.push_back(temp);
			}

			NetworkPacket* tempNP = new NetworkPacket();
			tempNP->type = "Success";
			tempNP->ip = np->ip;
			SendPacket(np->ip, tempNP);
		}

		else if (messageType == "Disconnect") {
			int size = connectedPlayerInfo.size();
			for (int i = 0; i < size; i++) {
				if (connectedPlayerInfo[i].id == np->playerID) {
					connectedPlayerInfo.erase(connectedPlayerInfo.begin() + i);
					break;
				}
			}
		}

		else if (messageType == "PlayerData") {
			mutexR.lock();
			int size = connectedPlayerInfo.size();
			for (int i = 0; i < size; i++) {
				if (connectedPlayerInfo[i].id == np->playerID) {
					connectedPlayerInfo[i].lastMessage = time(&timer);
					connectedPlayerInfo[i].data.clear();
					connectedPlayerInfo[i].update = true;
					connectedPlayerInfo[i].data = np->data;
				}
			}
			mutexR.unlock();
		}
		else if (messageType == "Dead") {
			mutexR.lock();
			int size = connectedPlayerInfo.size();
			for (int i = 0; i < size; i++) {
				if (connectedPlayerInfo[i].id == np->playerID) {
					connectedPlayerInfo[i].dead = true;
				}
			}
			mutexR.unlock();
		}
		else if (messageType == "Text") {
			textQueue.push(np->message);
		}
	}
}

void Network::SendPacket(IpAddress ip, NetworkPacket* np)
{
	np->ip = ip.toString();
	sentMessages.push_back(np);

	Parameter p;
	p.param = this;

	ThreadPool::GetInstance()->AddTask(ThreadedSend, p);
}

void Network::SendPacketThread()
{
	sf::Packet packet;
	if (sentMessages.size() > 0) {
		packet << *(sentMessages[0]);
		IpAddress reciever = sentMessages[0]->ip;
		unsigned short port = 8080;
		sendSocket.send(packet, reciever, port);
		delete sentMessages[0];
		sentMessages.erase(sentMessages.begin());
	}
}

void Network::CheckDisconnect()
{
	int timeout = 30;
	int size = connectedPlayerInfo.size();
	for (int i = 0; i < size; i++) {
		if (difftime(time(&timer), connectedPlayerInfo[i].lastMessage) > timeout) {
			connectedPlayerInfo[i].dead = true;
		}
	}
}

void Network::CheckWinner()
{
	int playersLeft = 0;
	PlayerInfo winningPlayer;

	int size = connectedPlayerInfo.size();
	for (int i = 0; i < size; i++) {
		if (!connectedPlayerInfo[i].dead) {
			playersLeft++;
			winningPlayer = connectedPlayerInfo[i];
		}
	}

	if (playersLeft == 1) {
		NetworkPacket* tempNP = new NetworkPacket();
		tempNP->type = "Winner";
		tempNP->ip = winningPlayer.ip;
		SendPacket(tempNP->ip, tempNP);
	}
}


void Network::StartGame()
{
	int size = connectedPlayerInfo.size();
	for (int i = 0; i < size; i++) {
		connectedPlayerInfo[i].lastMessage = time(&timer);
		NetworkPacket* tempNP = new NetworkPacket();
		tempNP->type = "Start";
		tempNP->ip = connectedPlayerInfo[i].ip;
		SendPacket(tempNP->ip, tempNP);
	}
}

vector<PlayerInfo> Network::getPlayers()
{
	return connectedPlayerInfo;
}

int Network::NumberOfPlayers()
{
	return connectedPlayerInfo.size();
}

void Network::SendPlayerData()
{
	int size = connectedPlayerInfo.size();

	for (int i = 0; i < size; i++) {
		NetworkPacket* np = new NetworkPacket();
		np->type = "EnemyPlayerData";

		for (int j = 0; j < size; j++) {
			if (connectedPlayerInfo[j].dead && connectedPlayerInfo[j].data.size() > 0)
				connectedPlayerInfo[j].data[0] = 999999;
			vector<float> tempCopy = connectedPlayerInfo[j].data;
			np->data.push_back(connectedPlayerInfo[j].id);
			np->data.insert(np->data.end(), tempCopy.begin(), tempCopy.end());

		}
		np->dataSize = np->data.size();

		np->ip = connectedPlayerInfo[i].ip;
		SendPacket(np->ip, np);
	}
}

void Network::SendPlayerMessages()
{
	if (textQueue.size() > 0) {
		int size = connectedPlayerInfo.size();

		for (int i = 0; i < size; i++) {
			NetworkPacket* np = new NetworkPacket();
			np->type = "Text";
			np->message = textQueue.front();
			np->ip = connectedPlayerInfo[i].ip;
			SendPacket(np->ip, np);
		}

		textQueue.pop();
	}
}
