#include "World.h"

#define PI 3.14159265358979323846
World::World() {}

float Organism::possessed_move_speed = 1.0f;
float Organism::possessed_turn_angle = 0.0f;

const float World::PARTITION_SIZE = 10.0f;
void Organism::move_neural(float dt, World* world) {
	//Example: set speed based on first output neuron
	if (alive == false) return;
	brain.propagate_all();
	energy_capacity = 3000.0f * genome.size;
	if (brain.output_neurons.size() > 0) {
		//printf("Outputs: %.3f\n", vector_out(0, 1));
		float u_turn = brain.output(1)->nactivate();
		if (possessed) {
			//If possessed, use external controls
			u_speed = possessed_move_speed;
			u_turn = possessed_turn_angle;
		}
		else
		u_speed = brain.output(0)->nactivate(); //Get speed from output neurons
		speed = u_speed * genome.move_speed * dt * (0.1f / genome.size); //Scale speed
		angle += u_turn * 2.0f * PI * genome.turn_speed * dt; //Scale angle
		energy -= (fabs(u_speed * u_speed) + 0.5) * genome.energy_consumption_rate * dt;

		last_reproduce_time += dt;
		life_time += dt;
		//brain.print_outputs();
		//02
		if (life_time > genome.life_span) alive = false;
		if (energy < 5) alive = false;


		move(dt, speed);

		std::vector<int> close_foods;
		for (const _2_int_32& partition_key : visible_food_partitions) {
			for (unsigned int food_index : world->food_partition[partition_key]) {
				Food& food = world->foods[food_index];
				if (vec2(food.x - x, food.y - y).mag() < genome.size + food.radius) {
					energy += food.nutrition;
					food.consumed = true;
				}
			}
		}

		if (energy >energy_capacity) {
			energy = energy_capacity; //Cap energy to prevent overflow
		}
	}
}

Organism* Organism::reproduce_asex() {

	if (last_reproduce_time < genome.reproduction_rate or (energy < genome.reproduction_cost + 50) or (alive == false)) {
		return nullptr; //Cannot reproduce yet
	}
	energy -= genome.reproduction_cost;
	//printf("Organism at (%.2f, %.2f) is reproducing asexually.\n", x, y);
	//Asexual reproduction: create a copy of the organism with slight mutations
	Organism* offspring = new Organism(*this);
	offspring->last_reproduce_time = 0.0f;
	offspring->life_time = 0.0f;
	offspring->alive = true;

	offspring->x += randnf() * 0.2f; //Offset position slightly
	offspring->y += randnf() * 0.2f; //Offset position slightly
	offspring->angle = randf() * 2.0f * PI; //Randomize angle
	offspring->generation++;

	offspring->energy = min(genome.reproduction_cost, energy); //Split energy with offspring

	offspring->genome.mutate_asex();
	offspring->brain.alter_random(8); //Apply random mutations to the brain
	//Add offspring to the world (this would require access to the World instance, omitted for brevity)

	return offspring;
}

void Organism::stimulate_senses(World* world) {
	
	world->org_get_partitions(this);
	std::vector<unsigned int> close_foods;
	std::vector<unsigned int> close_organisms;

	//Recollect the actual individual organisms/foods from the partitions;
	for (const _2_int_32& partition_key : visible_food_partitions) {
		for (unsigned int food_index : world->food_partition[partition_key]) {
			close_foods.push_back(food_index);
		}

	}
	for (const _2_int_32& partition_key : visible_org_partitions) {
		for (unsigned int org_index : world->org_partition[partition_key]) {
			close_organisms.push_back(org_index);
		}
	}
	partitions_view = visible_food_partitions.size();
	foods_view = close_foods.size();

	//There are 7 zones
	//For each zone, find the CLOSEST object and it's type;

	const int ZONES = 7;
	bool types[ZONES] = {false};
	vec2 angle_vectors[ZONES] = { vec2() };
	vec2 org_dir(cosf(angle), sinf(angle));
	const float zone_range = cosf((1.5 * genome.field_of_view) / float(ZONES) );
	//Pre-load the angle vectors (access > math)
	const float half = (float(ZONES) - 1.0f) * 0.5f;
	for (int i = 0; i < ZONES; ++i) {
		float i_t = (float(i) / half) - 1.0f;
		float zone_angle = angle + (i_t * genome.field_of_view);
		distances[i] = genome.sight;
		types[i] = false;
		angle_vectors[i] = vec2(cosf(zone_angle), sinf(zone_angle));
	}

	bool type = false;
	//Loop through each of the objects (food/organism)

	for (unsigned int& f : close_foods) {
		if (f < 0 || f >= world->foods.size()) continue;
		Food& food = world->foods[f];
		if (food.consumed) continue;
		vec2 to_vec(food.x - x, food.y - y);
		if (org_dir.dot(to_vec) <= 0.0f) continue;
		if (to_vec.mag() >= genome.sight) continue;
		for (int i = 0; i < ZONES; ++i) {
			if (angle_vectors[i].dot(to_vec.normalize()) >= zone_range) {
				if (to_vec.mag() <= distances[i]) {
					distances[i] = to_vec.mag();
					types[i] = false;
					break;
				}
			}
		}
	}
	for (unsigned int& o : close_organisms) {
		if (o < 0 || o >= world->organisms.size()) continue;
		Organism*& org = world->organisms[o];

		if (org == this) continue;
		if (!org->alive) continue;
		vec2 to_vec(org->x - x, org->y - y);
		if (org_dir.dot(to_vec) <= 0.0f) continue;
		if (to_vec.mag() >= genome.sight) continue;

		int zone = 0;
		for (int i = 0; i < ZONES; ++i) {
			if (angle_vectors[i].dot(to_vec.normalize()) >= zone_range) {
				if (to_vec.mag() <= distances[i]) {
					distances[i] = to_vec.mag();
					types[i] = true;
					break;
				}
			}
		}
	}

	//Now we actually stimulate the neurons
	for (int in : brain.input_neurons) {
		brain.neurons[in].stimulus = 0.0f;
	}
	for (int i = 0; i < ZONES; ++i) {
		int zone_id = i * 2;
		if (types[i] == true) ++zone_id;
		neuron* in = brain.input(zone_id);
		float st = (1.0f - (distances[i] / genome.sight));
		in->stimulus = st * st;
	}

	brain.input(14)->stimulus = (energy / energy_capacity)* (energy / energy_capacity);
	brain.input(15)->stimulus = (1.0 - (energy / energy_capacity))* (1.0 - (energy / energy_capacity));

	brain.input(16)->stimulus = brain.output(0)->stimulus;
	brain.input(17)->stimulus = brain.output(1)->stimulus;
	brain.input(18)->stimulus = brain.output(2)->stimulus;
	
	//Now add the interoceptive inputs

	//Low energy (if energy < 10000 * rate, input is higher)
	//High energy (if energy > 10000 * rate, input is higher)
}

