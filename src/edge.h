#pragma once

#include <SFML/Graphics.hpp>
#include <math.h>

#include "quadtree.h"

#define RADTODEG 57.29577951f

class Node;

class Edge
{
	friend class Node;

private:

	Edge(Node * n1, Node * n2, float thickness = 2);

public:

	~Edge();
	
	void init();

	void update();

	void move(int dx, int dy);
	void move(float dx, float dy);

	bool operator==(const Edge & rhs) const;

	bool operator==(const Edge * rhs) const;

	Node * n1;
	Node * n2;
	float ht; // half-thickness
	bool selected;
	bool updateDisabled;
	sf::RectangleShape rect;
	sf::RectangleShape srect;

	//
	// Static
	//
	static Edge * createEdge(Node * n1, Node * n2, float thickness = 2);
	static bool destroyEdge(Node * n1, Node * n2);
	static void setEdgeSet(std::set<Edge*> * edgeSet);
	static void setQuadTree(QuadTree<Edge*> * quadTree);
	static std::set<Edge*> * eset;
	static QuadTree<Edge*> * qtree;
};
