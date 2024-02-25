#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

typedef enum {AND, OR, NOT} GATE_TYPE;

typedef struct _node {
	bool input;
	GATE_TYPE gate;
	int *inputs, inputs_number, node_number;
} node;

typedef struct _sat_node {
	bool input;
	GATE_TYPE gate;
	struct _sat_node **inputs;
	int inputs_number, node_number;
} sat_node;

node *csat_nodes;
sat_node *sat_root;
int sat_nodes;
int inputs_number, additional_inputs;

void print_csat(int output_no) {
	for (int i = 1; i <= output_no; i ++) {
		if (csat_nodes[i].input)
			printf("Input node: %d\n", i);
		else {
			printf("Gate: %d, type: %s, inputs: ", i, csat_nodes[i].gate == AND ? "AND" : (csat_nodes[i].gate == OR ? "OR" : "NOT"));
			for (int j = 0; j < csat_nodes[i].inputs_number; j++)
				printf("|%d| ", csat_nodes[i].inputs[j]);
			printf("\n");
		}
	}
}

void print_tabs(int tabs) {
	for (int i = 0; i < tabs; i++)
		printf("\t");
}

void print_sat(sat_node *node, int level)
{	
	print_tabs(level);
	printf("---- ");
	print_tabs(level);
	if (node->input)
		printf("Input node: %d\n", node->node_number);
	else {
		printf("Gate: %d, type: %s, inputs: \n", node->node_number, node->gate == AND ? "AND" : (node->gate == OR ? "OR" : "NOT"));
		for (int j = 0; j < node->inputs_number; j++)
			print_sat(node->inputs[j], level + 1);
	}
}

void init_nodes(node *nodes, int inputs, int total_nodes)
{
	for (int i = 1; i <= inputs; i++) {
		csat_nodes[i].input = true;
		csat_nodes[i].node_number = i;
	}

	for (int i = inputs + 1; i <= total_nodes; i++)
		csat_nodes[i].node_number = i;
}

char **string_split(char *line, int *components_no)
{
	*components_no = 0;
	char buffer[strlen(line) + 1];
	strcpy(buffer, line);

	char **components = NULL;
	char *token = strtok(buffer, " ");
	while (token) {
		if (token[strlen(token) - 1] == '\n')
			token[strlen(token) - 1] = '\0';

		*components_no += 1;
		components = realloc(components, *components_no * sizeof(char *));
		components[*components_no - 1] = calloc(strlen(token) + 1, sizeof(char));
		strcpy(components[*components_no - 1], token);

		token = strtok(NULL, " ");
	}

	return components;
}

void create_gate(char *gate_type, char *_gate_number, char **inputs, int inputs_number) {
	int gate_number = atoi(_gate_number);
	if (strcmp(gate_type, "AND") == 0)
		csat_nodes[gate_number].gate = AND;
	else if (strcmp(gate_type, "OR") == 0)
		csat_nodes[gate_number].gate = OR;
	else
		csat_nodes[gate_number].gate = NOT;

	csat_nodes[gate_number].input = false;
	csat_nodes[gate_number].inputs_number = inputs_number;
	csat_nodes[gate_number].inputs = calloc(inputs_number, sizeof(int));
	for (int i = 0; i < inputs_number; i++)
		csat_nodes[gate_number].inputs[i] = atoi(inputs[i]);
}

void copy_csat_in_sat(sat_node *node, int node_number) {
	node->node_number = node_number;
	node->gate = csat_nodes[node_number].gate;
	node->input = csat_nodes[node_number].input;
}

sat_node *csat_to_sat(int node_number) {
	sat_node *node = calloc(1, sizeof(sat_node));
	copy_csat_in_sat(node, node_number);

	if (node->input)
		return node;
	
	node->inputs_number = csat_nodes[node_number].inputs_number;
	node->inputs = calloc(node->inputs_number, sizeof(sat_node *));
	for (int i = 0; i < csat_nodes[node_number].inputs_number; i++)
		node->inputs[i] = csat_to_sat(csat_nodes[node_number].inputs[i]);

	return node;
}

sat_node *create_sat_node(GATE_TYPE type, bool input, int inputs_number) {
	sat_node *node = calloc(1, sizeof(sat_node));
	node->gate = type;
	node->input = input;
	node->inputs_number = inputs_number;
	if (!input)
		node->inputs = calloc(inputs_number, sizeof(sat_node *));

	sat_nodes += 1;
	node->node_number = sat_nodes;
	return node;
}

sat_node *create_additional_input_node() {
	sat_node *node = create_sat_node(AND, true, 0);
	additional_inputs += 1;
	node->node_number = inputs_number + additional_inputs;
	return node;
}

sat_node *empty_not_gate() {
	return create_sat_node(NOT, false, 1);
}

void free_sat_node(sat_node *node) {
	if (!node)
		return;

	if (!node->input)
		free(node->inputs);
	free(node);
}

void free_sat_tree(sat_node *node) {
	if (!node)
		return;

	for (int i = 0; i < node->inputs_number; i++)
		free_sat_tree(node->inputs[i]);
	free_sat_node(node);
}

