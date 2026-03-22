#include "neuron.h"


bool neural_network::synapse_predicate(_2_int_32& p1, _2_int_32& p2) {
	unsigned int f1, t1, f2, t2;
	extract_upair(p1, f1, t1);
	extract_upair(p2, f2, t2);
	//Compare based on from-layer index first, then to-layer index
	return neurons[f1].layer_index < neurons[f2].layer_index;
}

void neural_network::alter_random(int ALTER) {
	for (int i = 0; i < ALTER; ++i) {
		//printf("Applying random alteration %d/%d\n", i + 1, ALTER);
		int choice = get_alter_choice();
		if (choice == 1) {
			//Alter a random synapse weight
			if (synapses.size() == 0) continue;
			auto it = synapses.begin();
			std::advance(it, rand() % synapses.size());
			//printf("Altering weight of synapse from neuron %llu to neuron %llu\n", (it->first & 0xFFFFFFFFULL), (it->first >> 32) & 0xFFFFFFFFULL);
			alter_weight(it->second, MAX_WEIGHT_ALTER * randnf());
		}
		else if (choice == 0) {
			//Alter a random neuron bias
			if (neurons.size() == 0) continue;
			int neu_index = rand() % neurons.size();
			alter_bias(neu_index, MAX_BIAS_ALTER * randnf());
		}
		else if (choice == 2) {
			//Add a random synapse
			add_random_synapse();
		}
		else {
			int neu = add_random_neuron();
			add_random_synapse_to(neu);
		}
	}
}

float clampf(float value, float min, float max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;

}
float max(float value, float max) {
	if (value > max) return value;
	return max;
}
float stallf(float value, float stall) {
	if (value > -stall && value < stall) return 0.0f;
	return value;
}
void neural_network::add_random_synapse(int neu) {
	//Pick a random neuron

	float weight = randnf();
	unsigned int from_index = 0;
	if (neu != -1) {
		from_index = neu;
	}
	else {
		from_index = rand() % neurons.size();
	}
	unsigned int to_index;
	for (int attempts = 0; attempts < 10; ++attempts) {
		unsigned int to_index = rand() % neurons.size();
		if (neurons[to_index].layer_index == neurons[from_index].layer_index) continue;
		//synapse s(weight, &neurons[from_index], &neurons[to_index]);
		if (synapses.find(up(from_index, to_index)) == synapses.end()) {
			add_synapse(weight, up(from_index, to_index));
		}
		else {
			//Add the synapses weight to the existing synapse
			alter_weight(up(from_index, to_index), weight);
		}
		return;
	}

	//If we reach here, we failed to add a synapse after several attempts and got really unlucky :(
	//Pick all possible target neurons
	std::vector<unsigned int> possible_targets;
	for (unsigned int i = 0; i < neurons.size(); i++) {
		if (neurons[i].layer_index != neurons[from_index].layer_index && i != from_index) {
			possible_targets.push_back(i);
		}
	}

	if (possible_targets.size() == 0) {
		return; //If there are no possible targets, exit
	}

	to_index = possible_targets[rand() % possible_targets.size()];
	if (synapses.find(up(from_index, to_index)) == synapses.end()) {
		add_synapse(weight, up(from_index, to_index));
	}
	else {
		//Add the synapses weight to the existing synapse
		alter_weight(up(from_index, to_index), weight);
	} 

}
int neural_network::add_input_neuron() {
	neuron n(0.0f, 0, input_neurons.size());
	int ind = add_neuron(n);
	sorted_neurons = false;


	return ind;
}
int neural_network::add_output_neuron() {
	neuron n(0.0f, -1, output_neurons.size());
	int ind = add_neuron(n);
	sorted_neurons = false;


	return ind;
}
void neural_network::add_random_synapse_to(int neuron_index) {
	//Find all possible source neurons
	
	//Do 10 random attempts first
	for (unsigned int i = 0; i < 10; i++) {
		unsigned int random_index = rand() % neurons.size();
		if (neurons[random_index].layer_index != neurons[neuron_index].layer_index && random_index != neuron_index && neurons[neuron_index].layer_index != -1) {
			add_synapse(randnf(), up(random_index, neuron_index));
			return;
		}
	}

	std::vector<unsigned int> possible_sources;
	for (unsigned int i = 0; i < neurons.size(); i++) {
		if (neurons[i].layer_index != neurons[neuron_index].layer_index && i != neuron_index && neurons[neuron_index].layer_index != -1) {
			possible_sources.push_back(i);
		}
	}
	if (possible_sources.size() == 0) {
		return; //If there are no possible sources, exit
	}
	unsigned int from_index = possible_sources[rand() % possible_sources.size()];
	add_synapse(randnf(), up(from_index, neuron_index));
}
int neural_network::add_random_neuron() {
	int ind = add_neuron(neuron(0.0f, (rand() % MAX_LAYERS - 1) + 1, rand() % MAX_LAYERS));
	add_random_synapse(ind);
	//printf("Added random neuron %d at layer %d position %d\n", ind, neurons[ind].layer_index, neurons[ind].layer_position);
	return ind;
}

