//
// Created by Arch on 4/11/25.
//

#include "../src/bdd.h"
#include "../src/expression_parser.h"
#include "../src/utils.h"
#include <stdio.h>
#include <stdlib.h>

#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// Node in the BDD
typedef struct Node {
  int var;           // Variable index (-1 for terminal nodes)
  struct Node *high; // Child for variable = 1
  struct Node *low;  // Child for variable = 0
  int value;         // Terminal value (0 or 1) for terminal nodes
  struct Node *next; // For unique table
} Node;

// BDD structure
typedef struct BDD {
  int num_vars;    // Number of variables
  int size;        // Number of nodes
  Node *root;      // Root node
  char *var_order; // Variable ordering
} BDD;

// Unique table for nodes
typedef struct {
  Node **buckets;
  int size;
  int count; // Number of nodes in the table
} UniqueTable;

// Global unique table
UniqueTable unique_table;

// Initialize the unique table
void init_unique_table(int size) {
  if (unique_table.buckets != NULL) {
    free(unique_table.buckets);
  }
  unique_table.size = size;
  unique_table.count = 0;
  unique_table.buckets = (Node **)calloc(size, sizeof(Node *));
}

// Hash function for the unique table
int hash_node(int var, Node *low, Node *high) {
  unsigned long hash = (unsigned long)var * 101 + (unsigned long)low * 1009 +
                       (unsigned long)high * 10007;
  return (int)(hash % unique_table.size);
}

// Terminal nodes
Node *zero_terminal = NULL;
Node *one_terminal = NULL;

// Create a terminal node
Node *create_terminal(int value) {

  if (value == 0) {
    if (zero_terminal == NULL) {
      zero_terminal = (Node *)malloc(sizeof(Node));
      zero_terminal->var = -1;
      zero_terminal->low = zero_terminal->high = NULL;
      zero_terminal->value = 0;
      zero_terminal->next = NULL;
    }
    return zero_terminal;
  } else {
    if (one_terminal == NULL) {
      one_terminal = (Node *)malloc(sizeof(Node));
      one_terminal->var = -1;
      one_terminal->low = one_terminal->high = NULL;
      one_terminal->value = 1;
      one_terminal->next = NULL;
    }
    return one_terminal;
  }
}

// Find or add a node to the unique table
Node *find_or_add_node(int var, Node *low, Node *high) {
  // Apply reduction rules

  // Terminal case optimization: if both children are the same, return the child
  // directly
  if (low == high) {
    return low;
  }

  int hash = hash_node(var, low, high);
  Node *p = unique_table.buckets[hash];

  // Look for an existing node
  while (p != NULL) {
    if (p->var == var && p->low == low && p->high == high) {
      return p;
    }
    p = p->next;
  }

  // Create a new node
  Node *newNode = (Node *)malloc(sizeof(Node));
  newNode->var = var;
  newNode->low = low;
  newNode->high = high;
  newNode->value = -1; // Not a terminal node

  // Add to hash table
  newNode->next = unique_table.buckets[hash];
  unique_table.buckets[hash] = newNode;
  unique_table.count++;

  return newNode;
}

// Count variables in a Boolean function
int count_variables(const char *bfunkcia) {
  int max_var = -1;

  for (int i = 0; bfunkcia[i] != '\0'; i++) {
    if ((bfunkcia[i] >= 'A' && bfunkcia[i] <= 'Z') ||
        (bfunkcia[i] >= 'a' && bfunkcia[i] <= 'z')) {
      int var_idx;
      if (bfunkcia[i] >= 'A' && bfunkcia[i] <= 'Z') {
        var_idx = bfunkcia[i] - 'A';
      } else {
        var_idx = bfunkcia[i] - 'a';
      }
      if (var_idx > max_var)
        max_var = var_idx;
    }
  }

  return max_var + 1;
}

// Free the BDD
void BDD_free(BDD *bdd) {
  if (!bdd)
    return;

  if (bdd->var_order) {
    free(bdd->var_order);
  }

  free(bdd);
}

