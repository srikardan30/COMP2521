// Lance-Williams Algorithm for Hierarchical Agglomerative Clustering
// COMP2521 Assignment 2

// COMP2521 Assignment 2- CentralityMeasures
//
// Program designed to find clusters within a given graph and will output 
// an dendrogram in hierarchical order
//
// Authors:
// Viraj Srikar Danthurty (z5359063@unsw.edu.au)
//
// Written: July/August 2021

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "Graph.h"
#include "LanceWilliamsHAC.h"

#define INFINITY DBL_MAX

///////////////////////////////////////////////////////////////////////////
//                        FUNCTION PROTOTYPES                            //
///////////////////////////////////////////////////////////////////////////

static int adjust_dendA_array(Dendrogram *dendA, int shortest_x, int shortest_y, 
	int num_vertices);

static void new_dend(Dendrogram *dendA, int i);

static void max_weight(Graph g, double *incoming_weight, 
	double *outgoing_weight, int i, int j);

static void min_nodes(int num_vertices, int i, double *minimum_distance, 
	double distance[num_vertices][num_vertices], 
		int *shortest_x, int *shortest_y);

static void update_cluster_distance(int num_vertices, int i, int shortest_x, 
	int shortest_y, int method, double distance[num_vertices][num_vertices],
		 double new_cluster_dist[num_vertices - i]);

static void update_removed_clusters(int shortest_x, int shortest_y, 
	int num_vertices, double distance[num_vertices][num_vertices], int temp, 
		int i, double new_cluster_dist[num_vertices - i]);



/**
 * Generates  a Dendrogram using the Lance-Williams algorithm (discussed
 * in the spec) for the given graph  g  and  the  specified  method  for
 * agglomerative  clustering. The method can be either SINGLE_LINKAGE or
 * COMPLETE_LINKAGE (you only need to implement these two methods).
 * 
 * The function returns a 'Dendrogram' structure.
 */
Dendrogram LanceWilliamsHAC(Graph g, int method) {

///////////////////////////////////////////////////////////////////////////
//                             INITALISATION                             //
///////////////////////////////////////////////////////////////////////////

	int num_vertices = GraphNumVertices(g);

    double distance[num_vertices][num_vertices];

    Dendrogram dendA[num_vertices];

    for (int i = 0; i < num_vertices; i++) {

        for (int j = i; j < num_vertices; j++) {

			// If there is NO direct edge, set to INFINITY
            if (GraphIsAdjacent(g, i, j) == false && 
				GraphIsAdjacent(g, j, i) == false) {

                distance[i][j] = INFINITY;
				distance[j][i] = INFINITY;
            }
			
            // Find the MAXIMUM edge weight between two vertices
            double outgoing_weight = 0;
            double incoming_weight = 0;

			max_weight(g, &incoming_weight, &outgoing_weight, i, j);
			
			double wt;
			if (outgoing_weight - incoming_weight > 0) {
				wt = outgoing_weight;
			} else {
				wt = incoming_weight;
			}

			// Perform calculation and update in distance array 
			distance[i][j] = (1 / wt);
			distance[j][i] = (1 / wt);
        }

		// Pass into helper function to create inital dendrograms in dendA
		new_dend(dendA, i);
    }


///////////////////////////////////////////////////////////////////////////
//                     MERGING AND CREATING CLUSTERS                     //
///////////////////////////////////////////////////////////////////////////

	// Iterate through each vertex, find the closest neighbouring vertex
	// merge them into a new cluster, update the dist and dendA array 
    for (int i = 1; i < num_vertices; i++) {

        double minimum_distance = INFINITY;
        int shortest_x = 0;
        int shortest_y = 0;
		// Find the vertices in the distance array with minimum distance
		min_nodes(num_vertices, i, &minimum_distance, distance, &shortest_x, 
			&shortest_y);

		// Create new dendrogram nodes to store the values 
        Dendrogram clusterI = dendA[shortest_x];
        Dendrogram clusterJ = dendA[shortest_y];

		// Update the dendA array to accomidate new clusters	
		int temp = adjust_dendA_array(dendA, shortest_x, shortest_y, 
			num_vertices);

		// To make space for new cluster to store the previous clusters
		Dendrogram new_dendrogram = malloc(sizeof(Dendrogram));

		dendA[num_vertices - i - 1] = new_dendrogram;
		// Order is irrelevant
		dendA[num_vertices - i - 1]->right = clusterI;
		dendA[num_vertices - i - 1]->left = clusterJ;
		dendA[num_vertices - i - 1]->vertex = -1;
		
        // Array to store new distances from clusters
        double new_cluster_dist[num_vertices - i];

		// Readjust the distance array with the addition of new clusters
		update_cluster_distance(num_vertices, i, shortest_x, shortest_y, method, 
			distance, new_cluster_dist);

        // Remove the previous two independant clusters, update to new
		// cluster distances 
		update_removed_clusters(shortest_x, shortest_y, num_vertices, distance, 
			temp, i, new_cluster_dist);
    }

    return dendA[0];
}

