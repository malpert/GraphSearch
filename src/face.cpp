#include "face.h"

#include <vector>
#include <algorithm>

#include "node.h"
#include "edge.h"

Face::Face(Node * n1, Node * n2, Node * n3)
{

}

Face::~Face()
{

}

//
// Static
//

Face * Face::createFace(Node * n1, Node * n2, Node * n3)
{
	// If null
	if (!n1 || !n2 || !n3) return 0;

	// If same
	if (n1 == n2 || n1 == n3 || n2 == n3) return 0;

	// If not a face
	if (n1->neighbors.find(n2)==n1->neighbors.end() || n1->neighbors.find(n3) == n1->neighbors.end()) return 0;

	// Enforce order
	std::vector<Node*> v;
	v.push_back(n1);
	v.push_back(n2);
	v.push_back(n3);
	std::sort(v.begin(), v.end());

	// If face already exists
	for (std::set<Face*>::iterator it = v[0]->faces.begin(); it != v[0]->faces.end(); ++it)
	{
		// If face already existed, nodes would be in the same order.
		if ((*it)->n2 == v[1] && (*it)->n3 == v[2]) return 0;
	}

	// Create face
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