#pragma once
#include "chunkGeneration.h"


/* STRUCTS */
typedef struct Node{
	Chunk_t *chunk;
	struct Node *left;
	struct Node *right;
} Node_t;


/* FUNCTIONS */
Node_t *chunkTree_init();

//insert chunk in tree
void chunkTree_insert(Node_t *root, Chunk_t *chunk);

Chunk_t *chunkTree_pop(Node_t *root, int16_t x, int16_t y, int16_t z);

Chunk_t *chunkTree_peek(Node_t *root, int16_t x, int16_t y, int16_t z);

uint8_t chunkTree_exists(Node_t *root, int16_t x, int16_t y, int16_t z);

//recursive free memory
void chunkTree_destroy(Node_t *root);