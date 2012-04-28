#include "node.h"
#include "edge.h"

std::set<Node*> * Node::nset = 0;
QuadTree<Node*> * Node::qtree = 0;

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
	for (std::set<Edge*>::iterator it = edges.begin(); it != edges.end();)
	{
		Edge * e = *it; // Grab edge before it is erased from its nodes' edge sets.
		++it; // Increment before edge erased from set we're incrementing through.
		delete e; // Edge destructor will erase edge from its nodes' edge sets
		// (one of which we're incrementing through) and erase it from the edge
		// set and the quadtree (if they are set).
	}
	// Erase self from node set and quadtree (if they are set)
	if (nset) nset->erase(this);
	if (qtree) qtree->erase(this, x, y);
}

void Node::init()
{
	circ.setRadius(5);
	circ.setPosition(x-5, y-5);
	circ.setFillColor(sf::Color::Black);
	circ.setOutlineColor(sf::Color::Red);
	circ.setOutlineThickness(0);
	// Add self to node set and quadtree (if they are set)
	if (nset) nset->insert(this);
	if (qtree) qtree->insert(this, x, y);
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

void Node::setPosition(int nx, int ny)
{
	setPosition((float)nx, (float)ny);
}
void Node::setPosition(float nx, float ny)
{
	// Erase self from quadtree
	if (qtree) qtree->erase(this, x, y);

	// Set position
	x = nx;
	y = ny;
	circ.setPosition(nx-5, ny-5);
	for (std::set<Edge*>::iterator it = edges.begin(); it != edges.end(); ++it)
		(*it)->update();

	// Insert self into quadtree
	if (qtree) qtree->insert(this, x, y);
}

void Node::move(int dx, int dy)
{
	move((float)dx, (float)dy);
}
void Node::move(float dx, float dy)
{
	// Move in quadtree
	if (qtree) qtree->move(this, x, y, x+dx, y+dy);

	// Move
	x += dx;
	y += dy;
	circ.move(dx, dy);

	// Update edges
	for (std::set<Edge*>::iterator it = edges.begin(); it != edges.end(); ++it)
		(*it)->update();
}

std::ostream & operator<<(std::ostream & out, const Node & rhs)
{
	out << "(" << rhs.x << ", " << rhs.y << ")";
	return out;
}

void Node::setNodeSet(std::set<Node*> * nodeSet)
{
	nset = nodeSet;
}

void Node::setQuadTree(QuadTree<Node*> * quadTree)
{
	qtree = quadTree;
}
