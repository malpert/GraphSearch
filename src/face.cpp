#include "face.h"

#include <vector>
#include <algorithm>

#include "node.h"
#include "edge.h"

Face::Face(Node * n1, Node * n2, Node * n3) : n1(n1), n2(n2), n3(n3)
{
	// Assumes nodes ordered from address to largest

	if (!(n1 && n2 && n3))
		assert(!"Face::Face: nodes");

	// Add face to nodes
	n1->faces.insert(this);
	n2->faces.insert(this);
	n3->faces.insert(this);

	// Add face to edges
	int good[] = {0, 0, 0};

	for (auto it = n1->edges.begin(); it != n1->edges.end(); ++it)
	{
		// Assumes nodes in edges in order from smallest address to largest
		if ((*it)->n2 == n2)
		{
			++(*it)->faces;
			++good[0];
		}
		if ((*it)->n2 == n3)
		{
			++(*it)->faces;
			++good[1];
		}
	}

	for (auto it = n2->edges.begin(); it != n2->edges.end(); ++it)
	{
		// Assumes nodes in edges in order from smallest address to largest
		if ((*it)->n2 == n3)
		{
			++(*it)->faces;
			++good[2];
		}
	}

	// Enforce 1 edge between each node found and incremented
	if (good[0] != 1 || good[1] != 1 || good[2] != 1)
		assert(!"Face::Face: edge count");
}

Face::~Face()
{
	// Remove face from nodes
	n1->faces.erase(this);
	n2->faces.erase(this);
	n3->faces.erase(this);

	// Remove face from edges
	int good[] = {1, 1, 1};

	for (auto it = n1->edges.begin(); it != n1->edges.end(); ++it)
	{
		// Assumes nodes in edges in order from smallest address to largest
		if ((*it)->n2 == n2)
		{
			--(*it)->faces;
			--good[0];
		}
		if ((*it)->n2 == n3)
		{
			--(*it)->faces;
			--good[1];
		}
	}

	for (auto it = n2->edges.begin(); it != n2->edges.end(); ++it)
	{
		// Assumes nodes in edges in order from smallest address to largest
		if ((*it)->n2 == n3)
		{
			--(*it)->faces;
			--good[2];
		}
	}

	// Enforce 1 edge between each node found and decremented
	if (good[0] != 0 || good[1] != 0 || good[2] != 0)
		assert(!"Face::~Face: edge count");
}

//
// Static
//

Face * Face::createFace(Node * n1, Node * n2, Node * n3)
{
	//std::cout << n1 << ' ' << n2 << ' ' << n3 << std::endl;
	// If null
	if (!n1 || !n2 || !n3) return 0;

	// If same
	if (n1 == n2 || n1 == n3 || n2 == n3) return 0;

	// If not a face
	if (n1->neighbors.find(n2)==n1->neighbors.end() || n1->neighbors.find(n3) == n1->neighbors.end() || n2->neighbors.find(n3)==n2->neighbors.end()) return 0;

	// Enforce order
	std::vector<Node*> v;
	v.push_back(n1);
	v.push_back(n2);
	v.push_back(n3);
	std::sort(v.begin(), v.end());

	// If face already exists
	for (auto it = v[0]->faces.begin(); it != v[0]->faces.end(); ++it)
	{
		// If face already existed, nodes would be in the same order.
		if ((*it)->n2 == v[1] && (*it)->n3 == v[2]) return 0;
	}

	// Create face
	//std::cout << "create" << std::endl;
	return new Face(v[0], v[1], v[2]);
}

bool Face::destoryFace(Face * f)
{
	if (f)
	{
		delete f;
		return true;
	}
	return false;
}