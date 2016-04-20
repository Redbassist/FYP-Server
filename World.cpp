#include "World.h"#include "Stalker.h"

#include "Inventory.h"

World::World(bool load) {
	//loadedCharacrter = load;
	//CreateAssets();
	if (loadedCharacrter) {
		//LoadPlayer();
	}
	//CreateLevel();

	playerThreshold = 2;
}

void World::CreateAssets() {
	m_texture.loadFromFile("Sprites/background.png");
	m_texture.setSmooth(false);
	m_sprite.setTexture(m_texture);
	m_sprite.setTextureRect(sf::IntRect(0, 0, m_texture.getSize().x, m_texture.getSize().y));
	m_sprite.setOrigin(m_texture.getSize().x / 2, m_texture.getSize().y / 2);
	m_sprite.setScale(Vector2f(0.4, 0.4));
	m_sprite.setPosition(0, 0);

	enemyManager = EnemyManager();
}

void World::CreateLevel() {
	View view = View(FloatRect(0, 0, 1280, 720));
	view.zoom(0.7);
	window->setView(view);

	for (auto layer = ml->GetLayers().begin(); layer != ml->GetLayers().end(); ++layer)
	{
		/*if (!loadedCharacrter) {
			if (layer->name == "Player")
			{
				for (auto object = layer->objects.begin(); object != layer->objects.end(); ++object)
				{
					player = new Player(Vector2f(object._Ptr->GetPosition()));
					break;
				}
			}
		}*/

		if (layer->name == "Buildings")
		{
			for (auto object = layer->objects.begin(); object != layer->objects.end(); ++object)
			{
				if (object._Ptr->GetName() == "House")
					houses.push_back(new House(Vector2f(object._Ptr->GetPosition()), &items, 0));
				else if (object._Ptr->GetName() == "House2") {
					houses.push_back(new House(Vector2f(object._Ptr->GetPosition()), &items, 1));
				}
				else if (object._Ptr->GetName() == "Shop") {
					houses.push_back(new House(Vector2f(object._Ptr->GetPosition()), &items, 2));
				}
				else if (object._Ptr->GetName() == "CabinR") {
					houses.push_back(new House(Vector2f(object._Ptr->GetPosition()), &items, 3));
				}
				else if (object._Ptr->GetName() == "CabinL") {
					houses.push_back(new House(Vector2f(object._Ptr->GetPosition()), &items, 4));
				}
				else if (object._Ptr->GetName() == "Military") {
					houses.push_back(new House(Vector2f(object._Ptr->GetPosition()), &items, 5));
				}
				/*else if (object._Ptr->GetName() == "Enemy") {
					enemyManager.AddStalker(new Stalker(Vector2f(object._Ptr->GetPosition())));
				}*/
			}
		}

		if (layer->name == "Lights")
		{
			for (auto object = layer->objects.begin(); object != layer->objects.end(); ++object)
			{
				fillers.push_back(new FillerObject(object._Ptr->GetPosition(), 0, object._Ptr->GetName(), true, true));
			}
		}

		if (layer->name == "Trees")
		{
			for (auto object = layer->objects.begin(); object != layer->objects.end(); ++object)
			{
				trees.push_back(new Tree(object._Ptr->GetPosition()));
			}
		}
	}
}

void World::Update() {

	View view = window->getView();
	view.setCenter(2800, 2000);
	window->setView(view);

	if (Network::GetInstance()->NumberOfPlayers() >= playerThreshold && !gameStarted) {
		gameStarted = true;
		Network::GetInstance()->StartGame();
		cout << "Start Game" << endl;

		vector<PlayerInfo> p = Network::GetInstance()->getPlayers();
		int size = p.size();

		for (int i = 0; i < size; i++) {
			players.push_back(new Player(Vector2f(200, 200), p[i].id));
		}
	}
	if (gameStarted) {
		Network::GetInstance()->CheckDisconnect();
		Network::GetInstance()->CheckWinner();
		Network::GetInstance()->SendPlayerData();
		Network::GetInstance()->SendPlayerMessages();
	}

	/*vector<PlayerInfo> p = Network::GetInstance()->getPlayers();
	int size = p.size();
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < players.size(); j++) {
			if (p[i].id == players[j]->PlayerID() && p[i].update) {
				players[j]->UpdateNetworkPlayer(p[i].data);
				Network::GetInstance()->getPlayers()[i].update = false;
				break;
			}
		}
	}

	size = players.size();
	for (int i = 0; i < size; i++) {
		players[i]->Update();
	}

	if (size > 0) {
		players[0]->CenterCamera();
	}*/

	//player->Update();
	//enemyManager.Update(player);

	/*int size = trees.size();
	for (int i = 0; i < size; i++) {
		trees[i]->FadeOut(player->GetPosition());
	}*/
}

void World::Draw() {
	//window->draw(m_sprite);  
	/*
	int size = houses.size();
	for (int i = 0; i < size; i++) {
		houses[i]->Draw();
	}

	//drawing the items
	size = items.size();
	for (int i = 0; i < size; i++) {
		if (items[i]->destroy) {
			delete items[i];
			items.erase(items.begin() + i);
			size = items.size();
			i--;
		}
		else
			items[i]->Draw();
	}

	//enemyManager.Draw();

	size = players.size();
	for (int i = 0; i < size; i++) {
		players[i]->Draw();
	}

	size = fillers.size();
	for (int i = 0; i < size; i++) {
		fillers[i]->Draw();
	}

	size = trees.size();
	for (int i = 0; i < size; i++) {
		trees[i]->Draw();
	}*/
}