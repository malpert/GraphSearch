#pragma once

/*///=====================================================================

	quadtree.h

	Author: Matthew Alpert
	Updated: April 20, 2012

	A templatized quad tree class.

*///======================================================================

#include <assert.h>
#include <vector>
#include <algorithm>
#include <math.h>

#define SFMLDEBUG
#ifdef SFMLDEBUG
#include <SFML/Graphics.hpp>
#endif

template<typename T, int MAX_ITEMS_PER_CELL=6, int MAX_DEPTH=10>
class QuadTree
{
private:

	struct Item
	{
		Item(T data, float x, float y) : data(data), x(x), y(y) {}
		T data;
		float x;
		float y;
	};

	struct AABB
	{
		AABB() : cx(0), cy(0), hw(0), hh(0) {}
		AABB(float cx, float cy, float hw, float hh) : cx(cx), cy(cy), hw(hw), hh(hh) {}
		
		bool contains(float x, float y)
		{
			if (x < cx-hw || y < cy-hh || x >= cx+hw || y >= cy+hh)
				return false;
			return true;
		}

		bool contains(AABB other)
		{
			return (cx-hw <= other.cx-other.hw
				&&	cy-hh <= other.cy-other.hh
				&&	cx+hw >= other.cx+other.hw
				&&	cy+hh >= other.cy+other.hh );
		}

		bool intersects(AABB other)
		{
			return (cx-hw < other.cx+other.hw
				&&	cy-hh < other.cy+other.hh
				&&	cx+hw > other.cx-other.hw
				&&	cy+hh > other.cy-other.hh );
		}

		float cx; // center-x
		float cy; // center-y
		float hw; // half-width
		float hh; // half-height
	};

public:

	//
	// QuadTree
	//
	// Constructs a QuadTree cell based on the given lower and upper bounds.
	//
	QuadTree(float x1, float y1, float x2, float y2, int depth = 0)
		: depth(depth), hasChildren(0), c1(0), c2(0), c3(0), c4(0)
	{
		aabb.hw = std::fabs(x1-x2) / 2;
		aabb.hh = std::fabs(y1-y2) / 2;
		aabb.cx = std::min(x1,x2) + aabb.hw;
		aabb.cy = std::min(y1,y2) + aabb.hh;
	}

	~QuadTree()
	{
		if (c1) delete c1;
		if (c2) delete c2;
		if (c3) delete c3;
		if (c4) delete c4;
	}

	//
	// insert
	//
	// Inserts data into the appropriate cell. Does not check for uniqueness.
	//
	void insert(T data, int x, int y)
	{
		insert(data, (float)x, (float)y);
	}
	void insert(T data, float x, float y)
	{
		if (!aabb.contains(x,y))
			assert(!"QuadTree::insert: bounds");

		if (!hasChildren)
		{
			if (depth >= MAX_DEPTH || items.size() < MAX_ITEMS_PER_CELL)
				items.push_back(Item(data, x, y));
			else
				subdivide();
		}

		if (hasChildren)
		{
			if (c1->aabb.contains(x, y))
				c1->insert(data, x, y);
			else if (c2->aabb.contains(x, y))
				c2->insert(data, x, y);
			else if (c3->aabb.contains(x, y))
				c3->insert(data, x, y);
			else if (c4->aabb.contains(x, y))
				c4->insert(data, x, y);
			else
				assert(!"QuadTree::insert: child bounds");
		}
	}

	//
	// getAllItems
	//
	// Pushes all items bound by this cell into the vector and returns the
	// number of items.
	//
	int getAllItems(std::vector<T> & ret)
	{
		ret.reserve(ret.size() + numItems());
		getAllItemsWork(ret);
		return ret.size();
	}

	//
	// queryRegion
	//
	// Pushes all items bounded by the intersection of this cell and the given
	// region into the vector and returns the number of items.
	//
	int queryRegion(int x1, int y1, int x2, int y2, std::set<T> & ret)
	{
		return queryRegion((float)x1, (float)y1, (float)x2, (float)y2, ret);
	}
	int queryRegion(float x1, float y1, float x2, float y2, std::set<T> & ret)
	{
		std::vector<T> v;
		queryRegion(x1, y1, x2, y2, v);
		ret.insert(v.begin(), v.end());
		return ret.size();
	}
	int queryRegion(int x1, int y1, int x2, int y2, std::vector<T> & ret)
	{
		return queryRegion((float)x1, (float)y1, (float)x2, (float)y2, ret);
	}
	int queryRegion(float x1, float y1, float x2, float y2, std::vector<T> & ret)
	{
		AABB region;
		region.hw = std::fabs(x1-x2) / 2;
		region.hh = std::fabs(y1-y2) / 2;
		region.cx = std::min(x1,x2) + region.hw;
		region.cy = std::min(y1,y2) + region.hh;
		queryRegion(region, ret);
		return ret.size();
	}

