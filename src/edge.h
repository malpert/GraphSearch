#pragma once

#include <SFML/Graphics.hpp>
#include <math.h>
#include "node.h"

#define RADTODEG 57.29577951f

class Edge
{
public:
	Edge(Node * n1 = 0, Node * n2 = 0, float thickness = 2) : n1(n1), n2(n2), ht(thickness/2), selected(false)
	{
		init();
	}
	
	void init()
	{
		rect.setFillColor(sf::Color::Black);
		rect.setOutlineColor(sf::Color::Red);
		rect.setOutlineThickness(0);
		srect.setFillColor(sf::Color::Black);
		srect.setOutlineColor(sf::Color::Red);
		srect.setOutlineThickness(0);
		srect.setSize(sf::Vector2f(10,10));
		srect.setOrigin(5,5);
	}

	void update()
	{
		if (!n1 || !n2) return;

		float dx = n2->x - n1->x;
		float dy = n2->y - n1->y;
		float rot = atan2(dy, dx) * RADTODEG;
		rect.setSize(sf::Vector2f(std::sqrt(std::abs(dx)*std::abs(dx) + std::abs(dy)*std::abs(dy)), ht*2));
		rect.setOrigin(0, ht);
		rect.setPosition(n1->x, n1->y);
		rect.setRotation(rot);
		srect.setPosition(n1->x+(dx)/2, n1->y+(dy)/2);
		srect.setRotation(rot);
	}

	bool operator==(const Edge & rhs) const
	{
		return (rhs.n1 == n1 && rhs.n2 == n2);
	}

	bool operator==(const Edge * rhs) const
	{
		return (rhs->n1 == this->n1 && rhs->n2 == this->n2);
	}

	Node * n1;
	Node * n2;
	float ht; // half-thickness
	bool selected;
	sf::RectangleShape rect;
	sf::RectangleShape srect;
};

class EdgePtr
{
public:
	EdgePtr(Edge * p = 0) : p(p) {}
	EdgePtr& operator=(const EdgePtr & rhs) { p = rhs.p; return *this; }
	EdgePtr& operator=(Edge * rhs) { p = rhs; return *this; }
	Edge* operator->() { return p; }
	Edge& operator*() { return *p; }
	operator Edge*() { return p; } // Allows Edge* == EdgePtr
	bool operator==(Edge * rhs) const { return p == rhs; } // Allows EdgePtr == Edge*
	bool operator==(const EdgePtr & rhs) const
	{
		return rhs.p->n1 == p->n1 && rhs.p->n2 == p->n2;
	}
	bool operator<(const EdgePtr & rhs) const
	{
		if (p->n1 == rhs.p->n1)
			return p->n2 < rhs.p->n2;
		else
			return p->n1 < rhs.p->n1;
	}
//private:
	Edge * p;
};