// Evaluate a Boolean function for a given input
int eval_boolean_function(const char *bfunkcia, const char *inputs) {
  int result = 0;
  int i = 0;

  while (bfunkcia[i] != '\0') {
    int term_result = 1;

    // Process one product term
    while (bfunkcia[i] != '\0' && bfunkcia[i] != '+') {
      if (bfunkcia[i] >= 'A' && bfunkcia[i] <= 'Z') {
        int var_idx = bfunkcia[i] - 'A';
        if (var_idx < strlen(inputs)) {
          int var_value = inputs[var_idx] - '0';
          term_result &= var_value;
        }
      } else if (bfunkcia[i] >= 'a' && bfunkcia[i] <= 'z') {
        int var_idx = bfunkcia[i] - 'a';
        if (var_idx < strlen(inputs)) {
          int var_value = inputs[var_idx] - '0';
          term_result &= var_value;
        }
      }
      i++;
    }

    result |= term_result;

    // Skip the '+' separator
    if (bfunkcia[i] == '+') {
      i++;
    }
  }

  return result;
}

// Create a BDD for a minterm (a single input combination that produces a 1)
Node *create_minterm_bdd(int input_combination, int num_vars,
                         const char *var_order) {
  Node *curr = create_terminal(1);

  // Build the path from bottom up
  for (int i = num_vars - 1; i >= 0; i--) {
    int var_idx = var_order[i] - 'A';
    int bit_value = (input_combination >> var_idx) & 1;

    if (bit_value == 0) {
      curr = find_or_add_node(i, curr, create_terminal(0));
    } else {
      curr = find_or_add_node(i, create_terminal(0), curr);
    }
  }

  return curr;
}

// Apply operation (OR) between two BDDs
Node *apply_or(Node *f, Node *g) {
  // Terminal cases
  if (f->var == -1 && g->var == -1) {
    return create_terminal(f->value | g->value);
  }

  return (f->var == -1 && f->value == 1)   ? f
         : (g->var == -1 && g->value == 1) ? g
         : (f->var == -1 && f->value == 0) ? g
         : (g->var == -1 && g->value == 0) ? f
                                           : NULL;

  // Determine the top variable
  int var;
  if (f->var == -1) {
    var = g->var;
  } else if (g->var == -1) {
    var = f->var;
  } else {
    var = (f->var < g->var) ? f->var : g->var;
  }

  // Extract children based on the top variable
  Node *f_low = (f->var == var) ? f->low : f;
  Node *f_high = (f->var == var) ? f->high : f;
  Node *g_low = (g->var == var) ? g->low : g;
  Node *g_high = (g->var == var) ? g->high : g;

  // Recursive calls
  Node *low_result = apply_or(f_low, g_low);
  Node *high_result = apply_or(f_high, g_high);

  // Create a new node and add to the unique table
  return find_or_add_node(var, low_result, high_result);
}

// Build a BDD from a Boolean function and variable ordering
Node *build_bdd(const char *bfunkcia, const char *var_order, int num_vars) {
  // Initialize with the 0 function
  Node *bdd = create_terminal(0);

  // For each possible input combination
  for (int i = 0; i < (1 << num_vars); i++) {
    // Create input string
    char *inputs = (char *)malloc(num_vars + 1);
    if (!inputs) {
      fprintf(stderr, "Memory allocation failed for inputs\n");
      exit(1);
    }

    for (int j = 0; j < num_vars; j++) {
      inputs[j] = ((i >> j) & 1) ? '1' : '0';
    }
    inputs[num_vars] = '\0';

    // Evaluate the function for this input
    int result = eval_boolean_function(bfunkcia, inputs);

    // If this input produces a 1, add the corresponding path to the BDD
    if (result == 1) {
      Node *minterm_bdd = create_minterm_bdd(i, num_vars, var_order);
      bdd = apply_or(bdd, minterm_bdd);
    }

    free(inputs);
  }

  return bdd;
}

// Count the number of nodes in the BDD
int count_nodes(Node *root, int *visited, int next_id) {
  if (root == NULL)
    return next_id;
  if (root->var == -1)
    return next_id; // Don't count terminal nodes

  // Check if already visited
  for (int i = 0; i < next_id; i++) {
    if (visited[i] == (int)(uintptr_t)root) {
      return next_id;
    }
  }

  // Mark as visited
  visited[next_id++] = (int)(uintptr_t)root;

  // Recursively count children
  next_id = count_nodes(root->low, visited, next_id);
  next_id = count_nodes(root->high, visited, next_id);

  return next_id;
}

