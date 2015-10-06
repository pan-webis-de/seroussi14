/*
 * AChain.cpp
 *
 *  Created on: Sep 25, 2015
 *      Author: thomas
 */

#include "AChain.h"

AChain::AChain(int N, int* words, MIChain* model_chain) {
	// Constructor with number of words and word array as parameter.
	this->mc = model_chain;
	this->N = N;							// Number of words in document.
	this->words = new int[this->N];			// Array of words labeled according to the vocabulary of the corpus. Assign negative integers for previously unknown words.
	this->indicators = new int[this->N];	// Topic indicators.
	this->topics = new int[this->N];		// Array of topics.

	// Initialize indicators and topics.
	for (int i=0; i<this->N; i++) {
		this->indicators[i] = -1;
		this->topics[i] = -1;
	}

	// Copy words.
	for (int i=0; i<this->N; i++) this->words[i] = words[i];

	this->c_DA = new Static_Counter(1);			// Count of words assigned to author topics.
	this->c_DD = new Static_Counter(1);			// Count of words assigned to document topics.

	this->c_t_AT = new Static_Counter(this->mc->T_A);		// Count of words assigned to author topic t.
	this->c_t_DT = new Static_Counter(this->mc->T_D);		// Count of words assigned to document topic t.

	// Initialize working distributions.
	this->theta_A = new Distribution(this->mc->T_A);		// Author topic distribution.
	this->theta_D = new Distribution(this->mc->T_D);		// Document topic distribution.
	this->pi = 0;					// Topic ratio.

	// Initialize averaged distributions.
	this->avg_theta_A = new Distribution(this->mc->T_A);		// Author topic distribution.
	this->avg_theta_D = new Distribution(this->mc->T_D);		// Document topic distribution.
	this->avg_pi = 0;					// Topic ratio.


	// Number of iterations.
	this->num_iter = 0;

	// Seed the random number generator.
	this->init_rng();
	// Initialize probability vector.
	for (int i=0; i<this->mc->N_comb; i++) this->p.push_back(1);

	// Flag for first iteration.
	this->is_first_iteration = true;

	// Log time consumption.
	this->time_elapsed = 0;

	// Run initial iteration.
	this->iterate(1);
	this->is_first_iteration = false;
	this->num_iter = 0;
	this->time_elapsed = 0;

}

AChain::~AChain() {
	// Auto-generated destructor stub
	delete[] this->words;
	delete[] this->indicators;
	delete[] this->topics;
	delete this->c_DA;
	delete this->c_DD;
	delete this->c_t_AT;
	delete this->c_t_DT;
	delete this->theta_A;
	delete this->theta_D;
}

void AChain::init_rng() {
	// Seed the boost random number generator engine.
	// Use current time as seed.
	clock_t seed = clock();
	this->rng_seed = seed;
//	this->rng.seed(this->rng_seed);		// Boost rng
	this->generator.seed(this->rng_seed);	// Standard rng
}

void AChain::calc_prob(int w, int y_old, int t_old){
	// Calculate the probability according to Equation 14, Seroussi 2014, Authorship Attribution with Topic Models.

	double b, n, d;
	int cnt = 0;

	for (int t=0; t<this->mc->T_D; t++) {		// Loop over document topics.
		// Probability for document topic y = 0.
		// First calculate all the factors needed.
		// b is the term in parentheses.
		b = this->mc->delta_D + this->c_DD->at(0);
		// n is the numerator.
		n = this->mc->alpha_D + this->c_t_DT->at(t);

		// d is the denominator.
		d = this->mc->sum_alpha_D + this->c_t_DT->sum();
		if (y_old == 0) d--;	// All counts exclude current index.

		// Calculate the probability.
		this->p[cnt++] = b*n/d*this->mc->phi_tw_D[t]->at(w);
	}
	for (int t=0; t<this->mc->T_A; t++) {		// Loop over author topics.
		// Probability for author topic y = 1.
		// First calculate all the factors needed.
		// b is the term in parentheses.
		b = this->mc->delta_A + this->c_DA->at(0);
		// n is the numerator.
		n = this->mc->alpha_A + this->c_t_AT->at(t);

		// d is the denominator.
		d = this->mc->sum_alpha_A + this->c_t_AT->sum();
		if (y_old == 1) d--;	// All counts exclude current index.

		// Calculate the probability.
		this->p[cnt++] = b*n/d*this->mc->phi_tw_A[t]->at(w);
	}

	// Normalize the probability.
	double sum = 0;
	for (int i=0; i<this->mc->N_comb; i++) sum += this->p[i];
	for (int i=0; i<this->mc->N_comb; i++) this->p[i] = this->p[i]/sum;
}


void AChain::sample(int ind){
	// Sample from the probability distribution for i-th word in d-th document.

	// Get the word.
	int w = this->words[ind];

	// Get the topic indicator.
	int y_old = this->indicators[ind];
	// Get the topic.
	int t_old = this->topics[ind];

	// Calculate the conditional distribution
	this->calc_prob(w, y_old, t_old);

	// Draw from the distribution.
	int r = this->draw();

	// Lookup the new variables.
	int y_new = this->mc->lookup_indicator(r);
	int t_new = this->mc->lookup_topic(r);

	// Update counts.
	this->update_counts(w, y_old, y_new, t_old, t_new);

	// Update the variables.
	this->indicators[ind] = y_new;
	this->topics[ind] = t_new;
}

int AChain::draw() {
	// Draw from the discrete distribution given by weights p.
	std::discrete_distribution<int> dist (p.begin(), p.end());
	return dist(this->generator);
}

