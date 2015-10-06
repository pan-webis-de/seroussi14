/*
 * MIChain.h
 *
 *  Created on: Sep 16, 2015
 *      Author: thomas
 */

#ifndef MICHAIN_H_
#define MICHAIN_H_


#include <stdlib.h>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "Counter.h"
#include "Distribution.h"
#include <string>
#include <map>
#include <vector>
#include <iostream>
//#include <boost/timer.hpp>
//#include <boost/random/mersenne_twister.hpp>
//#include <boost/random/discrete_distribution.hpp>
#include <random>
#include <time.h>

class Dynamic_Counter;
class Static_Counter;


class MIChain {
public:

	std::string h5filename;		// Filename of the HDF5 file containing the data.
	int chain_id;				// ID of the chain. Use to distinguish different chains.

	int A;			// Number of authors.
	int D;			// Number of documents.
	int V;			// Number of words in vocabulary.
	int N_sw;		// Number of stopwords in vocabulary.
	int* N;			// Vector of number of words in each document.
	int N_words;	// Total number of words.
	int T_A;		// Number of author topics.
	int T_D	;		// Number of document topics.

	int* authors;		// Vector of author indices.
	int** words;		// Vector of word vectors.
	int** topics; 		// Vector of topic vectors.
	int** indicators;	// Vector of indicator vectors.

	double alpha_A;		// Author topic prior.
	double alpha_D;		// Document topic prior.

	double delta_A;		// Prior for author topic ratio.
	double delta_D;		// Prior for document topic ratio.

	double beta;		// Initial value of beta.
	double epsilon;		// Stopword parameter.
	double* beta_A;		// Word in author topic prior.
	double* beta_D;		// Word in document topic prior.

	double eta;		// Author in corpus prior.

	// Variables for sums needed for probability calculation.
	double sum_alpha_A;
	double sum_alpha_D;
	double sum_beta_A;
	double sum_beta_D;

	// Count variables.
	Static_Counter* c_a_AD;					// Count of documents of author a.
	Static_Counter* c_d_DA;      			// Count of words assigned to author topics in document d.
	Static_Counter* c_d_DD;					// Count of words assigned to document topics in document d.
	Static_Counter** c_dt_DT;				// Count of words assigned to document topic t in document d.
	Static_Counter** c_at_AT;				// Count of author topic t assignments to author a.
	Dynamic_Counter** c_tw_ATV;				// Count of word w in author topic t.
	Dynamic_Counter** c_tw_DTV;				// Count of word w in document topic t.

	// Distributions
	Distribution** theta_at_A;			// Author topic distributions.
	Distribution** theta_dt_D;			// Document topic distributions.
	Distribution** phi_tw_A;				// Author topic word distributions.
	Distribution** phi_tw_D;				// Document topic word distributions.
	double* pi_d;					// Topic ratios.
	Distribution* chi_a;					// Author distribution.

	std::map<int,std::pair<int,int> >* lookup_table;		// Lookuptable for combinations.
	int N_comb;										// Number of possible combinations of identifier and topics.

	int num_iter;		// Number of iterations.

	// Random number generator.
	int rng_seed;
//	boost::mt19937 rng;
	std::default_random_engine generator;

	// Vector of probabilities.
	std::vector<double> p;

	// Flag for first iteration.
	bool is_first_iteration;
	// Flag for save function.
	bool is_chain_group_created;

	// Log time consumption.
	double time_elapsed;


	MIChain(std::string h5filename, int id);
	virtual ~MIChain();

	void init_rng();

	void iterate(int count); 						// Iterate the chain count times.

	void sample(int d, int i);						// Sample from the probability distribution for i-th word in d-th document.
	void calc_prob(int w, int a, int y_old, int t_old, int d, int i);
//	int draw_boost();			// Draw from the discrete distribution given by weights p.
	int draw();
	void update_counts(int w, int d, int a, int y_old, int y_new, int t_old, int t_new);		// Update counts.

	int lookup_indicator(int ind);		// Return the topic indicator that belongs to the ind-th index in the probability vector.
	int lookup_topic(int ind);			// Return the topic that belongs to the ind-th index in the probability vector.

	void save_sample();					// Save counters, words, indicators and topics.
	void load_sample(int chain_id, int iteration);  	// Load counters, words, indicators and topics.

	bool check_sums();		// Check sums for consistency.

	void calc_distributions();	// Calculate the distributions from the counters.
};

#endif /* MICHAIN_H_ */
