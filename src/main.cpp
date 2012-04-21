#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <math.h>

#include "node.h"
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
	QuadTree<Node*,4> qt(0, 0, (float)App.getSize().x, (float)App.getSize().y);

	//
	// Selection
	//
	//std::vector<Node*> selection;
	int selectRange = 6;
	Selection selection(6, 0, 0, App.getSize().x, App.getSize().y);

	//
	// State
	//
	bool keyCtrlDown = false;
	bool keyShiftDown = false;
	bool mouseLeftDown = false;
	bool mouseRightDown = false;
	bool mouseDragMoving = false;
	bool mouseDragSelecting = false;
	bool mouseClickingSelection = false;
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
                    App.close();

                if (Event.key.code == sf::Keyboard::F1) // F1
                {
                    sf::Image Screen = App.capture();
                    Screen.saveToFile("../media/screenshot.jpg");
                }

				if (Event.key.code == sf::Keyboard::RControl) // RControl
				{
					keyCtrlDown = true;
				}

				if (Event.key.code == sf::Keyboard::RShift) // RShift
				{
					keyShiftDown = true;
				}

				if (Event.key.code == sf::Keyboard::Delete) // Delete
				{
					// Delete selected nodes
					for (size_t i = 0; i < selection.size(); ++i)
					{
						qt.erase(selection[i], selection[i]->x, selection[i]->y);
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
				if (Event.key.code == sf::Keyboard::RControl) // RControl
				{
					keyCtrlDown = false;
				}

				if (Event.key.code == sf::Keyboard::RShift) // RShift
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
						if (qt.contains(Event.mouseButton.x, Event.mouseButton.y))
						{
							Node* n = new Node(Event.mouseButton.x, Event.mouseButton.y);
							nodes.insert(n);
							qt.insert(n, Event.mouseButton.x, Event.mouseButton.y);
							//for (size_t i = 0; i < selection.size(); ++i)
								//selection[i]->deselect();
							//selection.clear();
							selection.clearSelection();
							selection.push_back(n);
							selection.updateSelectionBounds(n);
							n->select();
							mouseClickingSelection = true;
						}
					}
					else if (keyShiftDown) // Shift
					{
						// Check if mouse over nodes
						std::vector<Node*> v;
						if (qt.queryRegion(Event.mouseButton.x+selectRange, Event.mouseButton.y+selectRange, Event.mouseButton.x-selectRange, Event.mouseButton.y-selectRange, v))
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
									mouseClickingSelection = true;
									break;
								}
							}
						}
					}
					else // Else
					{
						// Check if mouse over nodes
						std::vector<Node*> v;
						if (qt.queryRegion(Event.mouseButton.x+selectRange, Event.mouseButton.y+selectRange, Event.mouseButton.x-selectRange, Event.mouseButton.y-selectRange, v))
						{
							// Check if all nodes not selected
							bool noneSelected = true;
							for (size_t i = 0; i < v.size(); ++i)
							{
								if (v[i]->isSelected())
								{
									noneSelected = false;
									mouseClickingSelection = true;
									break;
								}
							}
							if (noneSelected)
							{
								// Select single node
								//for (size_t i = 0; i < selection.size(); ++i)
									//selection[i]->deselect();
								//selection.clear();
								selection.clearSelection();
								selection.push_back(v[0]);
								selection.updateSelectionBounds(v[0]);
								v[0]->select();
								mouseClickingSelection = true;
							}
						}
						else
						{
							// Clear selection
							//for (size_t i = 0; i < selection.size(); ++i)
							//	selection[i]->deselect();
							//selection.clear();
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
					if (keyShiftDown) // Shift
					{
						if (mouseDragSelecting)
						{
							// Add all to selection (that aren't already selected)
							std::vector<Node*> v;
							qt.queryRegion(dragx1, dragy1, dragx2, dragy2, v);
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
					mouseClickingSelection = false;
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
					if (keyShiftDown) // Shift
					{
						mouseDragSelecting = true;
					}
					else if (mouseClickingSelection)
					{
						//for (size_t i = 0; i < selection.size(); ++i)
						//{
						//	qt.erase(selection[i], selection[i]->x, selection[i]->y);
						//	selection[i]->move(dragx2-prevx, dragy2-prevy);
						//	qt.insert(selection[i], selection[i]->x, selection[i]->y);
						//}
						for (size_t i = 0; i < selection.size(); ++i)
							qt.erase(selection[i], selection[i]->x, selection[i]->y);
						selection.moveSelection(dragx2-prevx, dragy2-prevy);
						for (size_t i = 0; i < selection.size(); ++i)
							qt.insert(selection[i], selection[i]->x, selection[i]->y);
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
		qt.draw(App);
		//std::cout << qt.numCells() << "  " << qt.numItems() << std::endl;
		//std::vector<int> v;
		//qt.getAllItems(v);
		//for (size_t i = 0; i < v.size(); ++i)
		//	std::cout << v[i] << ' ';
		//std::cout << std::endl;

		// Draw graph
		for (std::set<Node*>::iterator itr = nodes.begin(); itr != nodes.end(); ++itr)
		{
			App.draw((*itr)->circ);
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
