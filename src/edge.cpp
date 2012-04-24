#include "edge.h"
#include "node.h"

std::set<Edge*> * Edge::eset = 0;
QuadTree<Edge*> * Edge::qtree = 0;

Edge::Edge(Node * n1, Node * n2, float thickness) : n1(n1), n2(n2), ht(thickness/2), selected(false)
{
	if (!(n1 && n2))
		assert(!"Edge::Edge: nodes");
	init();
}

Edge::~Edge()
{
	// Erase nodes from their neighbors sets
	n1->neighbors.erase(n2);
	n2->neighbors.erase(n1);
	// Erase self from nodes' edge sets
	n1->edges.erase(this);
	n2->edges.erase(this);
	// Erase self from edge set and quadtree (if they are set)
	if (eset) eset->erase(this);
	if (qtree) qtree->erase(this, srect.getPosition().x, srect.getPosition().y);
}

void Edge::init()
{
	rect.setFillColor(sf::Color::Black);
	rect.setOutlineColor(sf::Color::Red);
	rect.setOutlineThickness(0);
	srect.setFillColor(sf::Color::Black);
	srect.setOutlineColor(sf::Color::Red);
	srect.setOutlineThickness(0);
	srect.setSize(sf::Vector2f(10,10));
	srect.setOrigin(5,5);

	// Add nodes to their neighbors set
	n1->neighbors.insert(n2);
	n2->neighbors.insert(n1);
	// Add self to nodes' edge sets
	n1->edges.insert(this);
	n2->edges.insert(this);
	// Add self to edge set and quadtree (if they are set)
	if (eset) eset->insert(this);
	//if (qtree) qtree->insert(this, srect.getPosition().x, srect.getPosition().y);

	update(); // Will add self to quadtree
}

void Edge::update()
{
	if (!n1 || !n2) return;

	// Erase self from quadtree
	if (qtree) qtree->erase(this, srect.getPosition().x, srect.getPosition().y);

	// Update
	float dx = n2->x - n1->x;
	float dy = n2->y - n1->y;
	float rot = atan2(dy, dx) * RADTODEG;
	rect.setSize(sf::Vector2f(std::sqrt(std::abs(dx)*std::abs(dx) + std::abs(dy)*std::abs(dy)), ht*2));
	rect.setOrigin(0, ht);
	rect.setPosition(n1->x, n1->y);
	rect.setRotation(rot);
	srect.setPosition(n1->x+(dx)/2, n1->y+(dy)/2);
	srect.setRotation(rot);

	// Insert self to into quadtree
	if (qtree) qtree->insert(this, srect.getPosition().x, srect.getPosition().y);
}

bool Edge::operator==(const Edge & rhs) const
{
	return (rhs.n1 == n1 && rhs.n2 == n2)
		|| (rhs.n1 == n2 && rhs.n2 == n1);
}

//bool Edge::operator==(const Edge * rhs) const
//{
//	return (rhs->n1 == n1 && rhs->n2 == n2);
//}

//
// Static
//
Edge * Edge::createEdge(Node * n1, Node * n2, float thickness)
{
	// Not null
	if (!n1 || !n2) return 0;

	// Not same
	if (n1 == n2) return 0;

	// If not already neighbors...
	if (n1->neighbors.find(n2)==n1->neighbors.end() && n2->neighbors.find(n1)==n2->neighbors.end())
		return new Edge(n1, n2, thickness); // Create edge
	else
		return 0;
}

bool Edge::destroyEdge(Node * n1, Node * n2)
{
	// Not null
	if (!(n1 && n2)) return false;

	// If not neighbors...
	if (n1->neighbors.find(n2)==n1->neighbors.end() && n2->neighbors.find(n1)==n2->neighbors.end())
	{
		return false;
	}
	else
	{
		// Find and delete edge
		for (std::set<Edge*>::iterator it = n1->edges.begin(); it != n1->edges.end(); ++it)
		{
			Edge * e = *it;
			if (e->n1 == n1 && e->n2 == n2 || e->n1 == n2 && e->n2 == n1)
			{
				delete e; // Edge destructor will erase edge from its nodes' edge sets
				// (one of which we're incrementing through) and erase it from the edge
				// set and the quadtree (if they are set).
				return true;
			}
		}
		// n1 and n2 are neighbors, they must have an edge!
		assert(!"Edge::destroyEdge: missing edge");
		return false;
	}
}

void Edge::setEdgeSet(std::set<Edge*> * edgeSet)
{
	eset = edgeSet;
}

void Edge::setQuadTree(QuadTree<Edge*> * quadTree)
{
	qtree = quadTree;
}
