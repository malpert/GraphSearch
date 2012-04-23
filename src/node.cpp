#include <SFML/Graphics.hpp>
#include <set>
#include <iostream>

#include "node.h"
#include "edge.h"

Node::Node(int x, int y) : x((float)x), y((float)y), selected(false)
{
	init();
}

Node::Node(float x, float y) : x(x), y(y), selected(false)
{
	init();
}

Node::~Node()
{
	//for (size_t i = 0; i < edges.size(); ++i)
	//{
	//	if (edges[i])
	//	{
	//		
	//	}
	//}
}

void Node::init()
{
	circ.setRadius(5);
	circ.setPosition(x-5, y-5);
	circ.setFillColor(sf::Color::Black);
	circ.setOutlineColor(sf::Color::Red);
	circ.setOutlineThickness(0);
}

void Node::select()
{
	circ.setOutlineThickness(2);
	selected = true;
}

void Node::deselect()
{
	circ.setOutlineThickness(0);
	selected = false;
}

bool Node::isSelected()
{
	return selected;
}

void Node::setPosition(int x, int y)
{
	setPosition((float)x, (float)y);
}
void Node::setPosition(float x, float y)
{
	this->x = x;
	this->y = y;
	circ.setPosition(x-5, y-5);
}

void Node::move(int x, int y)
{
	move((float)x, (float)y);
}
void Node::move(float x, float y)
{
	this->x += x;
	this->y += y;
	circ.move(x, y);
}

std::ostream & operator<<(std::ostream & out, const Node & rhs)
{
	out << "(" << rhs.x << ", " << rhs.y << ")";
	return out;
}
