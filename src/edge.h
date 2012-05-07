#pragma once

#include <SFML/Graphics.hpp>
#include <set>

#include "quadtree.h"

#define RADTODEG 57.29577951f

class Node;
class Face;

class Edge
{
	friend class Node;
	friend class Face;

public:

	struct Comp
	{
		bool operator() (Edge * lhs, Edge * rhs) const
		{
			if (lhs->n1 < rhs->n1) return true;
			else if (lhs->n1 > rhs->n1) return false;
			else if (lhs->n2 < rhs->n2) return true;
			else return false;
		}
	};

private:

	Edge(Node * n1, Node * n2, float thickness);
	Edge(Node * n1, Node * n2); // Dummy for finding within a set using Comp

public:

	~Edge();
	
	void init();

	void updateBorder();

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
	int faces;
	bool dummy;
	sf::RectangleShape rect;
	sf::RectangleShape srect;

	//
	// Static
	//
	static Edge * createEdge(Node * n1, Node * n2, float thickness = 2);
	static bool destroyEdge(Node * n1, Node * n2);
	static Edge * findEdge(const std::set<Edge*, Edge::Comp> & s, Node * n1, Node * n2);
	static void setEdgeSet(std::set<Edge*> * edgeSet);
	static void setQuadTree(QuadTree<Edge*> * quadTree);
	static std::set<Edge*> * eset;
	static QuadTree<Edge*> * qtree;
	static bool intersect(Edge * e1, Edge * e2, float * x = 0, float * y = 0);
	static bool intersect(Edge * e1, Node * n1, Node * n2, float * x = 0, float * y = 0);
	static bool intersect(Node * n1, Node * n2, Node * n3, Node * n4, float * x = 0, float * y = 0);
};
