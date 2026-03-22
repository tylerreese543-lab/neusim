#include "World.h"
#include "neuron.h"
#include "graphics.h"
#include <iostream>
#include <thread>

void get_partitions(std::vector<_2_int_32>& partitions, float x, float y, float view_distance, float cell_size) {
	int min_x = static_cast<int>(floorf((x - view_distance) / cell_size));
	int max_x = static_cast<int>(floorf((x + view_distance) / cell_size));
	int min_y = static_cast<int>(floorf((y - view_distance) / cell_size));
	int max_y = static_cast<int>(floorf((y + view_distance) / cell_size));
	for (int px = min_x; px <= max_x; ++px) {
		for (int py = min_y; py <= max_y; ++py) {
			partitions.push_back(up(px, py));
		}
	}
}
int main(void) {


	if (!glfwInit()) {
		std::cerr << "Error initializing GLFW" << std::endl;
		return -1;
	}


	GLFWwindow* window = glfwCreateWindow(800, 600, "Neural Network Simulation", NULL, NULL);
	if (!window) {
		std::cerr << "Error creating GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	GLenum err = glewInit();
	if (err != GLEW_OK) {

		std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
		return -1;
	}



	srand(time(0));
	neural_network _root;

	//Root should have 7 zones (2 neurons each = 14 total neurons)
	for (int i = 0; i < 14; ++i) {
		_root.add_input_neuron();
	}

	//Interoception (low energy, high energy, high stamina, low stamina)
	_root.add_input_neuron();
	_root.add_input_neuron();
	//Outputs: speed, angular velocity, reproduce
	_root.add_output_neuron();
	_root.add_output_neuron();
	_root.add_output_neuron();
	//Recurrency
	_root.add_input_neuron();
	_root.add_input_neuron();
	_root.add_input_neuron();
	World world;

	char window_name[128];
	int window_width = 0;
	int window_height = 0;

	int TARGET_FPS = 60;
	double last_time = 0;
	float delta = 0;
	int max_food = 10000;
	float zoom = 0.1f;
	bool possessing = false;

	world.organisms.reserve(50000);
	world.foods.reserve(max_food + 1000);
	world.new_organisms.reserve(1000);

	std::vector<Organism*> new_organisms_0;
	std::vector<Organism*> new_organisms_1;
	std::vector<Organism*> new_organisms_2;
	std::vector<Organism*> new_organisms_3;
	std::vector<Organism*> new_organisms_4;
	std::vector<Organism*> new_organisms_5;
	std::vector<Organism*> new_organisms_6;
	std::vector<Organism*> new_organisms_7;
	new_organisms_0.reserve(250);
	new_organisms_1.reserve(250);
	new_organisms_2.reserve(250);
	new_organisms_3.reserve(250);
	new_organisms_4.reserve(250);
	new_organisms_5.reserve(250);
	new_organisms_6.reserve(250);
	new_organisms_7.reserve(250);

	for (int i = 0; i < 10000; ++i) {
			Organism* org = new Organism(randnf() * 300, randnf() * 300, _root);
			org->angle = 3.1415926f * 2.0f * randf();
			org->brain.alter_random(32);
			org->energy = 100.0f;
			org->last_reproduce_time = randnf() * org->genome.reproduction_rate;
			world.organisms.push_back(org);

		}
		for (int i = 0; i < max_food; ++i) {
			world.add_food();
		}


	GLFWgamepadstate state;

	float camera_x = 0.0f;
	float camera_y = 0.0f;

	world.sim_delta = 0.02f;

	float food_timer = 0.1f;

	int X_press = 0;
	int Y_press = 0;
	int B_press = 0;
	int T_press = 0;
	int N_press = 0;
	int E_press = 0;
	bool hi_speed = false;
	bool draw = false;
	Organism* selected_organism = nullptr;


	//Partition the food for faster lookup
	while(!glfwWindowShouldClose(window)) {

		if (glfwGetTime() - last_time < (1.0f / float(TARGET_FPS))) continue;
		glfwSetWindowTitle(window, window_name);
		sprintf_s(window_name, "Neural Network Simulation - Organisms: %d, Foods: %d, Partitions: %d, HI_SPEED: %s", world.organisms.size(), world.foods.size(), world.food_partition.size(), hi_speed ? "ON" : "OFF");


		delta = glfwGetTime() - last_time;
		last_time = glfwGetTime(); 
		// Simulation and rendering code would go here
		//printf("There are %d organisms.\n", world.organisms.size());

		glfwGetGamepadState(GLFW_JOYSTICK_1, &state);

		camera_x += glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? 0.5f * (1.0f / zoom) * delta : 0.0f;
		camera_x += glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? -0.5f * (1.0f / zoom) * delta : 0.0f;

		camera_y += glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? 0.5f * (1.0f / zoom) * delta : 0.0f;
		camera_y += glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? -0.5f * (1.0f / zoom) * delta : 0.0f;

		zoom += glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? 0.1f * delta : 0.0f;
		zoom -= glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ? 0.1f * delta : 0.0f;

		if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			++N_press;
			if (N_press == 1) {
				draw = !draw;
			}
		}
		else {
			N_press = 0;
		}
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
			++T_press;
			if (T_press == 1) {
				hi_speed = !hi_speed;
			}
		}
		else {
			T_press = 0;
		}
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			zoom = 0.2f;
		}
		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
			++B_press;
			if (B_press == 1) {
				if (selected_organism) {
					selected_organism->brain.print_inputs();
				}
			}
		}
			else {
				B_press = 0;
			}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			++X_press;
			if (X_press == 1) {
				if (selected_organism == nullptr) {
					selected_organism = world.organisms[0];
					for (Organism* org : world.organisms) {
						if (vec2(org->x - camera_x, org->y - camera_y).mag() < vec2(selected_organism->x - camera_x, selected_organism->y - camera_y).mag()) {
							selected_organism = org;
						}
					}
					selected_organism->possessed = possessing;
				}
				else {
					selected_organism->possessed = false;
					selected_organism = nullptr;
				}
			}
		}
		else {
			X_press = 0;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			++E_press;
			if (E_press == 1) {
				possessing = !possessing;
				if  (selected_organism) {
					selected_organism->possessed = possessing;
				}
			}
		}

		else {
			E_press = 0;
		}
		if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
			//Zoom control
			if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS) {
				zoom += 0.1f * delta;
			}
			if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS) {
				zoom -= 0.1f * delta;
			}
			if (state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS) {
				zoom = 0.1f;
			}
			zoom = clampf(zoom, 0.01f, 1.0f);
			if (state.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS) {
				++Y_press;
				if (Y_press == 1) {
					possessing = !possessing;
					if  (selected_organism) {
						selected_organism->possessed = possessing;
					}
				}
			}
			else {
				Y_press = 0;
			}
			if (state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS) {
				world.sim_delta = 0.1f;
			} else {
				if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS) {
					world.sim_delta = 0.2f;
				}
				else {
					world.sim_delta = 0.05f;
				}
			}
			if (state.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS) {
				++X_press;
				if (X_press == 1) {
					if (selected_organism == nullptr) {
						selected_organism = world.organisms[0];
						for (Organism* org : world.organisms) {
							if (vec2(org->x - camera_x, org->y - camera_y).mag() < vec2(selected_organism->x - camera_x, selected_organism->y - camera_y).mag()) {
								selected_organism = org;
							}
						}
						selected_organism->possessed = possessing;
					}
					else {
						selected_organism->possessed = false;
						selected_organism = nullptr;

					}
				}
			}
			else {
				X_press = 0;
			}
		}

		if (hi_speed) {
			world.sim_delta = 0.2f;
		}
		else {
			world.sim_delta = 0.05f;
		}

		food_timer += world.sim_delta;
		if (world.foods.size() < max_food and food_timer > 0.02) {

			for (int i = 0; i < food_timer / 0.02f; ++i) {
				world.add_food();

			}
			food_timer = 0;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			for (int i = 0; i < 20; ++i) {
				world.add_food();
			}
		}


		if (selected_organism != nullptr) {
			if (selected_organism->alive == false) {
				selected_organism = nullptr;
			} else {
			camera_x = selected_organism->x;
			camera_y = selected_organism->y;


			float move_direction = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
			Organism::possessed_move_speed = move_direction * 0.5;
			Organism::possessed_turn_angle = ((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) - (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)) * 0.2;
			sprintf_s(window_name, "Energy: %.2f, Generation: %d, Partitions: %d", selected_organism->energy, selected_organism->generation, selected_organism->partitions_view);
			}
		}
		else if (state.axes[GLFW_GAMEPAD_AXIS_LEFT_X] != 0.0f || state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] != 0.0f) {
			camera_x += stallf(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X], 0.2f) * 0.5f * (1.0f / zoom) * delta;
			camera_y += -stallf(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y], 0.2f) * 0.5f * (1.0f / zoom) * delta;
		}

		zoom = clampf(zoom, 0.001f, 1.0f);
		//Draw partition grid for debugging


		world.organisms.erase(std::remove_if(world.organisms.begin(), world.organisms.end(), [&](Organism*& o) {
			if (o->alive == false) {

				if (o == selected_organism) selected_organism = nullptr;
				delete o;
				return true;
			}
			return false;
		}), world.organisms.end());

		if (selected_organism) { if (selected_organism->alive == false) { selected_organism = nullptr; } }

		world.foods.erase(std::remove_if(world.foods.begin(), world.foods.end(), [](Food& f) {
			return f.consumed;
		}), world.foods.end());
		
		world.partition();

		new_organisms_0.clear();
		new_organisms_1.clear();
		new_organisms_2.clear();
		new_organisms_3.clear();
		new_organisms_4.clear();
		new_organisms_5.clear();
		new_organisms_6.clear();
		new_organisms_7.clear();
		std::thread  simulation_thread0([&]() {world.multi_step_quarter(0, &new_organisms_0);});
		std::thread  simulation_thread1([&]() {world.multi_step_quarter(1, &new_organisms_1);});
		std::thread  simulation_thread2([&]() {world.multi_step_quarter(2, &new_organisms_2);});
		std::thread  simulation_thread3([&]() {world.multi_step_quarter(3, &new_organisms_3);});
		std::thread  simulation_thread4([&]() {world.multi_step_quarter(4, &new_organisms_4); });
		std::thread  simulation_thread5([&]() {world.multi_step_quarter(5, &new_organisms_5); });
		std::thread  simulation_thread6([&]() {world.multi_step_quarter(6, &new_organisms_6); });
		std::thread  simulation_thread7([&]() {world.multi_step_quarter(7, &new_organisms_7); });
		simulation_thread0.join();
		simulation_thread1.join();
		simulation_thread2.join();
		simulation_thread3.join();
		simulation_thread4.join();
		simulation_thread5.join();
		simulation_thread6.join();
		simulation_thread7.join();
		new_organisms_0.erase(std::remove_if(new_organisms_0.begin(), new_organisms_0.end(), [&](Organism*& o) {
			world.organisms.push_back(o);
			return true;
		}), new_organisms_0.end());

		new_organisms_1.erase(std::remove_if(new_organisms_1.begin(), new_organisms_1.end(), [&](Organism*& o) {
			world.organisms.push_back(o);
			return true;
		}), new_organisms_1.end());
		new_organisms_2.erase(std::remove_if(new_organisms_2.begin(), new_organisms_2.end(), [&](Organism*& o) {
			world.organisms.push_back(o);
			return true;
			}), new_organisms_2.end());
		new_organisms_3.erase(std::remove_if(new_organisms_3.begin(), new_organisms_3.end(), [&](Organism*& o) {
			world.organisms.push_back(o);
			return true;
			}), new_organisms_3.end());
		new_organisms_4.erase(std::remove_if(new_organisms_4.begin(), new_organisms_4.end(), [&](Organism*& o) {
			world.organisms.push_back(o);
			return true;
			}), new_organisms_4.end());
		new_organisms_5.erase(std::remove_if(new_organisms_5.begin(), new_organisms_5.end(), [&](Organism*& o) {
			world.organisms.push_back(o);
			return true;
			}), new_organisms_5.end());
		new_organisms_6.erase(std::remove_if(new_organisms_6.begin(), new_organisms_6.end(), [&](Organism*& o) {
			world.organisms.push_back(o);
			return true;
			}), new_organisms_6.end());
		new_organisms_7.erase(std::remove_if(new_organisms_7.begin(), new_organisms_7.end(), [&](Organism*& o) {
			world.organisms.push_back(o);
			return true;
			}), new_organisms_7.end());
		if (draw) {

			glfwGetWindowSize(window, &window_width, &window_height);
			glViewport(0, 0, window_width, window_height);
			glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			for (auto& cell : world.food_partition) {
				int cx, cy;
				extract_pair(cell.first, cx, cy);
				float cell_x = cx * World::PARTITION_SIZE;
				float cell_y = cy * World::PARTITION_SIZE;

				if (cell_x < 0 and cell_y < 0) {
					//printf("Negative partition detected: %d, %d\n", cx, cy);
				}
				drawRect((cell_x - camera_x) * zoom, (cell_y - camera_y) * zoom, World::PARTITION_SIZE * zoom, World::PARTITION_SIZE * zoom, color3(0.3f, 0.3f, 0.3f));
			}

			for (Organism*& org : world.organisms) {
				//if (abs(org->x - camera_x) > 32.0f / zoom or abs(org->y - camera_y) > 32.0f / zoom) continue;
				//org->brain.print_outputs();
				if (not org) continue;
				if (abs(org->x - camera_x) > 1.0f / zoom or abs(org->y - camera_y) > 1.0f / zoom) continue;
				fillCircle((org->x - camera_x) * zoom, (org->y - camera_y) * zoom, org->genome.size * zoom, 60 * clampf(zoom, 0.1, 0.5), color3(1 - clampf(org->energy / org->energy_capacity, 0, 1), 0.8, 0.6));
				fillCircle((org->x - camera_x + (1.2 * org->genome.size * org->u_speed * cosf(org->angle))) * zoom, (org->y - camera_y + (1.2 * org->genome.size * org->u_speed * sinf(org->angle))) * zoom, 0.5 * org->genome.size * zoom, 30 * clampf(zoom, 0.2, 0.5), color3(0.3, 0.5, 0.5));
			}
			for (Food& food : world.foods) {
				if (abs(food.x - camera_x) > 1.0f / zoom or abs(food.y - camera_y) > 1.0f / zoom) continue;
				fillCircle((food.x - camera_x) * zoom, (food.y - camera_y) * zoom, food.radius * zoom, 60 * clampf(zoom, 0.1, 0.5), color3(0.8f, 0.5f, clampf((food.nutrition - 30.0f) * 0.25, 0.1, 1.0)));
			}

			//Draw vertical organism and food amount bars
			drawRect(0.9f, -0.9f, 0.05f, 1.8f, color3(1.0f, 1.0f, 1.0f));
			fillRect(0.9f, -0.9f, 0.05f, 1.8f * (world.organisms.size() / float(15000)), color3(0.0f, 1.0f, 0.0f));

			drawRect(0.85f, -0.9f, 0.05f, 1.8f, color3(1.0f, 1.0f, 1.0f));
			fillRect(0.85f, -0.9f, 0.05f, 1.8f * (world.foods.size() / float(max_food)), color3(1.0f, 1.0f, 0.0f));
			if (selected_organism != nullptr) {

				color3 highlight_color = color3(1.0f, 1.0f, 0.0f);
				if (possessing) highlight_color = color3(0.0f, 1.0f, 1.0f);
				drawCircle((selected_organism->x - camera_x) * zoom, (selected_organism->y - camera_y) * zoom, selected_organism->genome.size * zoom * 1.2f, 60, highlight_color);

				//Draw vision rays
				for (int i = 0; i < 7; ++i) {
					float ray_angle = (((float(i) / 3.0f) - 1.0f) * selected_organism->genome.field_of_view) + selected_organism->angle;
					vec2 ray_dir(cosf(ray_angle), sinf(ray_angle));
					float ray_length = selected_organism->genome.sight;

					float brightness = selected_organism->distances[i] / selected_organism->genome.sight;
					drawLine((selected_organism->x - camera_x) * zoom, (selected_organism->y - camera_y) * zoom,
						(selected_organism->x + ray_dir.x * ray_length - camera_x) * zoom,
						(selected_organism->y + ray_dir.y * ray_length - camera_y) * zoom,
						color3(brightness, 1.0f, 0.0f));
				}


				//Draw energy bar
				fillRect(-0.9f, 0.8f, 1.8f * clampf(selected_organism->energy / selected_organism->energy_capacity, 0.0f, 1.0f), 0.05f, color3(1.0f - clampf(selected_organism->energy / selected_organism->energy_capacity, 0.0f, 1.0f), clampf(selected_organism->energy / selected_organism->energy_capacity, 0.0f, 1.0f), 0.0f));
				drawRect(-0.9f, 0.8f, 1.8f, 0.05f, color3(1.0f, 1.0f, 1.0f));

				//Draw reproduction cooldown bar
				fillRect(-0.9f, 0.72f, 1.8f * clampf((selected_organism->last_reproduce_time) / 20.0f, 0.0f, 1.0f), 0.03f, color3(0.5f, 0.8f, 1.0f));
				drawRect(-0.9f, 0.72f, 1.8f, 0.03f, color3(1.0f, 1.0f, 1.0f));

				//Draw life span bar
				fillRect(-0.9f, 0.76f, 1.8f * (1 - clampf(selected_organism->life_time / selected_organism->genome.life_span, 0.0f, 1.0f)), 0.03f, color3(0.5f, 0.8f, 0.5f));
				drawRect(-0.9f, 0.76f, 1.8f, 0.03f, color3(1.0f, 1.0f, 1.0f));
				//Draw Neural Network synapses
				for (auto& syn : selected_organism->brain.synapses) {
					_2_int_32 np = syn.first;
					unsigned int from_index, to_index;
					extract_upair(np, from_index, to_index);
					neuron& from_neu = selected_organism->brain.neurons[from_index];
					neuron& to_neu = selected_organism->brain.neurons[to_index];
					float from_x_t = from_neu.layer_index / float(MAX_LAYERS);
					float from_y_t = from_neu.layer_position / 10.0f;
					if (from_neu.layer_index == -1) from_x_t = 1.0f;
					float to_x_t = to_neu.layer_index / float(MAX_LAYERS);
					float to_y_t = to_neu.layer_position / 10.0f;
					if (to_neu.layer_index == -1) to_x_t = 1.0f;
					color3 syn_color = color3(0.5f, 0.5f, 0.5f);
					if (syn.second > 0) {
						syn_color = color3(1.0f, 0.5f, 0.5f).mix(color3(1.0f, 1.0f, 1.0f), clampf(syn.second, 0, 1));
					}
					else {
						syn_color = color3(0.5f, 0.5f, 1.0f).mix(color3(1.0f, 1.0f, 1.0f), clampf(-syn.second, 0, 1));
					}
					drawLine((from_x_t * 0.5f) - 0.5f, from_y_t * 0.5f - 0.5f,
						(to_x_t * 0.5f) - 0.5f, to_y_t * 0.5f - 0.5f,
						syn_color);
				}

				//Draw Neural Network structure
				for (neuron& neu : selected_organism->brain.neurons) {
					float x_t = neu.layer_index / float(MAX_LAYERS);
					float y_t = neu.layer_position / 10.0f;

					color3 neuron_color = color3(0.2f, 0.2f, 0.2f);
					if (neu.layer_index == -1) x_t = 1.0f;

					if (neu.raw_activate() > 0) {
						neuron_color = color3(1.0f, 0.5f, 0.5f);
					}
					else {
						neuron_color = color3(0.5f, 0.5f, 1.0f);
					}
					fillCircle((x_t * 0.5f) - 0.5f, y_t * 0.5f - 0.5f, 0.02f, 30, color3(0.0f, 0.0f, 0.0f));
					fillCircle((x_t * 0.5f) - 0.5f, y_t * 0.5f - 0.5f, 0.016f * clampf(abs(neu.raw_activate()), 0, 2), 30, neuron_color);
				}

				//Draw visible partitions
				for (auto& p : selected_organism->visible_food_partitions) {
					int pcx, pcy;
					extract_pair(p, pcx, pcy);
					float part_x = pcx * World::PARTITION_SIZE;
					float part_y = pcy * World::PARTITION_SIZE;
					drawRect((part_x - camera_x) * zoom, (part_y - camera_y) * zoom, World::PARTITION_SIZE * zoom, World::PARTITION_SIZE * zoom, color3(1.0f, 1.0f, 0.0f));
				}
			}

			glfwSwapBuffers(window);
		}
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}