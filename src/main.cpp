#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <math.h>
#include <algorithm>
#include <stack>
#include <fstream>

#include <Windows.h>

#include "node.h"
#include "edge.h"
#include "face.h"
#include "selection.h"
#include "quadtree.h"

std::vector<std::string> split( const std::string & s, const std::string & pat )
{
	std::vector<std::string> v;
	// if pattern is emptry string, pushback entire string and return
	if( pat == "" )
	{
		v.push_back( s );
		return v;
	}

	std::string::size_type next, last;
	next = 0;
	last = 0;

	// loop until pattern not found
	while( next != std::string::npos )
	{
		// find pattern
		next = s.find(pat, last);
		if( next != std::string::npos )
		{
			// push back substr between last pattern and new pattern
			v.push_back( s.substr(last, next-last) );
			// next search will start from right after last pattern
			last = next + pat.size();
		}
	}

	// last pattern found, push back last substr
	v.push_back( s.substr(last) );

	return v;
}

int main()
{
	//
	// Check if console available
	//
	bool hasConsole = false;
	HWND console = GetConsoleWindow();
	if (console) hasConsole = true;

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
	std::set<Edge*> visibility;

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
	int mousex1 = 0;
	int mousey1 = 0;
	int mousexp = 0;
	int mouseyp = 0;
	int mousex2 = 0;
	int mousey2 = 0;
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
						for (auto it = selection.begin(); it != selection.end(); ++it)
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
							auto i1 = selection.begin(), i2 = selection.begin();
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
				else if(Event.key.code == sf::Keyboard::I) // I
				{
					//
					// Test for intersection
					//
					if (edges.size() == 2)
					{
						auto it = edges.begin();
						Edge * e1 = *it;
						++it;
						Edge * e2 = *it;
						float x = 0, y = 0;
						bool intersect = Edge::intersect(e1, e2, &x, &y);
						std::cout << intersect << ' ' << x << ' ' << y << std::endl;
					}
				}
				else if (Event.key.code == sf::Keyboard::R) // R
				{
					if (keyCtrlDown)
					{
						//
						// Reset
						//
						std::vector<Node*> v(nodes.begin(), nodes.end());
						for (auto it = v.begin(); it != v.end(); ++it)
							delete *it;
						selection.clear();
						faces.clear();
						visibility.clear();
						locked = false;
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
					int edgesBefore = edges.size();

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

						// Update edges' status (mark border edges)
						for (auto it = edges.begin(); it != edges.end(); ++it)
						{
							(*it)->updateBorder();
						}

						//
						// Build visibility graph
						//

						// Get stack of nodes
						std::deque<Node*> nodestack(nodes.begin(), nodes.end());

						// Until empty, pop node off stack...
						while (!nodestack.empty())
						{
							Node * n1 = nodestack.back();
							nodestack.pop_back();
							
							// For each node remaining on stack...
							for (size_t i = 0; i < nodestack.size(); ++i)
							{
								// New visibility edges must overlap at least
								// one internal edge and no border edges.
								bool good = false;

								Node * n2 = nodestack[i];

								// For each edge...
								for (auto jt = edges.begin(); jt != edges.end(); ++jt)
								{
									// Check for intersection between n1, n2, and edge
									if (Edge::intersect(*jt, n1, n2))
									{
										if ((*jt)->faces != 1)
										{
											good = true;
										}
										else if ((*jt)->faces == 1)
										{
											good = false;
											break;
										}
									}
								}

								// If edge between n1 and n2 is good
								if (good)
								{
									// Create new edge
									Edge * e = Edge::createEdge(n1, n2, 1);
									if (e)
									{
										e->srect.setFillColor(sf::Color::Transparent);
										visibility.insert(e);
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

						// Delete visibility edges
						for (auto it = visibility.begin(); it != visibility.end(); ++it)
							delete *it;
						visibility.clear();

						// Update edges' status (un-mark border edges)
						for (auto it = edges.begin(); it != edges.end(); ++it)
						{
							(*it)->updateBorder();
						}
					}


					if (locked)
					{
						std::cout << "Faces=" << faces.size() << '\n';
						std::cout << "Iterations=" << iterations << '\n';
						std::cout << "Edges before=" << edgesBefore << '\n';
						std::cout << "New edges=" << ((int)edges.size() - edgesBefore) << '\n';
						std::cout << "Edges after=" << edges.size() << std::endl;
					}
				}
				else if(Event.key.code == sf::Keyboard::S) // S
				{
					if (keyCtrlDown)
					{
						//
						// Save graph
						//
						std::map<Node*,int> m;
						int id = 0;
						for (auto it = nodes.begin(); it != nodes.end(); ++it)
						{
							m[*it] = id++;
						}

						std::vector<std::pair<int, int>> v;
						for (auto it = edges.begin(); it != edges.end(); ++it)
						{
							v.push_back(std::make_pair(m[(*it)->n1], m[(*it)->n2]));
						}


						std::string filename = "graph.dat";
						std::ofstream fout;

						if (!keyShiftDown && hasConsole)
						{
							SetForegroundWindow(console);
							std::cout << "Save graph file name (blank for graph.dat): ";
							std::string line;
							std::getline(std::cin, line);
							if (line.size()) filename = line;
							SetForegroundWindow(App.getSystemHandle());
						}

						std::cout << "Saving graph \"" + filename + "\"" << std::endl;
						fout.open("../graphs/" + filename);

						if (!fout.is_open())
						{
							std::cerr << "\nError opening file for write: \"" + filename + "\"" << std::endl;
						}
						else
						{
							for (auto it = m.begin(); it != m.end(); ++it)
							{
								fout << "v " << (*it).second << ' ' << (*it).first->x << ' ' << (*it).first->y << '\n';
							}

							for (auto it = v.begin(); it != v.end(); ++it)
							{
								fout << "e " << it->first << ' ' << it->second << '\n';
							}
						}
						fout.close();

						std::cout << "Nodes=" << m.size() << "  Edges=" << v.size() << std::endl;
					}
				}
				else if(Event.key.code == sf::Keyboard::L) // L
				{
					if (keyCtrlDown)
					{
						//
						// Load graph
						//
						std::string filename = "graph.dat";
						std::ifstream fin;

						if (!keyShiftDown && hasConsole)
						{
							SetForegroundWindow(console);
							std::cout << "Load graph file name (blank for graph.dat): ";
							std::string line;
							std::getline(std::cin, line);
							if (line.size()) filename = line;
							SetForegroundWindow(App.getSystemHandle());
						}

						std::cout << "Loading graph \"" + filename + "\"" << std::endl;
						fin.open("../graphs/" + filename);

						if (!fin.is_open())
						{
							std::cerr << "Error opening file for read: \"" + filename + "\"" << std::endl;
						}
						else
						{
							std::map<int, Node*> m;
							int edges = 0;
							std::string line;
							while (std::getline(fin, line))
							{
								auto v = split(line, " ");
								if (v.empty())
								{
									continue;
								}
								else if (v[0] == "v")
								{
									// Nodes
									int id = std::atoi(v[1].c_str());
									float x = (float)std::atof(v[2].c_str());
									float y = (float)std::atof(v[3].c_str());
									m[id] = new Node(x,y);
								}
								else if (v[0] == "e")
								{
									// Edges
									int id1 = std::atoi(v[1].c_str());
									int id2 = std::atoi(v[2].c_str());
									Edge::createEdge(m[id1], m[id2]);
									++edges;
								}
								else
								{
									continue;
								}
							}
							std::cout << "Nodes=" << m.size() << "  Edges=" << edges << std::endl;
						}
						fin.close();
					}
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
					mousex1 = mousexp = Event.mouseButton.x;
					mousey1 = mouseyp = Event.mouseButton.y;

					if (keySpaceDown) // Space
					{
						
					}
					else if (keyShiftDown && keyAltDown) // Shift and Alt
					{
						//
						// Extrude
						//
						if (selection.size() == 2)
						{
							auto it = selection.begin();
							Node * n1 = *it;
							++it;
							Node * n2 = *it;
							selection.clearSelection();
							
							Node * n3 = 0;
							Node * n4 = 0;

							// Check if mouse over edge
							std::vector<Edge*> v;
							if (qte.queryRegion(Event.mouseButton.x+selection.getRange(), Event.mouseButton.y+selection.getRange(), Event.mouseButton.x-selection.getRange(), Event.mouseButton.y-selection.getRange(), v))
							{
								// Extrude from edge and connect to existing edge
								Edge * e = v[0];
								n3 = e->n1;
								n4 = e->n2;

								if (!Edge::intersect(n1, n3, n2, n4))
								{
									Edge::createEdge(n1, n3);
									Edge::createEdge(n2, n4);
									if (n1->neighbors.size()+n4->neighbors.size() <= n2->neighbors.size()+n3->neighbors.size())
										Edge::createEdge(n1, n4);
									else
										Edge::createEdge(n2, n3);
								}
								else
								{
									Edge::createEdge(n1, n4);
									Edge::createEdge(n2, n3);
									if (n1->neighbors.size()+n3->neighbors.size() <= n2->neighbors.size()+n4->neighbors.size())
										Edge::createEdge(n1, n3);
									else
										Edge::createEdge(n2, n4);
								}
							}
							else
							{
								// Extrude from edge to new edge at mouse
								float x = (n1->x + n2->x) / 2;
								float y = (n1->y + n2->y) / 2;
								float dx = mousex1 - x;
								float dy = mousey1 - y;

								n3 = new Node(n1->x + dx, n1->y + dy);
								n4 = new Node(n2->x + dx, n2->y + dy);
								Edge::createEdge(n3, n4);

								Edge::createEdge(n1, n3);
								Edge::createEdge(n2, n4);
								if (n1->neighbors.size() <= n2->neighbors.size())
									Edge::createEdge(n1, n4);
								else
									Edge::createEdge(n2, n3);

								selection.insertSelection(n3);
								selection.insertSelection(n4);
							}



							mouseDownOnSelection = true;
						}
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
									for (auto it = selection.begin(); it != selection.end(); ++it)
										Edge::destroyEdge(n, *it);
								}
								else
								{
									Node * n = v[0];
									// Add edge between clicked node and all selected nodes
									for (auto it = selection.begin(); it != selection.end(); ++it)
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
									for (auto it = selection.begin(); it != selection.end(); ++it)
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
							qtn.queryRegion(mousex1, mousey1, mousex2, mousey2, v);
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
							qtn.queryRegion(mousex1, mousey1, mousex2, mousey2, v);
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
				mousex2 = Event.mouseMove.x;
				mousey2 = Event.mouseMove.y;
				 
				//
				// Left
				//
				if (mouseLeftDown)
				{
					if (mouseDownOnSelection || keySpaceDown)
					{
						mouseDragMoving = true;
						selection.moveSelection(mousex2-mousexp, mousey2-mouseyp);
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

				mousexp = mousex2;
				mouseyp = mousey2;
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
		qtn.draw(App);
		//qte.draw(App);

		// Draw edges
		for (auto it = edges.begin(); it != edges.end(); ++it)
		{
			App.draw((*it)->rect);
			App.draw((*it)->srect);
		}

		// Draw nodes
		for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			App.draw((*it)->circ);
			//std::cout << "n=" << (*it)->neighbors.size() << " e=" << (*it)->edges.size() << std::endl;
		}

		// Draw drag select
		if (mouseDragSelecting)
		{
			sf::RectangleShape rect(sf::Vector2f((float)std::abs(mousex1-mousex2), (float)std::abs(mousey1-mousey2)));
			rect.setPosition((float)std::min(mousex1,mousex2), (float)std::min(mousey1,mousey2));
			rect.setFillColor(sf::Color::Transparent);
			rect.setOutlineThickness(1);
			rect.setOutlineColor(sf::Color::Black);
			App.draw(rect);
		}

		//std::cout << "n=" << nodes.size() << " qn=" << qtn.numItems() << " e=" << edges.size() << " qe=" << qte.numItems() << std::endl;
		//std::cout << "s=" << selection.size() << " v=" << visibility.size() << " f=" << faces.size() << std::endl;

		// Update the window
		App.display();
    }

    return EXIT_SUCCESS;
}