void AChain::update_counts(int w, int y_old, int y_new, int t_old, int t_new){
	// Update counts.
	// Consider the first iteration.
	if (!this->is_first_iteration) {
		// Lower the counts according to the old variables.
		if (y_old==0) {		// Belonged to document topic.
			this->c_DD->dec(0);
			this->c_t_DT->dec(t_old);
		} else {			// Belonged to author topic.
			this->c_DA->dec(0);
			this->c_t_AT->dec(t_old);
		}
	}
	// Increment to counts according to the new variables.
	if (y_new == 0) {		// Belongs to a document topic.
		this->c_DD->inc(0);
		this->c_t_DT->inc(t_new);
	} else {				// Belongs to an author topic.
		this->c_DA->inc(0);
		this->c_t_AT->inc(t_new);
	}
}

void AChain::iterate(int count) {
	// Iterate count time over all documents and words.
//	boost::timer t;
	for (int i=0; i < count; i++) {
		// Update iteration counter.
		this->num_iter += 1;
//		if (this->num_iter % 10 == 0) {
//			std::cout << "Iteration " << this->num_iter << std::endl;
//		}
		for (int ind=0; ind < this->N; ind++) {
			this->sample(ind);
		}

	}
//	double time = t.elapsed();
//	this->time_elapsed += time;
//	std::cout << "AChain with MIChain ID-" << this->mc->chain_id << " : Ran " << count << " iterations in " << time << "sec. Average: " << time/count << " sec/iteration."  << std::endl;
}


void AChain::calc_distributions() {
	// Calculate the distributions according to Seroussi14.

	// Author topic distributions.

	for (int t=0; t<this->mc->T_A; t++) {	// For every author topic...
		double p = ( this->mc->alpha_A + this->c_t_AT->at(t) )/( this->mc->sum_alpha_A + this->c_t_AT->sum() );
		this->theta_A->set(t,p);
	}


	// Document topic distributions.
	for (int t=0; t<this->mc->T_D; t++) {	// For every document topic...
		double p = ( this->mc->alpha_D + this->c_t_DT->at(t) )/( this->mc->sum_alpha_D + this->c_t_DT->sum() );
		this->theta_D->set(t,p);
	}

	// Topic ratios.
	this->pi = ( this->mc->delta_A + this->c_DA->at(0) )/( this->mc->delta_D + this->mc->delta_A + this->N );

}


void AChain::average_iterate(int count) {
	// Iterate the chain count times and average over the distributions.
//	boost::timer t;
	for (int i=0; i < count; i++) {
		// Update iteration counter.
		this->num_iter += 1;
//		if (this->num_iter % 10 == 0) {
//			std::cout << "Iteration " << this->num_iter << std::endl;
//		}
		for (int ind=0; ind < this->N; ind++) {
			this->sample(ind);
		}
		// Calculate the distributions.
		this->calc_distributions();
		// Add the values of the new distribution to the average distributions.
		for (int i=0; i<this->mc->T_A; i++) this->avg_theta_A->set( i, this->avg_theta_A->at(i) + this->theta_A->at(i)  );
		for (int i=0; i<this->mc->T_D; i++) this->avg_theta_D->set( i, this->avg_theta_D->at(i) + this->theta_D->at(i)  );
		this->avg_pi += this->pi;
	}
	// Normalize averaged distribution.
	for (int i=0; i<this->mc->T_A; i++) this->avg_theta_A->set( i, this->avg_theta_A->at(i)/count  );
	for (int i=0; i<this->mc->T_D; i++) this->avg_theta_D->set( i, this->avg_theta_D->at(i)/count  );
	this->avg_pi *= 1.0/count;

//	double time = t.elapsed();
//	this->time_elapsed += time;
//	std::cout << "MIChain ID-" << this->chain_id << " : Ran " << count << " iterations in " << time << "sec. Average: " << time/count << " sec/iteration."  << std::endl;
}

void AChain::calc_author_probabilities() {
	// Calculate the probability that the test text belongs to a given author for all authors.
	// Equation 17 from Seroussi 14.
	for (int a=0; a<this->mc->A; a++) {		// For every author...
		// Probability in log form.
		double p = log(this->mc->chi_a->at(a));
		for (int w=0; w<this->N; w++) {		// For every word...
			double sum_A = 0;
			double sum_D = 0;
			for (int t=0; t<this->mc->T_A; t++) {	// For all author topics...
				sum_A += this->mc->theta_at_A[a]->at(t)*this->mc->phi_tw_A[t]->at(w);
//				std::cout << "x = " << x << ", y = " << y << ", sum_A = " << sum_A << std::endl;
			}
			for (int t=0; t<this->mc->T_D; t++) {	// For all document topics...
				sum_D += this->theta_D->at(t)*this->mc->phi_tw_D[t]->at(w);
			}
			p += log(this->pi*sum_A + (1-this->pi)*sum_D);
//			std::cout << "sum_A = " << sum_A << ", sum_D = " << sum_D << ", p = " << p << std::endl;
		}
		this->author_probabilities.push_back(p);
	}
}

int AChain::arg_max(std::vector<double> v) {
	// Return the argument of the maximum in v.
	double max = v[0];
	int arg_max = 0;
	for (int i=0; i<v.size(); i++) {
		if (max < v[i]) {
			max = v[i];
			arg_max = i;
		}
	}
	return arg_max;
}


int AChain::get_most_probable_author(int num_burn_in, int num_average) {
	// Return the most probable author of the text.
	// Burn in iterations.
	this->iterate(num_burn_in);
	// Iterations for averaging.
	this->average_iterate(num_average);
	this->calc_author_probabilities();
//	for (int i=0; i<this->mc->A; i++) {
//		std::cout << "Author " << i << " with p = " << this->author_probabilities[i] << std::endl;
//	}
	return arg_max(this->author_probabilities);
}
