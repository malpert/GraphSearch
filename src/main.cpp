#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <math.h>
#include <algorithm>

#include "node.h"
#include "edge.h"
#include "face.h"
#include "selection.h"
#include "quadtree.h"

int main()
{
	//
    // Create the main rendering window
	//
	sf::RenderWindow App	( sf::VideoMode
								( std::min(sf::VideoMode::getDesktopMode().width,(unsigned)600)
								, std::min(sf::VideoMode::getDesktopMode().height,(unsigned)600)
								, 32)
							, "Graph Search");

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
	QuadTree<Node*> qtn(0, 0, (float)App.getSize().x, (float)App.getSize().y, 4);
	Node::setNodeSet(&nodes);
	Node::setQuadTree(&qtn);

	std::set<Edge*> edges;
	QuadTree<Edge*> qte(0, 0, (float)App.getSize().x, (float)App.getSize().y, 4);
	Edge::setEdgeSet(&edges);
	Edge::setQuadTree(&qte);

	std::set<Face*> faces;

	//
	// Selection
	//
	Selection selection(6, 0, 0, App.getSize().x, App.getSize().y);

	//
	// State
	//
	bool keySpaceDown = false;
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
	bool locked = false;

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
				else if (Event.key.code == sf::Keyboard::LControl || Event.key.code == sf::Keyboard::RControl) // Control
				{
					keyCtrlDown = true;
				}
				else if (Event.key.code == sf::Keyboard::LShift || Event.key.code == sf::Keyboard::RShift) // Shift
				{
					keyShiftDown = true;
				}
				else if (Event.key.code == sf::Keyboard::LAlt || Event.key.code == sf::Keyboard::RAlt) // Alt
				{
					keyAltDown = true;
				}
				else if (Event.key.code == sf::Keyboard::Space) // Space
				{
					keySpaceDown = true;
				}
				else if (Event.key.code == sf::Keyboard::Back || Event.key.code == sf::Keyboard::Delete) // Backspace or Delete
				{
					if (!locked)
					{
						// Delete selected nodes and their edges
						for (Selection::iterator it = selection.begin(); it != selection.end(); ++it)
						{
							// Node destructor will remove node from node set and quadtree
							// and will delete its edges. Edge destructor will remove edge
							// from its nodes, the edge set, and quadtree.
							delete *it;
						}
						selection.clear();
					}
				}
				else if (Event.key.code == sf::Keyboard::Insert || Event.key.code == sf::Keyboard::BackSlash || Event.key.code == sf::Keyboard::E) // Insert, BackSlash, or E
				{
					if (!locked)
					{
						// Toggle edge on a pair of selected nodes
						if (selection.size() == 2)
						{
							Selection::iterator i1 = selection.begin(), i2 = selection.begin();
							++i2;
							// Factory creates edge only of nodes are not already neighbors.
							// Edges add themselves to edge set, quadtree and its nodes' edge sets.
							if (!Edge::createEdge(*i1, *i2))
							{
								// Edge already exists, so remove it instead
								Edge::destroyEdge(*i1, *i2);
							}
						}
					}
				}
			}

			//
			// Key released
			//
			if (Event.type == sf::Event::KeyReleased)
			{
				if (Event.key.code == sf::Keyboard::LControl || Event.key.code == sf::Keyboard::RControl) // Control
				{
					keyCtrlDown = false;
				}
				else if (Event.key.code == sf::Keyboard::LShift || Event.key.code == sf::Keyboard::RShift) // Shift
				{
					keyShiftDown = false;
				}
				else if (Event.key.code == sf::Keyboard::LAlt || Event.key.code == sf::Keyboard::RAlt) // Alt
				{
					keyAltDown = false;
				}
				else if (Event.key.code == sf::Keyboard::Space) // Space
				{
					keySpaceDown = false;
				}
				else if (Event.key.code == sf::Keyboard::B) // B
				{
					//
					// Find faces, find borders, build visibility graph
					//
					int iterations = 0;

					if (!locked)
					{
						locked = true;

						// Find all triangular faces
						for (auto it = edges.begin(); it != edges.end(); ++it)
						{
							Node *n1, *n2;
							n1 = (*it)->n1;
							n2 = (*it)->n2;

							for (auto jt = n1->neighbors.begin(); jt != n1->neighbors.end(); ++jt)
							{
								++iterations;
								Node * n3 = *jt;

								// Check if neighbors form a face
								if (n2->neighbors.find(n3) != n2->neighbors.end())
								{
									Face * f = Face::createFace(n1, n2, n3);
									if (f)
									{
										faces.insert(f);
									}
								}
							}
						}
					}
					else
					{
						locked = false;

						// Delete all faces
						for (auto it = faces.begin(); it != faces.end(); ++it)
							Face::destoryFace(*it);
						faces.clear();
					}

					// Update edges' status
					for (auto it = edges.begin(); it != edges.end(); ++it)
					{
						(*it)->updateBorder();
					}
					std::cout << "Faces=" << faces.size() << std::endl;
					std::cout << "Iterations=" << iterations << std::endl;
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

					if (keySpaceDown) // Space
					{
						
					}
					else if (keyCtrlDown) // Ctrl
					{
						if (!locked)
						{
							// Check if mouse over node
							std::vector<Node*> v;
							if (qtn.queryRegion(Event.mouseButton.x+selection.getRange(), Event.mouseButton.y+selection.getRange(), Event.mouseButton.x-selection.getRange(), Event.mouseButton.y-selection.getRange(), v))
							{
								if (keyAltDown)
								{
									Node * n = v[0];
									// Remove edges between clicked node and all selected nodes
									for (Selection::iterator it = selection.begin(); it != selection.end(); ++it)
										Edge::destroyEdge(n, *it);
								}
								else
								{
									Node * n = v[0];
									// Add edge between clicked node and all selected nodes
									for (Selection::iterator it = selection.begin(); it != selection.end(); ++it)
										Edge::createEdge(n, *it);
									// If shift key not down, clear selection set
									if (!keyShiftDown) selection.clearSelection();
									// Add to selection
									selection.insertSelection(n);
									mouseDownOnSelection = true;
								}
							}
							else
							{
								// Place node
								if (qtn.contains(Event.mouseButton.x, Event.mouseButton.y))
								{
									// Nodes add themselves to node set and quadtree
									Node * n = new Node(Event.mouseButton.x, Event.mouseButton.y);
									if (keyAltDown) selection.clearSelection();
									// Add edge between new node and all selected nodes
									for (Selection::iterator it = selection.begin(); it != selection.end(); ++it)
									{
										// Factory creates edge only of nodes are not already neighbors.
										// Edges add themselves to edge set, quadtree and their nodes' edge sets.
										Edge::createEdge(n, *it);
									}
									// Update selection
									if (!keyShiftDown) selection.clearSelection();
									// Add to selection
									selection.insertSelection(n);
									mouseDownOnSelection = true;
								}
							}
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
									selection.insertSelection(v[i]);
									mouseDownOnSelection = true;
									break;
								}
							}
						}
						else
						{
							// Check if mouse over edges
							std::vector<Edge*> v;
							if (qte.queryRegion(Event.mouseButton.x+selection.getRange(), Event.mouseButton.y+selection.getRange(), Event.mouseButton.x-selection.getRange(), Event.mouseButton.y-selection.getRange(), v))
							{
								// Check if there's an edge whose nodes are not both selected
								for (size_t i = 0; i < v.size(); ++i)
								{
									if (!v[i]->n1->isSelected() || !v[i]->n2->isSelected())
									{
										// Add edge's nodes to selection
										selection.insertSelection(v[i]->n1);
										selection.insertSelection(v[i]->n2);
										mouseDownOnSelection = true;
									}
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
									selection.eraseSelection(v[i]);
									break;
								}
							}
						}
						else
						{
							// Check if mouse over edges
							std::vector<Edge*> v;
							if (qte.queryRegion(Event.mouseButton.x+selection.getRange(), Event.mouseButton.y+selection.getRange(), Event.mouseButton.x-selection.getRange(), Event.mouseButton.y-selection.getRange(), v))
							{
								// Check if there's an edge with a selected node
								for (size_t i = 0; i < v.size(); ++i)
								{
									if (v[i]->n1->isSelected() || v[i]->n2->isSelected())
									{
										// Remove edge's nodes from selection
										selection.eraseSelection(v[i]->n1);
										selection.eraseSelection(v[i]->n2);
									}
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
								selection.insertSelection(v[0]);
								mouseDownOnSelection = true;
							}
						}
						else
						{
							// Check if mouse over edges
							std::vector<Edge*> v;
							if (qte.queryRegion(Event.mouseButton.x+selection.getRange(), Event.mouseButton.y+selection.getRange(), Event.mouseButton.x-selection.getRange(), Event.mouseButton.y-selection.getRange(), v))
							{
								// Check if an edge's nodes are both in selection
								bool noneSelected = true;
								for (size_t i = 0; i < v.size(); ++i)
								{
									if (v[i]->n1->isSelected() && v[i]->n2->isSelected())
									{
										noneSelected = false;
										mouseDownOnSelection = true;
										break;
									}
								}
								if (noneSelected)
								{
									// Select single edge
									selection.clearSelection();
									selection.insertSelection(v[0]->n1);
									selection.insertSelection(v[0]->n2);
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
							// Remove all from selection
							std::vector<Node*> v;
							qtn.queryRegion(dragx1, dragy1, dragx2, dragy2, v);
							selection.eraseSelection(v);
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
							selection.insertSelection(v);
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
					if (mouseDownOnSelection || keySpaceDown)
					{
						mouseDragMoving = true;
						selection.moveSelection(dragx2-prevx, dragy2-prevy);
					}
					else //if (keyShiftDown || keyAltDown)
					{
						mouseDragSelecting = true;
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
		//sf::Vertex vertices[2] =	{ sf::Vertex(sf::Vector2f(123,231), sf::Color::Blue)
		//							, sf::Vertex(sf::Vector2f(257,249), sf::Color::Red) };
		//App.draw(vertices, 2, sf::Lines);

		// Draw QuadTree
		//qtn.draw(App);
		qte.draw(App);

		// Draw edges
		for (std::set<Edge*>::iterator it = edges.begin(); it != edges.end(); ++it)
		{
			App.draw((*it)->rect);
			App.draw((*it)->srect);
		}

		// Draw nodes
		for (std::set<Node*>::iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			App.draw((*it)->circ);
			//std::cout << "n=" << (*it)->neighbors.size() << " e=" << (*it)->edges.size() << std::endl;
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

		//std::cout << "n=" << nodes.size() << " qn=" << qtn.numItems() << " e=" << edges.size() << " qe=" << qte.numItems() << std::endl;
		//std::cout << selection.size() << std::endl;

		// Update the window
		App.display();
    }

    return EXIT_SUCCESS;
}
