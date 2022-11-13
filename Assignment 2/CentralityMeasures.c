// Centrality Measures API implementation
// COMP2521 Assignment 2

// COMP2521 Assignment 2- CentralityMeasures
//
// Program which is designed to calculate the closeness centrality, 
// betweeness centrality or normalised betweeness centrality of a 
// given graph
//
// Written: July/August 2021


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "CentralityMeasures.h"
#include "Dijkstra.h"
#include "PQ.h"

///////////////////////////////////////////////////////////////////////////
//                        FUNCTION PROTOTYPES                            //
///////////////////////////////////////////////////////////////////////////

static void between_centrality(ShortestPaths shortest_paths, int v, 
	double *values, int t, double num_shortest_paths, double num_throughV);

static double find_sigma_values(ShortestPaths shortest_paths, int t, int v,
	double *num_throughV);

///////////////////////////////////////////////////////////////////////////
//                        CLOSENESS CENTRALITY                           //
///////////////////////////////////////////////////////////////////////////

NodeValues closenessCentrality(Graph g) {

	NodeValues nvs;

	int num_vertices = GraphNumVertices(g);
	nvs.numNodes = num_vertices;

	nvs.values = malloc(num_vertices * sizeof(double));

	ShortestPaths shortest_paths;	
	int counter = 0;
	double reachable_nodes;
	double vertex_sum;
	// Iterate through all of the verticies in the graph
	for (int i = 0; i < num_vertices; i++) {
		// Pass dequeued vertex into dijkstra function
		shortest_paths = dijkstra(g, i);
		reachable_nodes = 1;
		vertex_sum = 0;
		// Calculate the vertex sum and the number of reachable nodes
		for (int j = 0; j < num_vertices; j++) {
			if (shortest_paths.dist[j] != INFINITY && 
				shortest_paths.dist[j] != 0) {

				vertex_sum = vertex_sum + shortest_paths.dist[j];
				reachable_nodes++;
			}
		}
		
		double LHS, RHS;
		double LHS_numerator = reachable_nodes - 1;
		double LHS_denominator = num_vertices - 1;
		double RHS_numerator = reachable_nodes - 1;
		double RHS_denominator = vertex_sum;

		// Checking if LHS and RHS == 0 (to avoid divide by 0 error)
		if (LHS_denominator == 0 || LHS_numerator == 0) {
			LHS = 0;
		} else {
			LHS = (reachable_nodes - 1) / (num_vertices - 1);
		}

		if (RHS_denominator == 0 || RHS_numerator == 0) {
			RHS = 0;
		} else {
			RHS = (reachable_nodes - 1) / (vertex_sum);
		}

		double close_cen = LHS*RHS;
		// Update values array
		nvs.values[counter++] = close_cen;
		freeShortestPaths(shortest_paths);
	}

	return nvs;
}

///////////////////////////////////////////////////////////////////////////
//                        BETWEENNESS CENTRALITY                         //
///////////////////////////////////////////////////////////////////////////

NodeValues betweennessCentrality(Graph g) {
	
	NodeValues nvs;

	int num_vertices = GraphNumVertices(g);
	nvs.numNodes = num_vertices;

	nvs.values = malloc(num_vertices * sizeof(double));

	// Tnitalise values array to 0
	for (int i = 0; i < num_vertices; i++) {
		nvs.values[i] = 0;
	}	

	ShortestPaths shortest_paths;
	// Obtains a source vertex, passes it into te dijkstra function and 
	// then will pass the destination vertex into the between_centrality 
	// function to get the centrality
	for (int s = 0; s < num_vertices; s++) {
		// Get the shortest paths for the source vertex
		shortest_paths = dijkstra(g, s);
		for (int v = 0; v < num_vertices; v++) {
			// Check the source vertex is not the destination vertex
			if (v != shortest_paths.src) {
                for (int t = 0; t < num_vertices; t++) {
					double num_throughV = 0;
                    double num_shortest_paths = 0;
                    between_centrality(shortest_paths, v, nvs.values, t, 
						num_shortest_paths, num_throughV);
                }
			}
		}
		freeShortestPaths(shortest_paths);
	}

	return nvs;
}


// Helper function which will get the total number of paths between two 
// vertices and the occurance of vertex 'v'. Calculates the betweenness
// centrality and stores it in the values array
static void between_centrality(ShortestPaths shortest_paths, int v, 
	double *values, int t, double num_shortest_paths, double num_throughV) {

    num_throughV = 0;

    if (t != v && shortest_paths.src != t && 
		shortest_paths.dist[t] != INFINITY) {

        num_shortest_paths = find_sigma_values(shortest_paths, t, v, 
			&num_throughV);

		// Perform the betweenness centrality calculation
        double centrality = (num_throughV / num_shortest_paths);

		// Update the array
        values[v] = values[v] + centrality;
    }
}


// Helper function which calculates the total number of shortest paths between 
// two given vertices, along with calculating the number of times a certain 
// was encountered through the traversal
static double find_sigma_values(ShortestPaths shortest_paths, int t, int v, 
	double *num_throughV) {

	// Checking if 't' value passed in is the source vertex
	if (shortest_paths.pred[t] == NULL) {
		return 1;
	}
	
	double num_shortest_paths = 0;

	// Back track through the pred array 
	PredNode *temp = shortest_paths.pred[t];
	while (temp != NULL) {
		num_shortest_paths = num_shortest_paths + 
			find_sigma_values(shortest_paths, temp->v, v, num_throughV);

		temp = temp->next;
	}

	// Checking if the 'v' vertex was passed in the path
	if (t == v) {
		*num_throughV = *num_throughV + num_shortest_paths;
	}

	return num_shortest_paths;
}

///////////////////////////////////////////////////////////////////////////
//                   NORMALISED BETWEENNESS CENTRALITY                   //
///////////////////////////////////////////////////////////////////////////

NodeValues betweennessCentralityNormalised(Graph g) {

	NodeValues nvs;
	
	int num_vertices = GraphNumVertices(g);
	nvs.numNodes = num_vertices;

	nvs.values = malloc(num_vertices * sizeof(double));

	// Obtains the betweenness centrality of the given graph 
	NodeValues between_centrality = betweennessCentrality(g);

	for (int i = 0; i < num_vertices; i++) {
		double between_cen = between_centrality.values[i];

		// Performs the normalised betweenness centrality calculation
		double LHS_denominator = (num_vertices - 1)*(num_vertices - 2);
		double LHS = (1 / LHS_denominator);
		double normal_between = LHS * between_cen;
		
		// Stores the value in the array
		nvs.values[i] = normal_between;
	}
	
	return nvs;
}


void showNodeValues(NodeValues nvs) {

}


void freeNodeValues(NodeValues nvs) {

		// Free the values array 
		free(nvs.values);
}