	//
	// erase
	//
	// Searches all items bounded by this cell and erases the first one that
	// equals the argument, if any.
	//
	bool erase(T data)
	{
		bool canUnify = false;
		return erase(data, canUnify);
	}

	//
	// erase
	//
	// Traverses down to find the leaf cell that contains the given point, then
	// deletes the first item in that cell that equals the argument, if any.
	//
	bool erase(T data, int x, int y)
	{
		return erase(data, (float)x, (float)y);
	}
	bool erase(T data, float x, float y)
	{
		bool canUnify = false;
		return erase(data, x, y, canUnify);
	}

	//
	// erase
	//
	// Erases all items that equal the argument, if any, that are bounded by
	// the intersection of this cell and the given region.
	//
	int erase(T data, int x1, int y1, int x2, int y2)
	{
		return erase(data, (float)x1, (float)y1, (float)x2, (float)y2);
	}
	int erase(T data, float x1, float y1, float x2, float y2)
	{
		AABB region;
		region.hw = std::fabs(x1-x2) / 2;
		region.hh = std::fabs(y1-y2) / 2;
		region.cx = std::min(x1,x2) + region.hw;
		region.cy = std::min(y1,y2) + region.hh;
		bool canUnify = false;
		return erase(data, region, canUnify);
	}

	//
	// contains
	//
	// Returns whether the given point is contained within this cell.
	//
	bool contains(int x, int y)
	{
		return contains((float)x, (float)y);
	}
	bool contains(float x, float y)
	{
		return aabb.contains(x, y);
	}

	//
	// numItems
	//
	// Returns the number of items bound by this cell by recursively checking
	// all child cells.
	//
	int numItems()
	{
		int ret = items.size();
		if (hasChildren)
		{
			ret += c1->numItems();
			ret += c2->numItems();
			ret += c3->numItems();
			ret += c4->numItems();
		}
		return ret;
	}

	//
	// numCells
	//
	// Returns the number of cells bound by and including this cell by
	// recursively checking all child cells.
	//
	int numCells()
	{
		int ret = 1;
		if (hasChildren)
		{
			ret += c1->numCells();
			ret += c2->numCells();
			ret += c3->numCells();
			ret += c4->numCells();
		}
		return ret;
	}

#ifdef SFMLDEBUG
	//
	// draw
	//
	// Recursively draws the QuadTree to the given RenderWindow.
	//
	void draw(sf::RenderWindow & rw)
	{
		sf::RectangleShape rect(sf::Vector2f(aabb.hw*2, aabb.hh*2));
		rect.setPosition(aabb.cx-aabb.hw, aabb.cy-aabb.hh);
		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineThickness(1);
		rect.setOutlineColor(sf::Color::Black);
		rw.draw(rect);
		if (hasChildren)
		{
			c1->draw(rw);
			c2->draw(rw);
			c3->draw(rw);
			c4->draw(rw);
		}
	}
#endif

private:

	void getAllItemsWork(std::vector<T> & ret)
	{
		if (hasChildren)
		{
			c1->getAllItemsWork(ret);
			c2->getAllItemsWork(ret);
			c3->getAllItemsWork(ret);
			c4->getAllItemsWork(ret);
		}
		else
		{
			for (size_t i = 0; i < items.size(); ++i)
				ret.push_back(items[i].data);
		}
	}

	int getAllItems(std::vector<Item> & ret)
	{
		ret.reserve(ret.size() + numItems());
		getAllItemsWork(ret);
		return ret.size();
	}

	void getAllItemsWork(std::vector<Item> & ret)
	{
		if (hasChildren)
		{
			c1->getAllItemsWork(ret);
			c2->getAllItemsWork(ret);
			c3->getAllItemsWork(ret);
			c4->getAllItemsWork(ret);
		}
		else
		{
			ret.insert(ret.end(), items.begin(), items.end());
		}
	}

	void queryRegion(AABB region, std::vector<T> & ret)
	{
		if (region.contains(aabb))
		{
			getAllItems(ret);
		}
		else if (region.intersects(aabb))
		{
			if (hasChildren)
			{
				c1->queryRegion(region, ret);
				c2->queryRegion(region, ret);
				c3->queryRegion(region, ret);
				c4->queryRegion(region, ret);
			}
			else
			{
				for (size_t i = 0; i < items.size(); ++i)
				{
					if (region.contains(items[i].x, items[i].y)) ret.push_back(items[i].data);
				}
			}
		}
	}

