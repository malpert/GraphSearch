#pragma once

#include "node.h"
#include "edge.h"
#include <vector>

class Selection : public std::set<Node*>
{
public:

	Selection(int selectionRange, int globalXMin, int globalYMin, int globalXMax, int globalYMax)
		: range(selectionRange), gxmin((float)globalXMin), gymin((float)globalYMin), gxmax((float)globalXMax), gymax((float)globalYMax), xmin(0), ymin(0), xmax(0), ymax(0) {}

	Selection(int selectionRange, float globalXMin, float globalYMin, float globalXMax, float globalYMax)
		: range(selectionRange), gxmin(globalXMin), gymin(globalYMin), gxmax(globalXMax), gymax(globalYMax), xmin(0), ymin(0), xmax(0), ymax(0) {}

	int getRange()
	{
		return range;
	}

	void insertSelection(Node * n)
	{
		insert(n);
		updateSelectionBounds(n);
		n->select();
	}

	void insertSelection(std::vector<Node*> v)
	{
		for (std::vector<Node*>::iterator it = v.begin(); it != v.end(); ++it)
		{
			insertSelection(*it);
		}
	}

	void eraseSelection(Node * n)
	{
		erase(n);
		resetSelectionBounds();
		n->deselect();
	}

	void eraseSelection(std::vector<Node*> v)
	{
		for (std::vector<Node*>::iterator it = v.begin(); it != v.end(); ++it)
		{
			erase(*it);
			(*it)->deselect();
		}
		resetSelectionBounds();
	}

	void clearSelection()
	{
		for (Selection::iterator it = begin(); it != end(); ++it)
			(*it)->deselect();
		clear();
	}

	void moveSelection(int x, int y)
	{
		moveSelection((float)x, (float)y);
	}
	void moveSelection(float x, float y)
	{
		if (empty() || xmin+x <= gxmin || ymin+y <= gymin || xmax+x >= gxmax || ymax+y >= gymax) return;
		for (Selection::iterator it = begin(); it != end(); ++it)
			(*it)->move(x, y);
		moveSelectionBounds(x, y);
	}

private:

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

		for (Selection::iterator it = begin(); it != end(); ++it)
		{
			if ((*it)->x < xmin) xmin = (*it)->x;
			if ((*it)->y < ymin) ymin = (*it)->y;
			if ((*it)->x > xmax) xmax = (*it)->x;
			if ((*it)->y > ymax) ymax = (*it)->y;
		}
	}

	void moveSelectionBounds(float x, float y)
	{
		xmin += x;
		ymin += y;
		xmax += x;
		ymax += y;
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