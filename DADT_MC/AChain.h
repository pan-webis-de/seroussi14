/*
 * AChain.h
 *
 *  Created on: Sep 25, 2015
 *      Author: thomas
 */

#ifndef ACHAIN_H_
#define ACHAIN_H_

#include "MIChain.h"
#include "Counter.h"

class AChain {
	// Class for Markov chain for author attribution phase.
public:
	MIChain* mc;			// The Markov Chain used for model inference. Use this object to load the samples and calculate distributions rather than reimplementing the load routine.

	int test_text_id;	// ID of the test text.

	int N;				// Number of words in document.
	int* words;			// Array of words labeled according to the vocabulary of the corpus. Assign negative integers for previously unknown words.
	int* indicators;	// Topic indicators.
	int* topics;		// Array of topics.

	Static_Counter* c_DA;			// Count of words assigned to author topics.
	Static_Counter* c_DD;			// Count of words assigned to document topics.

	Static_Counter* c_t_AT;		// Count of words assigned to author topic t.
	Static_Counter* c_t_DT;		// Count of words assigned to document topic t.

	// Working distributions.
	Distribution* theta_A;		// Author topic distribution.
	Distribution* theta_D;		// Document topic distribution.
	double pi;					// Topic ratio.
	// Averaged distributions.
	Distribution* avg_theta_A;		// Author topic distribution.
	Distribution* avg_theta_D;		// Document topic distribution.
	double avg_pi;					// Topic ratio.

	int num_iter;		// Number of iterations.

	// Random number generator.
	int rng_seed;
//	boost::mt19937 rng;
	std::default_random_engine generator;

	// Vector of probabilities.
	std::vector<double> p;

	// Vector of author probabilities.
	std::vector<double> author_probabilities;

	// Flag for first iteration.
	bool is_first_iteration;

	// Log time consumption.
	double time_elapsed;

	AChain(int N, int* words, MIChain* model_chain);		// Constructor with number of words and word array as parameter.
	virtual ~AChain();

	void init_rng();

	void iterate(int count); 						// Iterate the chain count times.
	void average_iterate(int count);				// Iterate the chain count times and average over the distributions.

	void sample(int i);						// Sample from the probability distribution for i-th word.
	void calc_prob(int w, int y_old, int t_old);

	int draw();
	void update_counts(int w, int y_old, int y_new, int t_old, int t_new);		// Update counts.

	void calc_distributions();	// Calculate the distributions from the counters.

	void calc_author_probabilities();	// Calculate the probability that the test text belongs to a given author for all authors.

	int arg_max(std::vector<double> v);	// Return the argument of the maximum in v.

	int get_most_probable_author(int num_burn_in, int num_average);		// Return the most probable author of the text.
};

#endif /* ACHAIN_H_ */
