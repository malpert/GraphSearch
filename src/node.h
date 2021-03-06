#pragma once

#include <SFML/Graphics.hpp>
#include <set>
#include <iostream>

#include "quadtree.h"

class Edge;

class Node
{
	friend class Edge;

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

	void move(int dx, int dy);
	void move(float dx, float dy);

	friend std::ostream & operator<<(std::ostream & out, const Node & rhs);

	float x;
	float y;
	bool selected;
	std::set<Edge*> edges;
	std::set<Node*> neighbors;
	sf::CircleShape circ;
	
	//
	// Static
	//
	static void setNodeSet(std::set<Node*> * nodeSet);
	static void setQuadTree(QuadTree<Node*> * quadTree);
	static std::set<Node*> * nset;
	static QuadTree<Node*> * qtree;
};
