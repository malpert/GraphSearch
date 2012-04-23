#pragma once

#include <SFML/Graphics.hpp>
#include <set>
#include <iostream>

class Edge;
class EdgePtr;

class Node
{
public:
	Node(int x, int y);
	Node(float x, float y);
	~Node();
	void init();
	void select();
	void deselect();
	bool isSelected();
	void setPosition(int x, int y);
	void setPosition(float x, float y);
	void move(int x, int y);
	void move(float x, float y);
	friend std::ostream & operator<<(std::ostream & out, const Node & rhs);

	float x;
	float y;
	bool selected;
	//std::vector<EdgePtr> edges;
	std::set<Edge*> edges;
	std::vector<Node*> neighbors;
	sf::CircleShape circ;
};
