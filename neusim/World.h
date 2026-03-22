#pragma once

#include "neuron.h"
#include <cassert>
using food_partition_map = std::unordered_map<_2_int_32, std::vector<unsigned int>>;
using org_partition_map = std::unordered_map<_2_int_32, std::vector<unsigned int>>;

class World;

struct vec2 {
	float x;
	float y;
	vec2(float x = 0, float y = 0) : x(x), y(y) {}
	vec2 normalize() {
		float length = sqrtf(x * x + y * y);
		if (length == 0) return vec2(0, 0);
		return vec2(x / length, y / length);
	}

	vec2 euler(float rad) {
		return vec2(cosf(rad), sinf(rad)); 
	}

	float mag() {
		return sqrtf((x * x) + (y * y));
	}

	float dot(const vec2& other) {
		return (x * other.x) + (y * other.y);
	}
};


class OrganismGenome {
	public:
		float reproduction_rate = 3.0f;
		float move_speed = 25.0f;
		float turn_speed = 0.5f;
		float size = 0.2f;
		float energy_consumption_rate = 1.0f;
		float sight = 20.0f;
		float field_of_view = 1.0f;
		float life_span = 500.0f;
		float reproduction_cost = 50.0f;

		OrganismGenome() {}
		OrganismGenome(float reproduction_rate, float move_speed, float turn_speed, float size, float energy_consumption_rate, float sight, float field_of_view, float life_span, float reproduction_cost) :
			reproduction_rate(reproduction_rate), move_speed(move_speed), turn_speed(turn_speed), size(size), energy_consumption_rate(energy_consumption_rate), sight(sight), field_of_view(field_of_view), life_span(life_span), reproduction_cost(reproduction_cost) {
		}
		void mutate_asex() {
			reproduction_rate += randnf() * 0.1f;
			move_speed += randnf() * 1.0f;
			turn_speed += randnf() * 0.05f;
			size += randnf() * 0.01f;
			energy_consumption_rate += randnf() * 0.05f;
			sight += randnf() * 0.2f;
			field_of_view += randnf() * 0.05f;
			life_span += randnf() * 5.0f;
			reproduction_cost += randnf() * 1.0f;

			reproduction_rate = clampf(reproduction_rate, 0.1f, 10.0f);
			move_speed = clampf(move_speed, 1.0f, 100.0f);
			turn_speed = clampf(turn_speed, 0.1f, 5.0f);
			size = clampf(size, 0.05f, 1.0f);
			energy_consumption_rate = clampf(energy_consumption_rate, 0.1f, 5.0f);
			sight = clampf(sight, 1.0f, 100.0f);
			field_of_view = clampf(field_of_view, 0.1f, 3.14159f);
			life_span = clampf(life_span, 50.0f, 1000.0f);
			reproduction_cost = clampf(reproduction_cost, 10.0f, 200.0f);

		}

		void mutate_sex(OrganismGenome* g0, OrganismGenome* g1) {
			//Crossover mutation
			reproduction_rate = (randf() < 0.5f) ? g0->reproduction_rate : g1->reproduction_rate;
			move_speed = (randf() < 0.5f) ? g0->move_speed : g1->move_speed;
			turn_speed = (randf() < 0.5f) ? g0->turn_speed : g1->turn_speed;
			size = (randf() < 0.5f) ? g0->size : g1->size;
			energy_consumption_rate = (randf() < 0.5f) ? g0->energy_consumption_rate : g1->energy_consumption_rate;
			sight = (randf() < 0.5f) ? g0->sight : g1->sight;
			field_of_view = (randf() < 0.5f) ? g0->field_of_view : g1->field_of_view;
			life_span = (randf() < 0.5f) ? g0->life_span : g1->life_span;
			reproduction_cost = (randf() < 0.5f) ? g0->reproduction_cost : g1->reproduction_cost;
			//Apply slight random mutations
			mutate_asex();
		}


};
class Food {
public:
	bool consumed = false;
	float x;
	float y;
	float radius;
	float nutrition;

	Food(float x = 0, float y = 0, float radius = 1.0f, float nutrition = 10.0f) : x(x), y(y), radius(radius), nutrition(nutrition) {}
};
class Organism {
public:


	std::vector<_2_int_32> visible_food_partitions;
	std::vector<_2_int_32> visible_org_partitions;
	int partitions_view = 0;
	int foods_view = 0;
	int offspring_count = 3;
	int generation = 0;
	bool possessed = false;
	float x = 0.0f;
	float y = 0.0f;
	float angle = 0.0f;
	float speed = 0.0f;
	float u_speed = 0.0f;
	float energy_capacity;

	float distances[7] = { 0.0f };
	//The organism will learn to hunt while conserving energy.

	float life_time = 0.0f;
	float last_reproduce_time = 0.0f;
	//How much energy it costs to reproduce and how much energy is given to the offspring.
	float energy = 100.0f;
	float stamina = 50.0f;
	OrganismGenome genome;

	bool alive = true;

	static float possessed_turn_angle;
	static float possessed_move_speed;
	//A neural network brain to control the organism.

	neural_network brain;

	Organism(float x = 0, float y = 0, neural_network brain = neural_network()) : x(x), y(y), brain(brain) {
		last_reproduce_time = randnf();
	}
	inline void move(float delta_time, float s) {
		float dx = cosf(angle) * s * delta_time;
		float dy = sinf(angle) * s * delta_time;
		x += dx;
		y += dy;
	}

	void move_neural(float dt, World* world);

	inline float vector_out(int n0, int n1) {
		if (brain.output_neurons.size() < 2) return 0.0f;
		float out0 = brain.output(n0)->activate();
		float out1 = brain.output(n1)->activate();
		//printf("Vector output neurons %d and %d: %.3f, %.3f\n", n0, n1, out0, out1);
		return (out1 - out0); //Return difference for direction (1 = full positive, -1 = full negative)
	}

	inline float n_vector_out(int n0, int n1) {
		if (brain.output_neurons.size() < 2) return 0.0f;
		float out0 = brain.output(n0)->nactivate();
		float out1 = brain.output(n1)->nactivate();
		//printf("Vector output neurons %d and %d: %.3f, %.3f\n", n0, n1, out0, out1);
		return (out1 - out0); //Return difference for direction (1 = full positive, -1 = full negative)
	}
	Organism* reproduce_asex();

	void stimulate_senses(World*);
};

class World {
	public:
	float sim_delta;
	static const float PARTITION_SIZE;
	std::vector<Organism*> organisms;
	std::vector<Organism*> new_organisms;
	std::vector<Food> foods;
	food_partition_map food_partition;
	org_partition_map org_partition;
	World();

	void multi_step_quarter(int off = 0, std::vector<Organism*>* org_container = nullptr);

	inline void add_food() {
		foods.push_back(Food(randnf() * 300.0f, randnf() * 300.0f, 0.1f, 50.0f));
	}

	void partition();

	void org_get_partitions(Organism* org);
};