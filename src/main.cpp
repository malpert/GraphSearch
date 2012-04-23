#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <math.h>
#include <algorithm>

#include "node.h"
#include "edge.h"
#include "selection.h"
#include "quadtree.h"

int main()
{
	//
    // Create the main rendering window
	//
    sf::RenderWindow App(sf::VideoMode(800, 800, 32), "Graph Search");

	//
	// Create lines
	//
	std::vector<sf::RectangleShape*> lines;
	for (float i = -1; i < App.getSize().x; i += float(App.getSize().x) / 20)
	{
		sf::RectangleShape* r = new sf::RectangleShape(sf::Vector2f(2, float(App.getSize().y)));
		r->setPosition(i, 0);
		r->setFillColor(sf::Color::Black);
		lines.push_back(r);
	}
	for (float i = -1; i < App.getSize().y; i += float(App.getSize().y) / 20)
	{
		sf::RectangleShape* r = new sf::RectangleShape(sf::Vector2f(float(App.getSize().x), 2));
		r->setFillColor(sf::Color::Black);
		r->setPosition(0, i);
		lines.push_back(r);
	}

	//
	// Graph
	//
	std::set<Node*> nodes;
	QuadTree<Node*,4> qtn(0, 0, (float)App.getSize().x, (float)App.getSize().y);
	std::set<EdgePtr> edges;
	QuadTree<EdgePtr,4> qte(0, 0, (float)App.getSize().x, (float)App.getSize().y);
	Edge edge;

	//
	// Selection
	//
	Selection selection(6, 0, 0, App.getSize().x, App.getSize().y);

	//
	// State
	//
	bool keyAltDown = false;
	bool keyCtrlDown = false;
	bool keyShiftDown = false;
	bool mouseLeftDown = false;
	bool mouseRightDown = false;
	bool mouseDragMoving = false;
	bool mouseDragSelecting = false;
	bool mouseDownOnSelection = false;
	int dragx1 = 0;
	int dragy1 = 0;
	int prevx = 0;
	int prevy = 0;
	int dragx2 = 0;
	int dragy2 = 0;

	//
    // Start game loop
	//
    while (App.isOpen())
    {
		//
        // Process events
		//

        sf::Event Event;
		while (App.pollEvent(Event))
        {
            // Close window : exit
            if (Event.type == sf::Event::Closed)
                App.close();
			
			//
			// Key pressed
			//
            if (Event.type == sf::Event::KeyPressed)
            {
				if (Event.key.code == sf::Keyboard::Escape) // Escape
				{
                    App.close();
				}
				else if (Event.key.code == sf::Keyboard::F1) // F1
                {
                    sf::Image Screen = App.capture();
                    Screen.saveToFile("../media/screenshot.jpg");
                }
				else if (Event.key.code == sf::Keyboard::RAlt) // RAlt
				{
					keyAltDown = true;
				}
				else if (Event.key.code == sf::Keyboard::RControl) // RControl
				{
					keyCtrlDown = true;
				}
				else if (Event.key.code == sf::Keyboard::RShift) // RShift
				{
					keyShiftDown = true;
				}
				else if (Event.key.code == sf::Keyboard::Delete) // Delete
				{
					// Delete selected nodes
					for (size_t i = 0; i < selection.size(); ++i)
					{
						qtn.erase(selection[i], selection[i]->x, selection[i]->y);
						nodes.erase(selection[i]);
						delete selection[i];
					}
					selection.clear();
				}
			}

			//
			// Key released
			//
			if (Event.type == sf::Event::KeyReleased)
			{
				if (Event.key.code == sf::Keyboard::RAlt) // RAlt
				{
					keyAltDown = false;
				}
				else if (Event.key.code == sf::Keyboard::RControl) // RControl
				{
					keyCtrlDown = false;
				}
				else if (Event.key.code == sf::Keyboard::RShift) // RShift
				{
					keyShiftDown = false;
				}
			}

			//
			// Mouse pressed
			//
			if (Event.type == sf::Event::MouseButtonPressed)
			{
				//
				// Left
				//
				if (Event.mouseButton.button == sf::Mouse::Left)
				{
					mouseLeftDown = true;
					dragx1 = prevx = Event.mouseButton.x;
					dragy1 = prevy = Event.mouseButton.y;

					if (keyCtrlDown) // Ctrl
					{
						// Place node
						if (qtn.contains(Event.mouseButton.x, Event.mouseButton.y))
						{
							Node* n = new Node(Event.mouseButton.x, Event.mouseButton.y);
							nodes.insert(n);
							qtn.insert(n, Event.mouseButton.x, Event.mouseButton.y);
							// Add edges
							for (size_t i = 0; i < selection.size(); ++i)
							{
								EdgePtr e(new Edge(n, selection[i]));
								e->update();
								edges.insert(e);
								qte.insert(e, e->srect.getPosition().x, e->srect.getPosition().y);
								// Check for duplicate edges
							}
							// Update selection
							selection.clearSelection();
							selection.push_back(n);
							selection.updateSelectionBounds(n);
							n->select();
							mouseDownOnSelection = true;
						}
					}
					else if (keyShiftDown) // Shift
					{
						// Check if mouse over nodes
						std::vector<Node*> v;
						if (qtn.queryRegion(Event.mouseButton.x+selection.getRange(), Event.mouseButton.y+selection.getRange(), Event.mouseButton.x-selection.getRange(), Event.mouseButton.y-selection.getRange(), v))
						{
							// Check if there's a node that's not selected
							for (size_t i = 0; i < v.size(); ++i)
							{
								if (!v[i]->isSelected())
								{
									// Add single node to selection
									selection.push_back(v[i]);
									selection.updateSelectionBounds(v[i]);
									v[i]->select();
									mouseDownOnSelection = true;
									break;
								}
							}
						}
					}
					else if (keyAltDown) // Alt
					{
						// Check if mouse over nodes
						std::vector<Node*> v;
						if (qtn.queryRegion(Event.mouseButton.x+selection.getRange(), Event.mouseButton.y+selection.getRange(), Event.mouseButton.x-selection.getRange(), Event.mouseButton.y-selection.getRange(), v))
						{
							// Check if there's a node that's selected
							for (size_t i = 0; i < v.size(); ++i)
							{
								if (v[i]->isSelected())
								{
									// Remove single node from selection
									v[i]->deselect();
									Selection::iterator it = std::find(selection.begin(), selection.end(), v[i]);
									selection.erase(it);
									selection.resetSelectionBounds();
									break;
								}
							}
						}
					}
					else // Else
					{
						// Check if mouse over nodes
						std::vector<Node*> v;
						if (qtn.queryRegion(Event.mouseButton.x+selection.getRange(), Event.mouseButton.y+selection.getRange(), Event.mouseButton.x-selection.getRange(), Event.mouseButton.y-selection.getRange(), v))
						{
							// Check if all nodes not selected
							bool noneSelected = true;
							for (size_t i = 0; i < v.size(); ++i)
							{
								if (v[i]->isSelected())
								{
									noneSelected = false;
									mouseDownOnSelection = true;
									break;
								}
							}
							if (noneSelected)
							{
								// Select single node
								selection.clearSelection();
								selection.push_back(v[0]);
								selection.updateSelectionBounds(v[0]);
								v[0]->select();
								mouseDownOnSelection = true;
							}
						}
						else
						{
							// Clear selection
							selection.clearSelection();
						}
					}
				}

				//
				// Right
				//
				if (Event.mouseButton.button == sf::Mouse::Right)
				{
					mouseRightDown = true;
				}
			}

			//
			// Mouse released
			//
			if (Event.type == sf::Event::MouseButtonReleased)
			{
				//
				// Left
				//
				if (Event.mouseButton.button == sf::Mouse::Left)
				{
					if (mouseDragSelecting)
					{
						if (keyAltDown) // Alt
						{
							// Remove all from selection (that are selected)
							std::vector<Node*> v;
							qtn.queryRegion(dragx1, dragy1, dragx2, dragy2, v);
							std::set<Node*> s(v.begin(), v.end());
							bool erasedSomething = false;
							Selection::iterator it = selection.begin();
							while (it != selection.end())
							{
								if (s.find(*it) != s.end())
								{
									(*it)->deselect();
									it = selection.erase(it);
									erasedSomething = true;
								}
								else
								{
									++it;
								}
							}
							if (erasedSomething) selection.resetSelectionBounds();
						}
						else if (!keyCtrlDown)
						{
							if (!keyShiftDown) // ! Shift
							{
								// Making new selection
								selection.clearSelection();
							}
							// Add all to selection (that aren't already selected)
							std::vector<Node*> v;
							qtn.queryRegion(dragx1, dragy1, dragx2, dragy2, v);
							for (size_t i = 0; i < v.size(); ++i)
							{
								if (!v[i]->isSelected())
								{
									v[i]->select();
									selection.push_back(v[i]);
									selection.updateSelectionBounds(v[i]);
								}
							}
						}
					}

					mouseLeftDown = false;
					mouseDragMoving = false;
					mouseDragSelecting = false;
					mouseDownOnSelection = false;
				}

				//
				// Right
				//
				if (Event.mouseButton.button == sf::Mouse::Right)
				{
					mouseRightDown = false;
				}

			}

			//
			// Mouse moved
			//
			if (Event.type == sf::Event::MouseMoved)
			{
				dragx2 = Event.mouseMove.x;
				dragy2 = Event.mouseMove.y;

				//
				// Left
				//
				if (mouseLeftDown)
				{
					if (!mouseDownOnSelection || keyShiftDown || keyAltDown)
					{
						mouseDragSelecting = true;
					}
					else if (mouseDownOnSelection)
					{
						mouseDragMoving = true;
						for (size_t i = 0; i < selection.size(); ++i)
							qtn.erase(selection[i], selection[i]->x, selection[i]->y);
						selection.moveSelection(dragx2-prevx, dragy2-prevy);
						for (size_t i = 0; i < selection.size(); ++i)
							qtn.insert(selection[i], selection[i]->x, selection[i]->y);
					}
				}

				//
				// Right
				//
				if (mouseRightDown)
				{

				}

				prevx = dragx2;
				prevy = dragy2;
			}
        }

		//
		// Draw
		//

        // Clear the screen
        App.clear(sf::Color(255, 255, 255));

		// Draw lines
		//for (size_t i = 0; i < lines.size(); ++i) App.draw(*(lines[i]));

		// Draw QuadTree
		qtn.draw(App);

		// Draw edges
		for (std::set<EdgePtr>::iterator it = edges.begin(); it != edges.end(); ++it)
		{
			it->p->update();
			App.draw(it->p->rect);
			App.draw(it->p->srect);
		}

		// Draw nodes
		for (std::set<Node*>::iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			App.draw((*it)->circ);
		}

		// Draw drag select
		if (mouseDragSelecting)
		{
			sf::RectangleShape rect(sf::Vector2f((float)std::abs(dragx1-dragx2), (float)std::abs(dragy1-dragy2)));
			rect.setPosition((float)std::min(dragx1,dragx2), (float)std::min(dragy1,dragy2));
			rect.setFillColor(sf::Color::Transparent);
			rect.setOutlineThickness(1);
			rect.setOutlineColor(sf::Color::Black);
			App.draw(rect);
		}

		// Update the window
		App.display();
    }

    return EXIT_SUCCESS;
}
