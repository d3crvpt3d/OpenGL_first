#include "chunkTree.h"

uint8_t smaller(Chunk_t *a, Chunk_t *b){
	return (
		( (uint64_t) a->x ) << 32 | ( (uint64_t) a->y ) << 16 | ( (uint64_t) a->z )
	)
	< (
		( (uint64_t) b->x ) << 32 | ( (uint64_t) b->y ) << 16 | ( (uint64_t) b->z )
	);
}

Node_t *chunkTree_init(){
	//TODO:
	return NULL;
}

void chunkTree_insert(Node_t *root, Chunk_t *chunk){
	if(root == NULL){
		root = (Node_t *) malloc(sizeof(Node_t));
		root->chunk = chunk;
		root->left = NULL;
		root->right = NULL;
		return;
	}
	
	//insert left if &chunk is smaller than &node_chunk
	if(smaller(chunk, root->chunk)){

		if(root->left == NULL){
			root->left = (Node_t *) malloc(sizeof(Node_t));
			root->left->chunk = chunk;
			root->left->left = NULL;
			root->left->right = NULL;
		}else{
			chunkTree_insert(root->left, chunk);
		}
		
	}else{

		if(root->right == NULL){
			root->right = (Node_t *) malloc(sizeof(Node_t));
			root->right->chunk = chunk;
			root->right->left = NULL;
			root->right->right = NULL;
		}else{
			chunkTree_insert(root->right, chunk);
		}
		
	}

}

Chunk_t *chunkTree_pop(Node_t *root, int16_t x, int16_t y, int16_t z){
	//TODO:
}

Chunk_t *chunkTree_peek(Node_t *root, int16_t x, int16_t y, int16_t z){
	//TODO:
}

uint8_t chunkTree_exists(Node_t *root, int16_t x, int16_t y, int16_t z){
	//TODO:
}

void chunkTree_destroy(Node_t *root){
	//TODO:
}