// Helper function which adjusts the dendA array to accomidate for the new
// clusters that are to be made 
static int adjust_dendA_array(Dendrogram *dendA, int shortest_x, int shortest_y, 
	int num_vertices) {

	// Shift all of the contents of array across
	int j = shortest_x;
	while (j < (num_vertices - 1)) {
		dendA[j] = dendA[j + 1];
		j++;
	}

	int temp = shortest_y;
	if (shortest_y > shortest_x) {
		temp = temp - 1;
	}

	// Place in new cluster items into the array space 
	for (int j = temp; j < (num_vertices - 1); j++) {
		dendA[j] = dendA[j + 1];
	}

	return temp;
}

// This helper function creates space within the INITIAL dendA array, 
// allocating enough space within each index to store a dendrogram
static void new_dend(Dendrogram *dendA, int i) {

	Dendrogram new_dendrogram = malloc(sizeof(Dendrogram));

	dendA[i] = new_dendrogram;
	dendA[i]->right = NULL;
	dendA[i]->left = NULL;
	// Initally all of the vertex values are their respective number
	dendA[i]->vertex = i;

}

// This helper function traverses through the distance array and seeks the 
// smallest distance value and returns its weight 
static void max_weight(Graph g, double *incoming_weight, 
	double *outgoing_weight, int i, int j) {

	// Need to check for maximum weight from both outgoing and incoming weights 
	AdjList incoming = GraphInIncident(g, i);
	AdjList outgoing = GraphOutIncident(g, i);
	
	// Iterate through the linked list 
	while (incoming != NULL) {
		if (incoming->v == j) {
			(*incoming_weight) = incoming->weight;
		}
		incoming = incoming->next;
	}

	while (outgoing != NULL) {
		if (outgoing->v == j) {
			(*outgoing_weight) = outgoing->weight;
		}
		outgoing = outgoing->next;
	}

}

// Helper function to seek the clusters with the minimum distance between them
// to then form into a new cluster
static void min_nodes(int num_vertices, int i, double *minimum_distance, 
	double distance[num_vertices][num_vertices], 
		int *shortest_x, int *shortest_y) {

	// Traverse through the entire distance array to find clusters with 
	// shortest distance 
	int j = 0;
	while (j < (num_vertices - i + 1)) {
		int k = j;
		while (k < (num_vertices - i + 1)) {
			if ((*minimum_distance) >= distance[j][k]) {
				if (j != k) { 
					// Update the variables once found 
					(*minimum_distance) = distance[j][k];
					(*shortest_x) = j;
					(*shortest_y) = k;
				}
			}
			k++;
		}
		j++;
	}
}

// Helper function which will update the distances between the clusters. 
// Will do so depending on the desired method passed in. Then will update 
// the distance array with new values 
static void update_cluster_distance(int num_vertices, int i, int shortest_x, 
	int shortest_y, int method, double distance[num_vertices][num_vertices],
		 double new_cluster_dist[num_vertices - i]) {

	int counter = 0;

	for (int x = 0; x < (num_vertices - i + 1); x++) {

		if (x == shortest_x || x == shortest_y) {
			continue;
		}

		// Depending on passed in method, find neccessary distance 
		if (method == SINGLE_LINKAGE) { // Single linkage looks for the 
		// most minimal distace between vertices in clusters
			if (distance[shortest_x][x] < distance[shortest_y][x]) {
				new_cluster_dist[counter++] = distance[shortest_x][x];
			} else {
				new_cluster_dist[counter++] = distance[shortest_y][x];
			}
		} else { // method == COMPLETE_LINKAGE
		// Complete linkage seeks for largest distace between vertices 
		// in clusters
			if (distance[shortest_x][x] > distance[shortest_y][x]) {
				new_cluster_dist[counter++] = distance[shortest_x][x];
			} else {
				new_cluster_dist[counter++] = distance[shortest_y][x];
			}
		}
	}
}

// Helper function which updates the distance array after the previous 
// clusters which were merged are removed. places the new distance based on 
// method into the array
static void update_removed_clusters(int shortest_x, int shortest_y, 
	int num_vertices, double distance[num_vertices][num_vertices], int temp, 
		int i, double new_cluster_dist[num_vertices - i]) {

	// Remove the first cluster from distance array
	int row = shortest_x;
	while (row < num_vertices) {
		for (int col = 0; col < num_vertices; col++) {
			distance[col][row] = distance[col][row + 1];
			distance[row][col] = distance[row + 1][col];
		}
		row++;
	}

	// Remove second cluster from distance array
	int row2 = temp;
	while (row2 < num_vertices) {
		for (int col = 0; col < num_vertices; col++) {
			distance[col][row2] = distance[col][row2 + 1];
			distance[row2][col] = distance[row2 + 1][col];
		}
		row2++;
	}

	// Update distance array with new distances from new clusters
	int j = 0;
	while (j < num_vertices - i) {
		distance[num_vertices - i - 1][j] = new_cluster_dist[j];
		distance[j][num_vertices - i - 1] = new_cluster_dist[j];
		j++;
	}
}


/**
 * Frees all memory associated with the given Dendrogram structure.
 */
void freeDendrogram(Dendrogram d) {

	if (d == NULL) {
		return;
	} 

    freeDendrogram(d->left);
    freeDendrogram(d->right);
    free(d);
}
