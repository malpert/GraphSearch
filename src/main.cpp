#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <math.h>

#include "quadtree.h"
#include "node.h"

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
	std::vector<Node*> nodes;
	QuadTree qt(0, 0, (float)App.getSize().x, (float)App.getSize().y);

	//
	// Input state
	//
	bool keyCtrlDown = false;
	bool keyShiftDown = false;
	bool mouseLeftDown = false;
	bool mouseRightDown = false;
	bool showDragSelect = false;
	int dragx1 = 0;
	int dragx2 = 0;
	int dragy1 = 0;
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
			// Keyboard
			//

            // Key pressed
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
			}

			// Key released
			if (Event.type == sf::Event::KeyReleased)
			{
				if (Event.key.code == sf::Keyboard::RControl) // RControl
				{
					keyCtrlDown = false;
				}

				if (Event.key.code == sf::Keyboard::RShift) // RShift
				{
					keyShiftDown = false;
					showDragSelect = false;
				}
			}

			//
			// Mouse
			//

			// Mouse pressed
			if (Event.type == sf::Event::MouseButtonPressed)
			{
				if (Event.mouseButton.button == sf::Mouse::Left) // Left
				{
					mouseLeftDown = true;

					if (!keyShiftDown)
					{
						nodes.push_back(new Node((float)Event.mouseButton.x, (float)Event.mouseButton.y));
						static int i = 0;
						qt.insert(i++, (float)Event.mouseButton.x, (float)Event.mouseButton.y);
					}

					if (keyShiftDown)
					{
						dragx1 = Event.mouseButton.x;
						dragy1 = Event.mouseButton.y;
					}
				}
				if (Event.mouseButton.button == sf::Mouse::Right) // Right
				{
					mouseRightDown = true;
				}
			}

			// Mouse released
			if (Event.type == sf::Event::MouseButtonReleased)
			{
				if (Event.mouseButton.button == sf::Mouse::Left) // Left
				{
					mouseLeftDown = false;
					showDragSelect = false;

					if (keyShiftDown)
					{
						std::vector<int> v;
						qt.queryRegion(dragx1, dragy1, dragx2, dragy2, v);
						std::cout << "size=" << v.size() << ": ";
						for (size_t i = 0; i < v.size(); ++i)
						{
							std::cout << v[i] << ' ';
							qt.erase(v[i], dragx1, dragy1, dragx2, dragy2);
						}
						std::cout << std::endl;
					}
				}
				if (Event.mouseButton.button == sf::Mouse::Right) // Right
				{
					mouseRightDown = false;
				}

			}

			// Mouse move
			if (Event.type == sf::Event::MouseMoved)
			{
				if (mouseLeftDown) // Left
				{
					if (keyShiftDown)
					{
						dragx2 = Event.mouseMove.x;
						dragy2 = Event.mouseMove.y;
						showDragSelect = true;
					}
				}
				if (mouseRightDown) // Right
				{

				}
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
		for (std::vector<Node*>::iterator itr = nodes.begin(); itr != nodes.end(); ++itr)
		{
			App.draw((*itr)->circleShape);
		}

		// Draw drag select
		if (showDragSelect)
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
