#pragma once
#include <cmath>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <algorithm>
class neuron;
class synapse;
class neural_network;

//TODO: replace pair with longlong int for performance & hashability;
using _2_int_32 = uint64_t;
typedef float* neu_out_ptr;
typedef neuron** neu_ptr_ptr;
#define MAX_BIAS_ALTER (float)0.7
#define MAX_WEIGHT_ALTER (float)0.7
#define BIAS_ALTER_CHANCE (float)0.5
#define WEIGHT_ALTER_CHANCE (float)0.5
#define ADD_NEURON_CHANCE (float)0.3
#define ADD_SYNAPSE_CHANCE (float)0.3
#define MAX_LAYERS (int)16
//The maximum magnitudes biases and weights can be.
#define BIAS_MAX_MAG (float)4
#define WEIGHT_MAX_MAG (float)4
#define PRUNE_THRESHOLD (float)0.125

class neuron
{ 

public:

	float stimulus = 0;
	float bias = 0;

	/*
	0 - input layer
	-1 - output layer
	1..n - hidden layers
	*/
	int layer_index = 0;
	int layer_position = 0;

	neuron (float bias = 0.0f, int layer_index = 0, int layer_position = 0) : stimulus(0.0), bias(bias), layer_index(layer_index), layer_position(layer_position) {}

	inline float activate() const
	{
		// Simple activation function: sigmoid
		return 1.0f / (1.0f + expf(-(stimulus + bias)));
	}
	inline float nactivate() const
	{
		// Hyperbolic tangent activation function
		return activate() * 2.0f - 1.0f;
	}
	inline float raw_activate() const
	{
		if (layer_index == 0) {
			//Input neurons return their stimulus directly
			return stimulus;
		}
		return stimulus + bias;
	}
};
/*
A class representing a neural network.
*/
class neural_network {
	public:
	
		//A score representing the fitness of the neural network. Higher scores move on to the next generation.
	float score = 0;
	bool sorted_neurons = false;
	bool sorted_synapses = false;
	std::vector<neuron> neurons;
	std::unordered_map<_2_int_32, float> synapses;
	std::vector<int> input_neurons;
	std::vector<int> output_neurons;

	// Sort the synapse keys for ordered processing
	std::vector<_2_int_32> syn_sorted;
	void add_synapse(float weight, _2_int_32 np);
	int add_neuron(neuron n);

	neural_network() {}

	void propagate_all();
	/*
	Returns a pointer to an array of neuron pointers representing the input neurons.
	*/
	void return_input(neu_ptr_ptr data, int length);
	/*
	Returns a pointer to an array of floats representing the Sigmoid activated output values.
	*/
	void return_output(neu_out_ptr* data, int length);
	/*
	Returns a pointer to an array of floats representing the raw output values.
	*/
	void return_raw_output(neu_out_ptr* data, int length);
	/*
	Alters the bias of a neuron by a given delta.
	*/
	void alter_bias(int neuron_index, float delta);
	/*
	Alters the weight of a synapse by a given delta.
	*/
	int add_input_neuron();
	int add_output_neuron();
	void alter_weight(_2_int_32 np, float delta);

	void alter_random(int = 1);

	void add_random_synapse(int = -1);
	void add_random_synapse_to(int neuron_index);

	int add_random_neuron();


	void clear();

	void print_structure() const;

	void print_outputs();

	void print_inputs();

	inline neuron* input(int in_index) {
		return &neurons[input_neurons[in_index]];
	}
	inline neuron* output(int in_index) {
		return &neurons[output_neurons[in_index]];
	}
private:
	void sort_neurons();
	void sort_synapses();
	static int get_alter_choice();
	bool synapse_predicate(_2_int_32& p1, _2_int_32& p2);
};
//Returns a random float between 0.0 and 1.0
float randf();
//Returns a random float between -1.0 and 1.0
float randnf();

float clampf(float value, float min, float max);
float max(float value, float max);
float min(float value, float min);
float stallf(float value, float stall);
inline _2_int_32 up(unsigned int from, unsigned int to) {
	// Mask to 32 bits first to avoid accidental propagation and ensure clean packing
    uint64_t low  = static_cast<uint64_t>(static_cast<uint32_t>(from));
    uint64_t high = static_cast<uint64_t>(static_cast<uint32_t>(to));
    return static_cast<_2_int_32>( (low) | (high << 32) );
};

inline _2_int_32 ip(int from, int to) {
    // Preserve raw 32-bit two's-complement bit patterns, then pack into 64-bit
    uint64_t low  = static_cast<uint64_t>(static_cast<uint32_t>(from));
    uint64_t high = static_cast<uint64_t>(static_cast<uint32_t>(to));
    return static_cast<_2_int_32>( (low) | (high << 32) );
}

inline void extract_upair(const _2_int_32 p, unsigned int& from, unsigned int& to) {
    from = static_cast<unsigned int>( static_cast<uint32_t>( p & 0xFFFFFFFFULL ) );
    to   = static_cast<unsigned int>( static_cast<uint32_t>( (p >> 32) & 0xFFFFFFFFULL ) );
}

inline void extract_pair(const _2_int_32 p, int& from, int& to) {
    // Extract as uint32_t then reinterpret as int32_t via cast to preserve bit pattern
    from = static_cast<int>( static_cast<uint32_t>( p & 0xFFFFFFFFULL ) );
    to   = static_cast<int>( static_cast<uint32_t>( (p >> 32) & 0xFFFFFFFFULL ) );
}

bool neural_network_compare(const neural_network& a, const neural_network& b);