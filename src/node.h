#pragma once

#include <SFML/Graphics.hpp>
#include <set>

class Node
{
public:
	Node(float x, float y) : x(x), y(y)
	{
		circleShape.setRadius(5);
		circleShape.setPosition(x-5, y-5);
		circleShape.setFillColor(sf::Color::Black);
	}
	
	bool addNeighbor(Node * n)
	{
		return neighbors.insert(n).second;
	}
	
	bool eraseNeighbor(Node * n)
	{
		if (neighbors.find(n) != neighbors.end())
		{
			neighbors.erase(n);
			return true;
		}
		return false;
	}

	float x;
	float y;
	std::set<Node*> neighbors;
	sf::CircleShape circleShape;
};