void neural_network::sort_neurons() {

	if (sorted_neurons) return;
	// sort the in/out neurons by their layer positions
	std::sort(input_neurons.begin(), input_neurons.end(), [&](int a, int b) {
		return neurons[a].layer_position < neurons[b].layer_position;
	});
	std::sort(output_neurons.begin(), output_neurons.end(), [&](int a, int b) {
		return neurons[a].layer_position < neurons[b].layer_position;
	});
	sorted_neurons = true;
}
void neural_network::sort_synapses() {

	if (sorted_synapses) return;
	syn_sorted.clear();
	//printf("Sorting synapses...\n");
	syn_sorted.reserve(synapses.size()); // Pre-allocate to avoid re-allocations
	int i = 0;

	// The correct, safe loop structure:
	for (auto it = synapses.begin(); it != synapses.end(); ) {

		if (std::abs(it->second) < PRUNE_THRESHOLD) {
			// Erase the element safely:
			// The erase() member function returns the iterator 
			// that points to the *next* valid element in the container.
			it = synapses.erase(it);
		}
		else {
			// If the item is kept, add it to your sorted list
				syn_sorted.push_back(it->first);

			// Only increment the counter and the iterator here
			++i;
			++it;
		}
	}

	//Sort synapses by from-layer index
	std::sort(syn_sorted.begin(), syn_sorted.end(), [&](const _2_int_32& a, const _2_int_32& b) {
		unsigned int from_a, from_b, to_a, to_b;
		extract_upair(a, from_a, to_a);
		extract_upair(b, from_b, to_b);
		return neurons[from_a].layer_index < neurons[from_b].layer_index;
	});
	sorted_synapses = true;
}
void neural_network::clear() {
	neurons.clear();
	synapses.clear();
	input_neurons.clear();
	output_neurons.clear();
	sorted_neurons = false;
	sorted_synapses = false;
}

void neural_network::print_structure() const {
	printf("Neural Network Structure:\n");
	printf("Neurons:\n");
	for (int i = 0; i < neurons.size(); i++) {
		const neuron& n = neurons[i];
		printf("  Neuron %d: Layer %d, Position %d, Bias %.3f\n", i, n.layer_index, n.layer_position, n.bias);
	}
	printf("Synapses:\n");
	for (const auto& pair : synapses) {
		const _2_int_32& np = pair.first;
		unsigned int from, to;
		extract_upair(np, from, to);
		printf("  Synapse from Neuron %d to Neuron %d: Weight %.3f\n", from, to, pair.second);
	}
}

