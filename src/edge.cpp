#include "edge.h"
#include "node.h"
#include "face.h"

#include <math.h>
#include <limits>

std::set<Edge*> * Edge::eset = 0;
QuadTree<Edge*> * Edge::qtree = 0;

Edge::Edge(Node * n1, Node * n2, float thickness) : n1(n1), n2(n2), ht(thickness/2), selected(false), updateDisabled(false), faces(0), dummy(false)
{
	// Assumes nodes ordered by address

	if (!(n1 && n2))
		assert(!"Edge::Edge: nodes");

	init();
}

Edge::Edge(Node * n1, Node * n2) : n1(n1), n2(n2), dummy(true) {}

Edge::~Edge()
{
	if (dummy) return;

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
	rect.setOrigin(0, ht);

	srect.setFillColor(sf::Color::Black);
	srect.setOutlineColor(sf::Color::Red);
	srect.setOutlineThickness(0);
	srect.setSize(sf::Vector2f(8,8));
	srect.setOrigin(4,4);

	// Add nodes to their neighbors set
	n1->neighbors.insert(n2);
	n2->neighbors.insert(n1);
	// Add self to nodes' edge sets
	n1->edges.insert(this);
	n2->edges.insert(this);
	// Add self to edge set and quadtree (if they are set)
	if (eset) eset->insert(this);
	if (qtree) qtree->insert(this, srect.getPosition().x, srect.getPosition().y);

	update(); // Will add self to quadtree
}

void Edge::updateBorder()
{
	if (faces == 1)
	{
		rect.setFillColor(sf::Color::Red);
		//rect.setOutlineThickness(1);
	}
	else
	{
		rect.setFillColor(sf::Color::Black);
		//rect.setOutlineThickness(0);
	}
}

void Edge::update()
{
	if (updateDisabled) return;

	if (!n1 || !n2) return;

	// Save current position
	float x = srect.getPosition().x;
	float y = srect.getPosition().y;

	// Update
	float dx = n2->x - n1->x;
	float dy = n2->y - n1->y;
	float rot = atan2(dy, dx) * RADTODEG;
	rect.setSize(sf::Vector2f(std::sqrt(std::abs(dx)*std::abs(dx) + std::abs(dy)*std::abs(dy)), ht*2));
	rect.setPosition(n1->x, n1->y);
	rect.setRotation(rot);
	srect.setPosition(n1->x+(dx)/2, n1->y+(dy)/2);
	srect.setRotation(rot);

	// Move in quadtree
	if (qtree) qtree->move(this, x, y, srect.getPosition().x, srect.getPosition().y);
}

void Edge::move(int dx, int dy)
{
	move((float)dx, (float)dy);
}
void Edge::move(float dx, float dy)
{
	// Move nodes without heavy edge update
	updateDisabled = true;
	n1->move(dx, dy);
	n2->move(dx, dy);
	updateDisabled = false;

	// Move edge and update in quadtree
	float x = srect.getPosition().x;
	float y = srect.getPosition().y;
	rect.move(dx, dy);
	srect.move(dx, dy);
	if (qtree) qtree->move(this, x, y, x+dx, y+dy);
}

bool Edge::operator==(const Edge & rhs) const
{
	return (rhs.n1 == n1 && rhs.n2 == n2)
		|| (rhs.n1 == n2 && rhs.n2 == n1);
}

//
// Static
//
Edge * Edge::createEdge(Node * n1, Node * n2, float thickness)
{
	// If null
	if (!n1 || !n2) return 0;
	// If same
	if (n1 == n2) return 0;
	// If already neighbors
	if (n1->neighbors.find(n2)!=n1->neighbors.end() || n2->neighbors.find(n1)!=n2->neighbors.end()) return 0;

	// Create edge, enforcing order of nodes
	if (n1 < n2) return new Edge(n1, n2, thickness);
	else return new Edge(n2, n1, thickness);
}

bool Edge::destroyEdge(Node * n1, Node * n2)
{
	// If null
	if (!(n1 && n2)) return false;
	// If same
	if (n1 == n2) return false;
	// If not neighbors
	if (n1->neighbors.find(n2)==n1->neighbors.end() && n2->neighbors.find(n1)==n2->neighbors.end()) return false;

	// Find and delete edge
	for (auto it = n1->edges.begin(); it != n1->edges.end(); ++it)
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

Edge * Edge::findEdge(const std::set<Edge*, Edge::Comp> & s, Node * n1, Node * n2)
{
	// If null
	if (!n1 || !n2) return false;
	// If same
	if (n1 == n2) return false;

	Edge * e;
	if (n1 < n2)
		e = new Edge(n1, n2);
	else
		e = new Edge(n2, n1);
	auto it = s.find(e);
	delete e;
	if (it != s.end())
		return *it;
	return 0;
}

void Edge::setEdgeSet(std::set<Edge*> * edgeSet)
{
	eset = edgeSet;
}

void Edge::setQuadTree(QuadTree<Edge*> * quadTree)
{
	qtree = quadTree;
}

bool Edge::intersect(Edge * e1, Edge * e2, float * xret, float * yret)
{
	return Edge::intersect(e1->n1, e1->n2, e2->n1, e2->n2, xret, yret);
}

bool Edge::intersect(Edge * e1, Node * n1, Node * n2, float * xret, float * yret)
{
	return Edge::intersect(e1->n1, e1->n2, n1, n2, xret, yret);
}

bool Edge::intersect(Node * n1, Node * n2, Node * n3, Node * n4, float * xret, float * yret)
{
	float x1 = n1->x,	y1 = n1->y;
	float x2 = n2->x,	y2 = n2->y;
	float x3 = n3->x,	y3 = n3->y;
	float x4 = n4->x,	y4 = n4->y;

	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

	if (d == 0)
	{
		return false;
	}
	else
	{
		// Get the x and y
		float pre = (x1*y2 - y1*x2);
		float post = (x3*y4 - y3*x4);
		float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
		float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;

		// If the point of intersection is not within both line segments, then the segments do not intersect. The
		// intersection point must not be an endpoint, i.e., lines must cross, not just touch. The equality check is
		// there to test for this rare case, which would likely only occur if the floats held identical integral values.
		if ( x <= std::min(x1, x2) || x >= std::max(x1, x2) || x <= std::min(x3, x4) || x >= std::max(x3, x4) ) return false;
		if ( y <= std::min(y1, y2) || y >= std::max(y1, y2) || y <= std::min(y3, y4) || y >= std::max(y3, y4) ) return false;

		// Return the point of intersection
		if (xret != 0 && yret != 0)
		{
			*xret = x;
			*yret = y;
		}
		return true;
	}
}