// Create a BDD for a Boolean function with a given variable ordering
BDD *BDD_create(const char *bfunkcia, const char *poradie) {
  if (!bfunkcia || !poradie) {
    fprintf(stderr, "Invalid input parameters\n");
    return NULL;
  }

  int num_vars = count_variables(bfunkcia);
  if (strlen(poradie) < num_vars) {
    fprintf(stderr, "Variable ordering has insufficient variables\n");
    return NULL;
  }

  // Initialize terminal nodes
  zero_terminal = create_terminal(0);
  one_terminal = create_terminal(1);

  // Initialize unique table
  init_unique_table(10000);

  // Build the BDD
  Node *root = build_bdd(bfunkcia, poradie, num_vars);

  // Create the BDD structure
  BDD *bdd = (BDD *)malloc(sizeof(BDD));
  if (!bdd) {
    fprintf(stderr, "Memory allocation failed for BDD\n");
    exit(1);
  }

  bdd->num_vars = num_vars;
  bdd->root = root;

  // Copy the variable ordering
  bdd->var_order = (char *)malloc(strlen(poradie) + 1);
  if (!bdd->var_order) {
    fprintf(stderr, "Memory allocation failed for variable ordering\n");
    free(bdd);
    exit(1);
  }

  strcpy(bdd->var_order, poradie);

  // Count the nodes
  int *visited = (int *)calloc(unique_table.count + 1, sizeof(int));
  if (!visited) {
    fprintf(stderr, "Memory allocation failed for visited array\n");
    free(bdd->var_order);
    free(bdd);
    exit(1);
  }

  bdd->size = count_nodes(root, visited, 0);
  free(visited);

  return bdd;
}

// Generate a random variable ordering
char *generate_random_order(int num_vars) {
  char *order = (char *)malloc(num_vars + 1);
  if (!order) {
    fprintf(stderr, "Memory allocation failed for random ordering\n");
    exit(1);
  }

  // Initialize with sequential order
  for (int i = 0; i < num_vars; i++) {
    order[i] = 'A' + i;
  }
  order[num_vars] = '\0';

  // Shuffle
  for (int i = num_vars - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    char temp = order[i];
    order[i] = order[j];
    order[j] = temp;
  }

  return order;
}

// Evaluate the BDD for given input values
char BDD_use(BDD *bdd, const char *vstupy) {
  if (!bdd || !bdd->root || !vstupy) {
    return -1; // Error
  }

  Node *current = bdd->root;

  // Traverse the BDD
  while (current->var != -1) {
    int var_idx = current->var;

    if (var_idx >= strlen(bdd->var_order)) {
      return -1; // Error: variable index out of bounds
    }

    char var_name = bdd->var_order[var_idx];
    int input_idx = var_name - 'A';

    if (input_idx < 0 || input_idx >= strlen(vstupy)) {
      return -1; // Error: input index out of bounds
    }

    char input_value = vstupy[input_idx];

    // if (input_value == '0') {
    //   current = current->low;
    // } else if (input_value == '1') {
    //   current = current->high;
    // } else {
    //   return -1; // Error: invalid input value
    // }
    input_value == '0' ? current = current->low : current->high;

    if (!current) {
      return -1; // Error: NULL node
    }
  }

  // Return the terminal value
  return '0' + current->value;
}

// Generate a simple random Boolean function
char *generate_random_boolean_function(int num_vars, int num_terms) {
  if (num_terms <= 0)
    num_terms = 1;

  // Allocate memory for the function (overestimate for safety)
  char *function = (char *)malloc(num_vars * num_terms * 2 + num_terms);
  if (!function) {
    fprintf(stderr, "Memory allocation failed for random function\n");
    exit(1);
  }

  function[0] = '\0';

  for (int i = 0; i < num_terms; i++) {
    int term_length = 0;
    char term[27] = {0}; // Max 26 variables (A-Z) + null terminator

    // Generate a random term
    for (int j = 0; j < num_vars; j++) {
      if (rand() % 3 > 0) {
        // 2/3 chance to include variable
        char var = 'A' + j;
        term[term_length++] = var;
      }
    }

    // Ensure each term has at least one variable
    if (term_length == 0) {
      term[term_length++] = 'A' + (rand() % num_vars);
    }

    term[term_length] = '\0';

    // Add the term to the function
    if (i > 0) {
      strcat(function, "+");
    }
    strcat(function, term);
  }

  return function;
}

void free_unique_table() {
  // Safety check
  if (unique_table.buckets == NULL)
    return;

  // Free all nodes in the unique table except terminal nodes
  for (int i = 0; i < unique_table.size; i++) {
    Node *p = unique_table.buckets[i];
    while (p != NULL) {
      Node *next = p->next;
      // Don't free terminal nodes here - they're handled separately
      if (p != zero_terminal && p != one_terminal) {
        free(p);
      }
      p = next;
    }
  }

  free(unique_table.buckets);
  unique_table.buckets = NULL;
  unique_table.size = 0;
  unique_table.count = 0;
}

