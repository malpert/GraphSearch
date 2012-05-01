#pragma once

#include <SFML/Graphics.hpp>
#include <set>
#include <iostream>

class Node;
class Edge;

class Face
{
private:
	Face(Node * n1, Node * n2, Node * n3);
	~Face();
public:


	Node * n1;
	Node * n2;
	Node * n3;

	//
	// Static
	//
	static Face * createFace(Node * n1, Node * n2, Node * n3);
	static bool destoryFace(Face * f);
};