void copy_sat_in_sat(sat_node *src, sat_node *dest) {
	src->node_number = dest->node_number;
	src->gate = dest->gate;
	src->input = dest->input;
	src->inputs_number = dest->inputs_number;

	if (src->inputs)
		free(src->inputs);
	if (src->input)
		return; // no inputs available for a input node
	
	src->inputs = calloc(dest->inputs_number, sizeof(sat_node *));
	for (int i = 0; i < dest->inputs_number; i++)
		src->inputs[i] = dest->inputs[i];
}

void add_input_to_node(sat_node *node, sat_node *input) {
	node->inputs_number += 1;
	node->inputs = realloc(node->inputs, node->inputs_number * sizeof(sat_node *));
	node->inputs[node->inputs_number - 1] = input;
}

void remove_input_from_node(sat_node *node, int index) {
	int initial_inputs = node->inputs_number;
	node->inputs_number -= 1;

	sat_node **inputs = calloc(node->inputs_number, sizeof(sat_node *));
	for (int i = 0; i < index; i++)
		inputs[i] = node->inputs[i];
	for (int i = index + 1; i < initial_inputs ; i++)
		inputs[i - 1] = node->inputs[i];

	free(node->inputs);
	node->inputs = inputs;
}

sat_node *clone_sat_node(sat_node *node) {
	sat_node *clone = create_sat_node(node->gate, node->input, node->inputs_number);
	if (node->input)
		clone->node_number = node->node_number;
	return clone;
}

sat_node *deep_clone_sat_tree(sat_node *node) {
	sat_node *clone = clone_sat_node(node);
	if (!node->input)
		for (int i = 0; i < node->inputs_number; i++)
			clone->inputs[i] = deep_clone_sat_tree(node->inputs[i]);
	return clone;
}

void split_not_gate(sat_node *node) {
	sat_node *gate = node->inputs[0];
	node->inputs_number = 0;
	free(node->inputs);
	node->inputs = calloc(gate->inputs_number, sizeof(sat_node *));

	for (int i = 0; i < gate->inputs_number; i++) {
		node->inputs[node->inputs_number] = empty_not_gate();
		node->inputs[node->inputs_number]->inputs[0] = gate->inputs[i];
		node->inputs_number += 1;
	}

	free_sat_node(gate);
}

void push_down_negations(sat_node *node) {
	// not gates have only one input
	if (node->inputs[0]->input)
		return;

	if (node->inputs[0]->gate == NOT) {
		sat_node *second_level_node = node->inputs[0]->inputs[0];
		free_sat_node(node->inputs[0]);
		copy_sat_in_sat(node, second_level_node);
		free_sat_node(second_level_node);
		return;
	} 

	node->gate = node->inputs[0]->gate == AND ? OR : AND;
	split_not_gate(node);
}

void simplify_gate(sat_node *node, int input_index) {
	sat_node *node_to_simplify = node->inputs[input_index];
	int initial_inputs = node->inputs_number;
	node->inputs_number = node->inputs_number + node_to_simplify->inputs_number - 1;

	sat_node **inputs = calloc(node->inputs_number, sizeof(sat_node *));
	for (int i = 0; i < initial_inputs; i++)
		inputs[i] = node->inputs[i];

	for (int i = 0; i < node_to_simplify->inputs_number - 1; i++)
		inputs[initial_inputs + i] = node_to_simplify->inputs[i];
	inputs[input_index] = node_to_simplify->inputs[node_to_simplify->inputs_number - 1];

	free(node->inputs);
	node->inputs = inputs;
	free_sat_node(node_to_simplify);
}

void simplify_sat_root(sat_node *node)
{
	if (node->input)
		return;

	// 1) break double negation
	if (node->gate == NOT)
		push_down_negations(node);

	// parse the logic tree
	for (int i = 0; i < node->inputs_number; i++)
		simplify_sat_root(node->inputs[i]);
	
	// 2) break cases like : x1 || (x2 || x3), should be (x1 || x2 || x3). Same for AND.
	int i = 0;
	while (i < node->inputs_number) {
		if (node->gate == node->inputs[i]->gate && !node->inputs[i]->input) {
			simplify_gate(node, i);
			continue;
		}

		i++;
	}
}

void make_or_gate(sat_node **new_inputs, int new_inputs_no, sat_node *parent, sat_node *node, int input_index) {
	sat_node *or_gate = create_sat_node(OR, false, 0);
	add_input_to_node(parent, or_gate);

	int start = input_index - 1;
	if (start < 0)
		start = 0;

	add_input_to_node(or_gate, deep_clone_sat_tree(node));
	for (int i = start; i < new_inputs_no; i++) {
		if (i == start && input_index > 0) {
			sat_node *not_gate = empty_not_gate();
			not_gate->inputs[0] = deep_clone_sat_tree(new_inputs[input_index - 1]);
			add_input_to_node(or_gate, not_gate);
			continue;
		}

		add_input_to_node(or_gate, deep_clone_sat_tree(new_inputs[i]));
	}
}

