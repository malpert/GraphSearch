#pragma once

#include "node.h"
#include "edge.h"
#include <vector>

class Selection : public std::vector<Node*>
{
public:

	Selection(int selectionRange, int globalXMin, int globalYMin, int globalXMax, int globalYMax)
		: range(selectionRange), gxmin((float)globalXMin), gymin((float)globalYMin), gxmax((float)globalXMax), gymax((float)globalYMax), xmin(0), ymin(0), xmax(0), ymax(0) {}

	Selection(int selectionRange, float globalXMin, float globalYMin, float globalXMax, float globalYMax)
		: range(selectionRange), gxmin(globalXMin), gymin(globalYMin), gxmax(globalXMax), gymax(globalYMax), xmin(0), ymin(0), xmax(0), ymax(0) {}

	void clearSelection()
	{
		for (size_t i = 0; i < size(); ++i)
			(*this)[i]->deselect();
		clear();
	}

	void moveSelection(int x, int y)
	{
		moveSelection((float)x, (float)y);
	}
	void moveSelection(float x, float y)
	{
		if (empty() || xmin+x <= gxmin || ymin+y <= gymin || xmax+x >= gxmax || ymax+y >= gymax) return;

		for (size_t i = 0; i < size(); ++i)
		{
			(*this)[i]->move(x, y);
			for (std::set<Edge*>::iterator jt = (*this)[i]->edges.begin(); jt != (*this)[i]->edges.end(); ++jt)
			{
				(*jt)->update();
			}
		}
		moveSelectionBounds(x, y);
	}

	void updateSelectionBounds(Node* n)
	{
		if (size() == 1)
		{
			xmin = xmax = n->x;
			ymin = ymax = n->y;
		}
		else
		{
			if (n->x < xmin) xmin = n->x;
			if (n->y < ymin) ymin = n->y;
			if (n->x > xmax) xmax = n->x;
			if (n->y > ymax) ymax = n->y;
		}
	}

	void resetSelectionBounds()
	{
		if (empty()) return;

		xmin = xmax = (*begin())->x;
		ymin = ymax = (*begin())->y;

		for (std::vector<Node*>::iterator i = begin()+1; i != end(); ++i)
		{
			if ((*i)->x < xmin) xmin = (*i)->x;
			if ((*i)->y < ymin) ymin = (*i)->y;
			if ((*i)->x > xmax) xmax = (*i)->x;
			if ((*i)->y > ymax) ymax = (*i)->y;
		}
	}

	void moveSelectionBounds(float x, float y)
	{
		xmin += x;
		ymin += y;
		xmax += x;
		ymax += y;
	}

	int getRange()
	{
		return range;
	}

private:

	int range;
	float gxmin;
	float gymin;
	float gxmax;
	float gymax;
	float xmin;
	float ymin;
	float xmax;
	float ymax;
};