#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BUFFER_SIZE 2048
#define NUMBER_OF_ASCII_CODE 128

struct Node {
    size_t count;
    char code[NUMBER_OF_ASCII_CODE];
    size_t code_len;
    struct Node* left;
    struct Node* right;
} typedef Node;

size_t* create_word_count_from(char* filename) {
    size_t* word_count = malloc(sizeof(size_t) * NUMBER_OF_ASCII_CODE);
    if ( word_count == NULL ) {
        printf("malloc is failed in function create_word_count_from()\n");
        exit(1);
    }
    memset(word_count, 0, sizeof(size_t)*NUMBER_OF_ASCII_CODE);

    char buffer[BUFFER_SIZE];
    size_t num_of_chars;
    FILE* f = fopen(filename, "r");
    if ( f == NULL ) {
        printf("The designated file can't be open. ");
        exit(1);
    }

    while ( ( num_of_chars = fread(buffer, sizeof(char), BUFFER_SIZE, f) ) > 0 ) {
        for (size_t i = 0; i < num_of_chars; i++) {
            word_count[(size_t) buffer[i]]++;
        }
    }

    fclose(f);

    return word_count;
}

size_t sum_word_count(size_t* word_count) {
    size_t total_word_count = 0;
    for (size_t i = 0; i < NUMBER_OF_ASCII_CODE; i++) {
        total_word_count += word_count[i];
    }
    return total_word_count;
}

size_t* create_cluster_count(size_t* word_count) {
    size_t* cluster_count = malloc(sizeof(size_t) * (2 * NUMBER_OF_ASCII_CODE - 1));
    if (cluster_count == NULL) {
        printf("malloc is failed in function create_cluster_count()\n");
        exit(1);
    }

    for (size_t i = 0; i < NUMBER_OF_ASCII_CODE; i++) {
        cluster_count[i] = word_count[i];
    }

    for (size_t i = NUMBER_OF_ASCII_CODE; i < 2 * NUMBER_OF_ASCII_CODE-1; i++) {
        cluster_count[i] = SIZE_MAX; // SIZE_MAXは無効なクラスタであることを表す
    }

    return cluster_count;
}

void initialize_cluster_list(Node* cluster_list, size_t len_of_list, size_t* cluster_count) {
    for (size_t i = 0; i < len_of_list; i++) {
        cluster_list[i].count = cluster_count[i];
        cluster_list[i].code[0] = '\0';
        cluster_list[i].code_len = 0;
        cluster_list[i].left = NULL;
        cluster_list[i].right = NULL;
    }
}

double calc_entropy(Node* cluster_list, size_t num_of_leaf, size_t total_word_count) {
    double entropy = 0.0;

    for (size_t i = 0; i < num_of_leaf; i++) {
        size_t word_count = cluster_list[i].count;
        if (word_count == 0) {
            continue;
        } else {
            double probability = (double) word_count / (double) total_word_count;
            entropy += - probability * log2(probability);
        }
    }

    return entropy;
}

double calc_averaged_code_length(Node* cluster_list, size_t num_of_leaf, size_t total_word_count) {
    double averaged_code_length = 0.0;

    for (size_t i = 0; i < num_of_leaf; i++) {
        size_t word_count = cluster_list[i].count;
        if (word_count == 0) {
            continue;
        } else {
            double probability = (double) word_count / (double) total_word_count;
            averaged_code_length += probability * (double) cluster_list[i].code_len;
        }
    }

    return averaged_code_length;
}

size_t calc_file_size(Node* cluster_list, size_t num_of_leaf) {
    size_t file_size = 0;
    for (size_t i = 0; i < num_of_leaf; i++) {
        file_size += cluster_list[i].code_len * cluster_list[i].count;
    }
    return file_size;
}

void print_code(Node* cluster_list, size_t num_of_leaf) {
    for (size_t i = 0; i < num_of_leaf; i++) {
        printf("%lld: (code: %s, code_len: %lld, count: %lld)\n", i, cluster_list[i].code, cluster_list[i].code_len, cluster_list[i].count);
    }
}

void search_mins(size_t* array, size_t array_len, size_t* min1_index, size_t* min2_index) {
    size_t min1 = SIZE_MAX;
    size_t min2 = SIZE_MAX;
    for (size_t i = 0; i < array_len; i++) {
        if (array[i] <= min1) {
            min2 = min1;
            *min2_index = *min1_index;
            min1 = array[i];
            *min1_index = i;
        } else if (array[i] > min1 && array[i] < min2) {
            min2 = array[i];
            *min2_index = i;
        }
    }
}

void preorder_dps(Node* nodep) {
    if (nodep == NULL) {
        return;
    } else {
        if (nodep->left != NULL) {
            strncpy(nodep->left->code, nodep->code, nodep->code_len);
            nodep->left->code[nodep->code_len] = '0';
            nodep->left->code[nodep->code_len + 1] = '\0';
            nodep->left->code_len = nodep->code_len + 1;
            preorder_dps(nodep->left);
        }
        if (nodep->right != NULL) {
            strncpy(nodep->right->code, nodep->code, nodep->code_len);
            nodep->right->code[nodep->code_len] = '1';
            nodep->right->code[nodep->code_len + 1] = '\0';
            nodep->right->code_len = nodep->code_len + 1;
            preorder_dps(nodep->right);
        }
    }
}

int main(int argc, char** argv) {

    if (argc == 2) {
        char* filename = argv[1];
        Node cluster_list[2 * NUMBER_OF_ASCII_CODE - 1];
        size_t* word_count = create_word_count_from(filename);
        size_t total_word_count = sum_word_count(word_count);
        size_t* cluster_count = create_cluster_count(word_count);
        initialize_cluster_list(cluster_list, 2 * NUMBER_OF_ASCII_CODE - 1, cluster_count);

        // Make a dendrogram
        for (size_t i = 0; i < NUMBER_OF_ASCII_CODE - 1; i++) {
            size_t min1_index = SIZE_MAX;
            size_t min2_index = SIZE_MAX;
            search_mins(cluster_count, 2 * NUMBER_OF_ASCII_CODE - 1, &min1_index, &min2_index);

            cluster_count[NUMBER_OF_ASCII_CODE + i] 
            = cluster_count[min1_index] + cluster_count[min2_index];
            cluster_count[min1_index] = SIZE_MAX;
            cluster_count[min2_index] = SIZE_MAX;

            cluster_list[NUMBER_OF_ASCII_CODE + i].count = cluster_count[NUMBER_OF_ASCII_CODE + i];
            cluster_list[NUMBER_OF_ASCII_CODE + i].left = cluster_list + (min1_index);
            cluster_list[NUMBER_OF_ASCII_CODE + i].right = cluster_list + (min2_index);
        }

        Node* rootp = cluster_list + (NUMBER_OF_ASCII_CODE*2 - 2);
        preorder_dps(rootp); // Assign a code to each cluster

        print_code(cluster_list, NUMBER_OF_ASCII_CODE);
        printf("Averaged code length: %f\n", calc_averaged_code_length(cluster_list, NUMBER_OF_ASCII_CODE, total_word_count));
        printf("Entropy: %f\n", calc_entropy(cluster_list, NUMBER_OF_ASCII_CODE, total_word_count));
        printf("File Size: %lld bit\n", 7*total_word_count);
        printf("Compressed File size: %lld bit\n", calc_file_size(cluster_list, NUMBER_OF_ASCII_CODE));

        return 0;
    } else {
        printf("usage: ./compression.exe <File Name>\n");
        return 1;
    }
}