#include "edge.h"
#include "node.h"
#include "face.h"

#include <math.h>
#include <limits>

std::set<Edge*> * Edge::eset = 0;
QuadTree<Edge*> * Edge::qtree = 0;

Edge::Edge(Node * n1, Node * n2, float thickness) : n1(n1), n2(n2), ht(thickness/2), selected(false), updateDisabled(false), faces(0)
{
	// Assumes nodes ordered by address

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

void Edge::setEdgeSet(std::set<Edge*> * edgeSet)
{
	eset = edgeSet;
}

void Edge::setQuadTree(QuadTree<Edge*> * quadTree)
{
	qtree = quadTree;
}

bool isEqualFloat(float a, float b)
{
    return fabs(a - b) < 0.0001;
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
#if 1

	// Line-segments go from p to p+r and q to q+s.
	float px = n1->x,       py = n1->y;
	float rx = n2->x - px,  ry = n2->y - py;
	float qx = n3->x,       qy = n3->y;
	float sx = n4->x - qx,  sy = n4->y - qy;

	// Lines intersect if (P + t*R = Q + u*S) is solvable for t and u.
	// Our particular 2d vector cross product: V × W = Vx*Wy − Vy*Wx
	// t = ((Q-P) x S) / (R x S)
	// u = ((Q-P) x R) / (R x S)
	float rs = rx*sy - ry*sx;               // R x S
	float qpx = qx - px,    qpy = qy - py;  // Q - P
	float qps = qpx*sy - qpy*sx;            // (Q-P) x S
	float qpr = qpx*ry - qpy*rx;            // (Q-P) x R

	if (rs == 0)
	{
		if (qpr == 0) return true; // collinear
		else return false; // parallel
	}

	float t = qps / rs;                     // t = ((Q-P) x S) / (R x S)
	float u = qpr / rs;                     // u = ((Q-P) x R) / (R x S)
	
	// Restrict intersection to line-segments.
	// Use > to disallow identical enpoints to
	// count as as intersections, else use >=.
	bool ret = (t > 0.f && t < 1.f && u > 0.f && u < 1.f);

	if (ret && xret != 0 && yret != 0)
	{
		// Return intersection point
		*xret = px + rx*t;
		*yret = py + ry*t;
	}

	return ret;

#else

	// E = B-A = ( Bx-Ax, By-Ay )
	// F = D-C = ( Dx-Cx, Dy-Cy ) 
	// P = ( -Ey, Ex )
	// Q = ( -Fy, Fx
	// h = ( (A-C) * P ) / ( F * P )
	// g = ( (B-D) * Q ) / ( E * Q )
	// intersect if 0 < h < 1 and 0 < g < 1
	// intersection point = C + F*h

	float ax = n1->x,  ay = n1->y;
	float bx = n2->x,  by = n2->y;
	float cx = n3->x,  cy = n3->y;
	float dx = n4->x,  dy = n4->y;

	float ex = bx-ax,  ey = by-ay;
	float fx = dx-cx,  fy = dy-cy;
	float px = -ey,    py = ex;
	float denomh = fx*px + fy*py;
	if (denomh == 0) return false;
	float qx = -fy,    qy = fx;
	float denomg = ex*qx + ey*qy;
	if (denomg == 0) return false;
	float numerh = (ax-cx) * px + (ay-cy) * py;
	float h = numerh / denomh;
	float numerg = (bx-dx) * qx + (by-dy) * qy;
	float g = numerg / denomg;

	bool ret = (h > 0.f && h < 1.f && g > 0.f && g < 1.f);

	if (ret && xret != 0 && yret != 0)
	{
		// Return intersection point
		*xret = cx + fx*h;
		*yret = cy + fy*h;
	}

	return ret;

#endif
}