// Clone a BDD structure including all of its nodes
BDD *BDD_clone(BDD *source) {
  if (!source)
    return NULL;

  // Create a new BDD structure
  BDD *new_bdd = (BDD *)malloc(sizeof(BDD));
  if (!new_bdd) {
    fprintf(stderr, "Memory allocation failed for BDD clone\n");
    return NULL;
  }

  // Copy simple fields
  new_bdd->num_vars = source->num_vars;
  new_bdd->size = source->size;

  // Copy variable ordering
  new_bdd->var_order = (char *)malloc(strlen(source->var_order) + 1);
  if (!new_bdd->var_order) {
    fprintf(stderr,
            "Memory allocation failed for variable ordering in clone\n");
    free(new_bdd);
    return NULL;
  }
  strcpy(new_bdd->var_order, source->var_order);

  // The root pointer remains the same - we don't clone the nodes
  // since they're in the unique table and should be shared
  new_bdd->root = source->root;

  return new_bdd;
}

void BDD_reset_system() {
  // Free the unique table, which includes all nodes
  free_unique_table();

  // Reset terminal nodes
  if (zero_terminal != NULL) {
    free(zero_terminal);
    zero_terminal = NULL;
  }

  if (one_terminal != NULL) {
    free(one_terminal);
    one_terminal = NULL;
  }
}

BDD *BDD_create_with_best_order(const char *bfunkcia) {
  if (!bfunkcia) {
    fprintf(stderr, "Invalid input parameter\n");
    return NULL;
  }

  int num_vars = count_variables(bfunkcia);
  int min_size = INT_MAX;
  BDD *best_bdd = NULL;
  char *best_order = NULL;

  // Try at least num_vars different orderings
  for (int i = 0; i < num_vars; i++) {
    // Generate a new random ordering
    char *order = generate_random_order(num_vars);

    // Clean up and re-initialize before each BDD creation
    if (i > 0) {
      BDD_reset_system();
      zero_terminal = create_terminal(0);
      one_terminal = create_terminal(1);
      init_unique_table(10000);
    }

    // Create BDD with this ordering
    BDD *bdd = BDD_create(bfunkcia, order);

    if (bdd) {
      // If this is the best BDD so far, remember its size and ordering
      if (bdd->size < min_size) {
        min_size = bdd->size;

        // Save the best ordering
        if (best_order)
          free(best_order);
        best_order = strdup(order);

        // Clean up current BDD since we just need its size
        BDD_free(bdd);
      } else {
        // Not the best, clean up
        BDD_free(bdd);
      }
    }

    free(order);

    // Clean up after each iteration
    BDD_reset_system();
  }

  // Now create the final BDD with the best ordering we found
  if (best_order) {
    // Initialize for final BDD creation
    zero_terminal = create_terminal(0);
    one_terminal = create_terminal(1);
    init_unique_table(10000);

    best_bdd = BDD_create(bfunkcia, best_order);
    free(best_order);
  }

  return best_bdd;
}

