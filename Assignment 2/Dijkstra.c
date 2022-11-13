// Dijkstra API implementation
// COMP2521 Assignment 2

// COMP2521 Assignment 2- Dijkstra's Algorithm
//
// Program which is designed to find all of the shortest paths within 
// a given graph. Will provide a pred array and distances array which
// indicate its predeccesors and minimum distances to destination vertex from 
// a given source vertex
//
// Authors:
// Viraj Srikar Danthurty (z5359063@unsw.edu.au)
//
// Written: July/August 2021

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "Dijkstra.h"
#include "Graph.h"
#include "PQ.h"

///////////////////////////////////////////////////////////////////////////
//                        DIJKSTRA'S ALGORITHM                           //
///////////////////////////////////////////////////////////////////////////

ShortestPaths dijkstra(Graph g, Vertex src) {
	
	ShortestPaths sps;

	// Set the source vertex and number of vertex variables in the struct
	sps.src = src;
	int num_vertices = GraphNumVertices(g);
	sps.numNodes = num_vertices;

	PQ queue = PQNew();

	sps.dist = malloc(num_vertices * sizeof(int));
	sps.pred = malloc(num_vertices * sizeof(PredNode *));
	
	// Initalise the queue and array
	for (int i = 0; i < num_vertices; i++) {
		PQInsert(queue, i, INFINITY);
		if (i == src) { // If it is the source vertex there is no predecessor
			sps.pred[i] = malloc(sizeof(PredNode));
			sps.pred[i] = NULL;
			sps.dist[i] = INFINITY;
		} else {
			sps.pred[i] = malloc(sizeof(PredNode));
			sps.pred[i]->v = -1;
			sps.pred[i]->next = NULL;
			sps.dist[i] = INFINITY;
		}
	}

	// Set inital distance to 0 and queue up the source vertex
	sps.dist[src] = 0;
	PQInsert(queue, src, 0);
	
	AdjList vertex_neighbours;
	// Iterate through all of the vertex's in the graph 
	while (!PQIsEmpty(queue)) {
		int vertex = PQDequeue(queue);
		vertex_neighbours = GraphOutIncident(g, vertex);
		while (vertex_neighbours != NULL) {
			// Relax the weight between the vertices
			int alt_distance = sps.dist[vertex] + vertex_neighbours->weight;
			if (alt_distance >= 0 && alt_distance < sps.dist[vertex_neighbours->v]) {
				PQInsert(queue, vertex_neighbours->v, alt_distance);
				
				sps.dist[vertex_neighbours->v] = alt_distance;

				// Below is the process of freeing all of the linked lists 
				// within the pred index value, reallocating space and updating 
				// the vertex value 
				PredNode *curr = sps.pred[vertex_neighbours->v];
				PredNode *next = NULL;
				while (curr != NULL) {
					next = curr->next;
					free(curr);
					curr = next;
				}
				// Allocate space in the index value for a linked list
				sps.pred[vertex_neighbours->v] = malloc(sizeof(PredNode));
				PredNode *NewNode;
				// Create a new node and allocate the vertex value
				NewNode = malloc(sizeof(PredNode));
				NewNode->v = vertex;
				NewNode->next = NULL;
				sps.pred[vertex_neighbours->v] = NewNode;
			} else if (alt_distance >= 0 && alt_distance == sps.dist[vertex_neighbours->v]) {
				PredNode *temp = sps.pred[vertex_neighbours->v];

				while (temp->next != NULL) {
					temp = temp->next;
				}
				// Add new node to the end of the linked list
				PredNode *newNode = malloc(sizeof(PredNode));
				newNode->v = vertex;
				newNode->next = NULL;
				temp->next = newNode;				
			}

			vertex_neighbours = vertex_neighbours->next;
		}
	}

	// Checking if the distance is infinity, then change the pred array
	for (int x = 0; x < num_vertices; x++) {
		if (sps.dist[x] == INFINITY) {
			sps.pred[x] = NULL;
		}
	}

	PQFree(queue);
	return sps;
}

void showShortestPaths(ShortestPaths sps) {

}

void freeShortestPaths(ShortestPaths sps) {

	// Free the dist array 
	free(sps.dist);

	// Iterate through the pred array 
	for (int i = 0; i < sps.numNodes; i++) {
		// In each array index value, free all of the nodes in the linked list
		PredNode *curr = sps.pred[i];
		while (curr != NULL) {
			PredNode *temp = curr;
			curr = curr->next;
			free(temp);
		}
	}

	// Free the pred array
	free(sps.pred);
}