bool has_and_gate(sat_node *node) {
	for (int i = 0; i < node->inputs_number; i++)
		if (!node->inputs[i]->input && node->inputs[i]->gate == AND)
			return true;
	return false;
}

sat_node *perform_or_distribution(sat_node *node, sat_node *parent)
{
	int new_inputs_no = node->inputs_number - 1;
	sat_node **new_inputs = calloc(new_inputs_no, sizeof(sat_node *));
	for (int i = 0; i < new_inputs_no; i++)
		new_inputs[i] = create_additional_input_node();

	sat_node *and_gate = create_sat_node(AND, false, 0);
	if (parent)
		add_input_to_node(parent, and_gate);

	for (int i = 0; i < node->inputs_number; i++) {
		if (node->inputs[i]->gate != AND || node->inputs[i]->input) {
			make_or_gate(new_inputs, new_inputs_no, and_gate, node->inputs[i], i);
			continue;
		}

		for (int j = 0; j < node->inputs[i]->inputs_number; j++)
			make_or_gate(new_inputs, new_inputs_no, and_gate, node->inputs[i]->inputs[j], i);
	}

	for (int i = 0; i < new_inputs_no; i++)
		free_sat_node(new_inputs[i]);
	free(new_inputs);

	return and_gate;
}

int distribute_or_over_and(sat_node *node, sat_node *parent, int node_index) {
	if (!has_and_gate(node))
		return 0;

	perform_or_distribution(node, parent);
	remove_input_from_node(parent, node_index);
	free_sat_tree(node);
	simplify_sat_root(parent);
	return 1;
}

int sat_to_cnf_sat(sat_node *node, sat_node *parent, int node_index)
{
	if (node->input)
		return 0;

	int i = 0;
	while (i < node->inputs_number) {
		int stay_put = sat_to_cnf_sat(node->inputs[i], node, i);
		if (stay_put == 0)
			i += 1;
	}

	// guaranteed that NOT gates have been simplified to the max
	if (node->gate == NOT || !parent || node->gate == parent->gate)
		return 0;

	// 3) apply distribution rule to the maximum: x1 || x2 || (x3 && x4) = (x1 || x5 || x6) && (x2 || !x5 || x6) && (x3 || !x6) && (x4 || !x6)
	if (node->gate == OR)
		return distribute_or_over_and(node, parent, node_index);
	return 0;
}

sat_node *dnf_to_cnf(sat_node *sat_root) {
	if (!has_and_gate(sat_root))
		return sat_root;

	sat_node *new_root = perform_or_distribution(sat_root, NULL);
	simplify_sat_root(new_root);
	free_sat_tree(sat_root);
	return new_root;
}

void parse_input(char *input_file) {
	FILE *input = fopen(input_file, "r");
	int output_no;
	fscanf(input, "%d %d\n", &inputs_number, &output_no);
	csat_nodes = calloc(output_no + 1, sizeof(node));
	init_nodes(csat_nodes, inputs_number, output_no);

	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	while ((read = getline(&line, &len, input)) != -1) {
		int components_no = 0;
		char **components = string_split(line, &components_no);
		create_gate(components[0], components[components_no - 1], components + 1, components_no - 2);
		for (int i = 0; i < components_no; i++)
			free(components[i]);
		free(components);
	}
	fclose(input);
	if (line)
		free(line);

	sat_root = csat_to_sat(output_no);
	for (int i = 1; i <= output_no; i++)
		free(csat_nodes[i].inputs);
	free(csat_nodes);
}

void sat_to_cnf() {
	sat_nodes = sat_root->node_number;
	simplify_sat_root(sat_root);
	sat_to_cnf_sat(sat_root, NULL, 0);
	simplify_sat_root(sat_root);

	if (sat_root->gate == OR)
		sat_root = dnf_to_cnf(sat_root);
}

void print_as_dimacs(sat_node *root, char *output_file) {
	FILE *output = fopen(output_file, "w");

	fprintf(output, "p cnf %d %d\n", inputs_number + additional_inputs, root->inputs_number);
	for (int i = 0; i < root->inputs_number; i++) {
		if (root->inputs[i]->input) {
			fprintf(output, "%d 0\n", root->inputs[i]->node_number);
			continue;
		}
		if (root->inputs[i]->gate == NOT) {
			fprintf(output, "-%d 0\n", root->inputs[i]->inputs[0]->node_number);
			continue;
		}

		for (int j = 0; j < root->inputs[i]->inputs_number; j++) {
			int not_gate = root->inputs[i]->inputs[j]->gate == NOT;
			int node_number = not_gate ? root->inputs[i]->inputs[j]->inputs[0]->node_number : root->inputs[i]->inputs[j]->node_number;
			fprintf(output, "%s%d ", not_gate ? "-" : "", node_number);
		}
		fprintf(output, "0\n");
	}

	fclose(output);
}

int main(int argc, char *argv[])
{
	parse_input(argv[1]);
	sat_to_cnf();
	print_as_dimacs(sat_root, argv[2]);

	free_sat_tree(sat_root);
	return 0;
}