// Modified test_bdd function to properly clean up all memory
void test_bdd() {
  // Initialize random seed
  srand((unsigned int)time(NULL));

  printf("Testing BDD implementation...\n");

  // Test with the simple example from the assignment
  {
    // Initialize structures for this test
    zero_terminal = create_terminal(0);
    one_terminal = create_terminal(1);
    init_unique_table(10000);

    BDD *bdd = BDD_create("AB+C", "ABC");
    if (!bdd) {
      fprintf(stderr, "Failed to create BDD for simple example\n");
      BDD_reset_system();
      return;
    }

    printf("Simple test with AB+C:\n");
    printf("BDD size: %d nodes\n", bdd->size);

    int errors = 0;

    if (BDD_use(bdd, "000") != '0') {
      printf("Error for A=0, B=0, C=0\n");
      errors++;
    }
    if (BDD_use(bdd, "001") != '1') {
      printf("Error for A=0, B=0, C=1\n");
      errors++;
    }
    if (BDD_use(bdd, "010") != '0') {
      printf("Error for A=0, B=1, C=0\n");
      errors++;
    }
    if (BDD_use(bdd, "011") != '1') {
      printf("Error for A=0, B=1, C=1\n");
      errors++;
    }
    if (BDD_use(bdd, "100") != '0') {
      printf("Error for A=1, B=0, C=0\n");
      errors++;
    }
    if (BDD_use(bdd, "101") != '1') {
      printf("Error for A=1, B=0, C=1\n");
      errors++;
    }
    if (BDD_use(bdd, "110") != '1') {
      printf("Error for A=1, B=1, C=0\n");
      errors++;
    }
    if (BDD_use(bdd, "111") != '1') {
      printf("Error for A=1, B=1, C=1\n");
      errors++;
    }

    printf("Simple test completed with %d errors\n\n", errors);

    BDD_free(bdd);
    BDD_reset_system();
  }

  // Number of variables to test (max 13 as per assignment)
  const int max_vars =
      6; // Reduced for testing, increase up to 13 for final version

  printf("Testing random functions with different variable counts:\n");

  for (int num_vars = 3; num_vars <= max_vars; num_vars++) {
    int total_nodes_direct = 0;
    int total_nodes_best_order = 0;
    int num_tests =
        10; // Reduced for testing, increase to 100 for final version

    printf("Testing with %d variables, %d random functions...\n", num_vars,
           num_tests);

    for (int i = 0; i < num_tests; i++) {
      // Generate a random Boolean function
      char *function =
          generate_random_boolean_function(num_vars, rand() % 3 + 1);

      // Create a default variable ordering
      char *default_order = (char *)malloc(num_vars + 1);
      if (!default_order) {
        fprintf(stderr, "Memory allocation failed for default ordering\n");
        free(function);
        continue;
      }

      for (int j = 0; j < num_vars; j++) {
        default_order[j] = 'A' + j;
      }
      default_order[num_vars] = '\0';

      // Initialize for direct BDD creation
      zero_terminal = create_terminal(0);
      one_terminal = create_terminal(1);
      init_unique_table(10000);

      // Create BDD with default ordering
      BDD *bdd_direct = BDD_create(function, default_order);

      if (!bdd_direct) {
        fprintf(stderr, "Failed to create BDD for function: %s\n", function);
        free(default_order);
        free(function);
        BDD_reset_system();
        continue;
      }

      // Save the size and free resources
      int direct_size = bdd_direct->size;
      BDD_free(bdd_direct);
      BDD_reset_system();

      // Initialize for best ordering
      zero_terminal = create_terminal(0);
      one_terminal = create_terminal(1);
      init_unique_table(10000);

      // Create BDD with best ordering
      BDD *bdd_best = BDD_create_with_best_order(function);

      if (!bdd_best) {
        fprintf(stderr, "Failed to create best-ordered BDD for function: %s\n",
                function);
        free(default_order);
        free(function);
        BDD_reset_system();
        continue;
      }

      // Collect statistics
      total_nodes_direct += direct_size;
      total_nodes_best_order += bdd_best->size;

      printf("Function %d: %s, Direct: %d nodes, Best: %d nodes\n", i + 1,
             function, direct_size, bdd_best->size);

      // Verify the BDD produces correct results
      int bdd_errors = 0;
      for (int j = 0; j < (1 << num_vars); j++) {
        char *inputs = (char *)malloc(num_vars + 1);
        if (!inputs) {
          fprintf(stderr, "Memory allocation failed for inputs\n");
          continue;
        }

        for (int k = 0; k < num_vars; k++) {
          inputs[k] = ((j >> k) & 1) ? '1' : '0';
        }
        inputs[num_vars] = '\0';

        int expected = eval_boolean_function(function, inputs);
        char expected_char = expected ? '1' : '0';

        char result_best = BDD_use(bdd_best, inputs);

        if (result_best != expected_char) {
          printf("Error: Function %s, Inputs %s, Expected %c, Got Best: %c\n",
                 function, inputs, expected_char, result_best);
          bdd_errors++;
        }

        free(inputs);
      }

      if (bdd_errors > 0) {
        printf("Function %s had %d evaluation errors\n", function, bdd_errors);
      }

      // Clean up everything
      BDD_free(bdd_best);
      free(function);
      free(default_order);
      BDD_reset_system();
    }

    // Calculate statistics
    float avg_direct = (float)total_nodes_direct / num_tests;
    float avg_best = (float)total_nodes_best_order / num_tests;
    float reduction_percent = 100.0f * (1.0f - avg_best / avg_direct);

    printf("\nResults for %d variables:\n", num_vars);
    printf("Average nodes (direct ordering): %.2f\n", avg_direct);
    printf("Average nodes (best ordering): %.2f\n", avg_best);
    printf("Average reduction: %.2f%%\n\n", reduction_percent);
  }

  printf("BDD testing completed\n");
}

int main() {
  // Initialize everything to NULL
  unique_table.buckets = NULL;
  unique_table.size = 0;
  unique_table.count = 0;
  zero_terminal = NULL;
  one_terminal = NULL;

  test_bdd();

  return 0;
}
