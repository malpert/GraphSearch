#pragma once

#include <SFML/Graphics.hpp>
#include <set>
#include <iostream>

class Edge;
class EdgePtr;

class Node
{
public:
	Node(int x, int y) : x((float)x), y((float)y), selected(false)
	{
		init();
	}

	Node(float x, float y) : x(x), y(y), selected(false)
	{
		init();
	}

	~Node()
	{
		for (size_t i = 0; i < edges.size(); ++i)
		{
			//edges[i]->update();
			//edges[i]->n1;
		}
	}

	void init()
	{
		circ.setRadius(5);
		circ.setPosition(x-5, y-5);
		circ.setFillColor(sf::Color::Black);
		circ.setOutlineColor(sf::Color::Red);
		circ.setOutlineThickness(0);
	}
	
	//bool addNeighbor(Node * n)
	//{
	//	return neighbors.insert(n).second;
	//}
	//
	//bool eraseNeighbor(Node * n)
	//{
	//	if (neighbors.find(n) != neighbors.end())
	//	{
	//		neighbors.erase(n);
	//		return true;
	//	}
	//	return false;
	//}

	void select()
	{
		circ.setOutlineThickness(2);
		selected = true;
	}

	void deselect()
	{
		circ.setOutlineThickness(0);
		selected = false;
	}

	bool isSelected()
	{
		return selected;
	}

	void setPosition(int x, int y)
	{
		setPosition((float)x, (float)y);
	}
	void setPosition(float x, float y)
	{
		this->x = x;
		this->y = y;
		circ.setPosition(x-5, y-5);
	}

	void move(int x, int y)
	{
		move((float)x, (float)y);
	}
	void move(float x, float y)
	{
		this->x += x;
		this->y += y;
		circ.move(x, y);
	}

	friend std::ostream & operator<<(std::ostream & out, const Node & rhs)
	{
		out << "(" << rhs.x << ", " << rhs.y << ")";
		return out;
	}

	float x;
	float y;
	bool selected;
	std::vector<EdgePtr> edges;
	std::vector<Node*> neighbors;
	sf::CircleShape circ;
};