	//
	// Erase first match
	//
	bool erase(T data, bool & canUnify)
	{
		if (hasChildren)
		{
			bool deleted = false;

			if (c1->erase(data, canUnify)) deleted = true;
			else if (c2->erase(data, canUnify)) deleted = true;
			else if (c3->erase(data, canUnify)) deleted = true;
			else if (c4->erase(data, canUnify)) deleted = true;
			else return false;

			if (canUnify)
			{
				if (numItems() <= MAX_ITEMS_PER_CELL/2)
				{
					unify();
					return true;
				}
				else
				{
					canUnify = false;
					return true;
				}
			}
			return deleted;
		}
		else
		{
			for (std::vector<Item>::iterator it = items.begin(); it != items.end(); ++it)
			{
				if (it->data == data)
				{
					items.erase(it); // invalidates iterators
					canUnify = true;
					return true;
				}
			}
			return false;
		}
	}

	//
	// Go to lowest cell bounding point and erase first match
	//
	bool erase(T data, float x, float y, bool & canUnify)
	{
		if (!aabb.contains(x,y))
			assert(!"QuadTree::erase: bounds");

		if (hasChildren)
		{
			bool deleted = false;

			if (c1->aabb.contains(x,y)) deleted = c1->erase(data, x, y, canUnify);
			else if (c2->aabb.contains(x,y)) deleted = c2->erase(data, x, y, canUnify);
			else if (c3->aabb.contains(x,y)) deleted = c3->erase(data, x, y, canUnify);
			else if (c4->aabb.contains(x,y)) deleted = c4->erase(data, x, y, canUnify);
			else return false;

			if (canUnify)
			{
				if (numItems() <= MAX_ITEMS_PER_CELL/2)
				{
					unify();
					return true;
				}
				else
				{
					canUnify = false;
					return true;
				}
			}
			return deleted;
		}
		else
		{
			for (std::vector<Item>::iterator it = items.begin(); it != items.end(); ++it)
			{
				if (it->data == data)
				{
					items.erase(it); // invalidates iterators
					canUnify = true;
					return true;
				}
			}
			return false;
		}
	}

	//
	// Erase all matches in region
	//
	int erase(T data, AABB region, bool & canUnify)
	{
		int deleted = 0;
		if (hasChildren)
		{
			deleted += c1->erase(data, region, canUnify);
			deleted += c2->erase(data, region, canUnify);
			deleted += c3->erase(data, region, canUnify);
			deleted += c4->erase(data, region, canUnify);

			if (canUnify)
			{
				if (numItems() <= MAX_ITEMS_PER_CELL/2)
					unify();
				else
					canUnify = false;
			}
			return deleted;
		}
		else
		{
			std::vector<Item>::iterator it = items.begin();
			while (it != items.end())
			{
				if (it->data == data)
				{
					canUnify = true;
					++deleted;
					it = items.erase(it);
				}
				else
				{
					++it;
				}

			}
			return deleted;
		}
	}

	void subdivide()
	{
		c1 = new QuadTree(aabb.cx, aabb.cy, aabb.cx+aabb.hw, aabb.cy+aabb.hh, depth+1); // x+ y+
		c2 = new QuadTree(aabb.cx, aabb.cy, aabb.cx+aabb.hw, aabb.cy-aabb.hh, depth+1); // x+ y-
		c3 = new QuadTree(aabb.cx, aabb.cy, aabb.cx-aabb.hw, aabb.cy+aabb.hh, depth+1); // x- y+
		c4 = new QuadTree(aabb.cx, aabb.cy, aabb.cx-aabb.hw, aabb.cy-aabb.hh, depth+1); // x- y-

		for (size_t i = 0; i < items.size(); ++i)
		{
			T data = items[i].data;
			float x = items[i].x;
			float y = items[i].y;

			if (c1->aabb.contains(x, y))
				c1->insert(data, x, y);
			else if (c2->aabb.contains(x, y))
				c2->insert(data, x, y);
			else if (c3->aabb.contains(x, y))
				c3->insert(data, x, y);
			else if (c4->aabb.contains(x, y))
				c4->insert(data, x, y);
			else
				assert(!"QuadTree::subdivide");
		}

		items.clear();
		hasChildren = true;
	}

	void unify()
	{
		getAllItems(items);
		delete c1;
		delete c2;
		delete c3;
		delete c4;
		c1 = 0;
		c2 = 0;
		c3 = 0;
		c4 = 0;
		hasChildren = false;
	}

	AABB aabb;
	int depth;
	bool hasChildren;
	QuadTree * c1; // x+ y+
	QuadTree * c2; // x+ y-
	QuadTree * c3; // x- y+
	QuadTree * c4; // x- y-
	std::vector<Item> items;
};