void World::multi_step_quarter(int off, std::vector<Organism*>* org_container) {
	//Process every 8th organism starting from offset 'off'
	if (not org_container) return;
	for (int i = off; i < organisms.size(); i += 8) {

		if (organisms[i]->alive == false) continue;
		if (i < 0 || i >= organisms.size()) continue;
		Organism* org = organisms[i];
		org->stimulate_senses(this);
		org->move_neural(sim_delta, this);

		 if (organisms[i]->brain.output(2)->activate() > 0.61f) {
			//Attempt to reproduce
			 int organism_count = 0;
			 for (int j = 0; j < organisms[i]->offspring_count; ++j) {
				 Organism* off = organisms[i]->reproduce_asex();
				 if (off != nullptr) { org_container->push_back(off); organism_count++; }
			 };

			 if (organism_count > 0) {
				 //Only reset if at least one offspring was produced
				 organisms[i]->last_reproduce_time = randnf();
			 }
		}
	}
}

void World::partition() {
	food_partition.clear();
	for (unsigned int i = 0; i < foods.size(); ++i) {
		Food& food = foods[i];
		int cell_x = static_cast<int>(floorf(food.x / PARTITION_SIZE));
		int cell_y = static_cast<int>(floorf(food.y / PARTITION_SIZE));

		if (cell_x < 0) {
			//printf("Warning: Food item %d has negative cell_x partition (%d, %d).\n", i, cell_x, cell_y);
		}
		food_partition[ip(cell_x, cell_y)].push_back(i);
	}

	org_partition.clear();
	for (unsigned int i = 0; i < organisms.size(); ++i) {
		Organism* org = organisms[i];
		int cell_x = static_cast<int>(floorf(org->x / PARTITION_SIZE));
		int cell_y = static_cast<int>(floorf(org->y / PARTITION_SIZE));
		org_partition[ip(cell_x, cell_y)].push_back(i);
	}
}

void World::org_get_partitions(Organism* org) {
	org->visible_org_partitions.clear();
	org->visible_food_partitions.clear();
	for (auto& p : food_partition) {
		int cx = 0;
		int cy = 0;
		extract_pair(p.first, cx, cy);
		
		float distance = org->genome.sight + (0.707 * PARTITION_SIZE);

		vec2 to(float((cx + 0.5) * PARTITION_SIZE) - org->x, float((cy + 0.5) * PARTITION_SIZE) - org->y);
		if (cx < 0) {
			//printf("Warning: Organism at (%.2f, %.2f) checking negative cell_x partition (%d, %d).\n", org->x, org->y, cx, cy);
		}
		if (to.mag() <= distance) {
			org->visible_food_partitions.push_back(p.first);
		}
	}
	for (auto& p : org_partition) {
		int cx, cy;
		extract_pair(p.first, cx, cy);

		float distance = org->genome.sight + PARTITION_SIZE;

		vec2 to(float(cx) - org->x, float(cy) - org->y);
		if (to.mag() <= distance) {
			org->visible_org_partitions.push_back(p.first);
		}
	}
}