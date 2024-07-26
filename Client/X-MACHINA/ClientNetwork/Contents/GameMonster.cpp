#include "stdafx.h"
#include "GameMonster.h"



GameMonster::GameMonster()
{
}

GameMonster::~GameMonster()
{
}

void GameMonsterInfo::InitPheros(std::string pheros)
{
  size_t length = pheros.length();
  for (size_t i = 0; i < length; i += 3) {
      PheroInfo phero;
      phero.ID = i / 3 + 1;
      phero.level = pheros[i] - '0'; // Convert char to int
      phero.pheroIndex = std::stoi(pheros.substr(i + 1, 2)); // Convert substring to int
      mPheros.push_back(phero);
    }
}