void neural_network::alter_weight(_2_int_32 np, float delta) {
	if (synapses.find(np) != synapses.end()) {
		synapses[np] += delta;

		if (synapses[np] > 1.0f) {
			synapses[np] = 1.0f;
		}
		else if (synapses[np] < -1.0f) {
			synapses[np] = -1.0f;
		}
	}
}
int neural_network::get_alter_choice() {
	float r = randf() * (BIAS_ALTER_CHANCE + WEIGHT_ALTER_CHANCE + ADD_NEURON_CHANCE + ADD_SYNAPSE_CHANCE);
	if (r < BIAS_ALTER_CHANCE) return 0;
	else if (r < BIAS_ALTER_CHANCE + WEIGHT_ALTER_CHANCE) return 1;
	else if (r < BIAS_ALTER_CHANCE + WEIGHT_ALTER_CHANCE + ADD_NEURON_CHANCE) return 2;
	else return 3;
}

float randf() {
	return static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
}
//Returns a random float between -1.0 and 1.0
float randnf() {
	return randf() * 2.0f - 1.0f;
}

void neural_network::print_outputs(){
	printf("Neural Network Outputs:\n");
	for (int i = 0; i < output_neurons.size(); i++) {
		const neuron* n = output(i);
		printf("  Output Neuron %d: Activated Value %.3f, Raw Value %.3f\n", i, n->activate(), n->raw_activate());
	}
}
void neural_network::print_inputs() {
	printf("Neural Network Inputs:\n");
	for (int i = 0; i < input_neurons.size(); i++) {
		const neuron* n = input(i);
		printf("  Input Neuron %d: Stimulus: %.3f\n", i, n->stimulus);
	}
}
void neural_network::alter_bias(int neuron_index, float delta) {
	if (neurons[neuron_index].layer_index == 0) return; //Cannot alter bias of input neurons
	if (neuron_index >= 0 && neuron_index < neurons.size()) {
		neurons[neuron_index].bias += delta;

		if (neurons[neuron_index].bias > BIAS_MAX_MAG) {
			neurons[neuron_index].bias = BIAS_MAX_MAG;
		}
		else if (neurons[neuron_index].bias < -BIAS_MAX_MAG) {
			neurons[neuron_index].bias = -BIAS_MAX_MAG;
		}
	}
}

void neural_network::return_raw_output(neu_out_ptr* data, int length) {
	*data = new float[length];
	for (int i = 0; i < length && i < output_neurons.size(); i++) {
		(*data)[i] = output(i)->raw_activate();
	}
}

void neural_network::return_output(neu_out_ptr* data, int length) {
	*data = new float[length];
	for (int i = 0; i < length && i < output_neurons.size(); i++) {
		(*data)[i] = output(i)->activate();
	}
}

void neural_network::return_input(neu_ptr_ptr data, int length) {
	data = new neuron * [length];
	for (int i = 0; i < length && i < input_neurons.size(); i++) {
		data[i] = (neuron*)input(i);
	}
}

void neural_network::propagate_all() {
	if (!sorted_neurons) {
		sort_neurons();
	}
	if (!sorted_synapses) {
		sort_synapses();
	}
	// Reset stimuli
	for (auto& neuron : neurons) {
		if (neuron.layer_index == 0) {
			continue;
		} // Skip input neurons
		neuron.stimulus = 0.0f;
	}
	// Propagate through all synapses
	for (const _2_int_32& s : syn_sorted) {
		unsigned int from, to;
		extract_upair(s, from, to);
		neurons[to].stimulus += neurons[from].raw_activate() * synapses[s];
		//printf("Propagating synapse from layer %d to layer %d with weight %.3f\n", neurons[from].layer_index, neurons[to].layer_index, synapses[s]);
	}
}



int neural_network::add_neuron(neuron n) {
	neurons.push_back(n);
	int n_ptr = neurons.size() - 1;
	if (n.layer_index == 0) {
		input_neurons.push_back(n_ptr);
	}
	else if (n.layer_index == -1) {
		output_neurons.push_back(n_ptr);
	}

	sorted_neurons = false;

	return neurons.size() - 1;
}

void neural_network::add_synapse(float weight, _2_int_32 np) {
	synapses.insert({ np, weight });

	sorted_synapses = false;
}

bool neural_network_compare(const neural_network& a, const neural_network& b) {
	return a.score < b.score;
}

float min(float value, float min) {
	if (value < min) return value;
	return min;
}