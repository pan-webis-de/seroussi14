/*
 * MIChain.cpp
 *
 *  Created on: Sep 16, 2015
 *      Author: thomas
 */

#include "MIChain.h"

MIChain::MIChain(std::string h5filename, int id) {
	// Constructor

	// Store the filename.
	this->h5filename = h5filename;
	// Store ID.
	this->chain_id = id;

	// Open the h5 filename.
	hid_t file_id;
	file_id = H5Fopen(this->h5filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

	// Read number of authors.
	H5LTread_dataset_int(file_id,"/corpus/A",&(this->A));
	// Read number of documents.
	H5LTread_dataset_int(file_id,"/corpus/D",&(this->D));
	// Read number of words in vocabulary.
	H5LTread_dataset_int(file_id,"/corpus/V",&(this->V));
	// Read word count vector.
	this->N = new int[this->D];
	H5LTread_dataset_int(file_id,"/corpus/N",this->N);

	// Calculate total number of words.
	this->N_words = 0;
	for (int i=0; i<this->D; i++) this->N_words +=this->N[i];


	this->T_A = 90;					// Number of author topics.
	this->T_D = 10;					// Number of document topics.

	// Read author vector.
	this->authors = new int[this->D];
	H5LTread_dataset_int(file_id,"/corpus/authors",this->authors);

	// Initialize per document arrays.
	this->words = new int*[this->D];		// Vector of word vectors.
	this->topics = new int*[this->D]; 		// Vector of topic vectors.
	this->indicators = new int*[this->D];	// Vector of indicator vectors.

	// Initialize on document level.
	for (int i=0; i<this->D; i++) {
		// Copy words.
		this->words[i] = new int[this->N[i]];
		std::string dataset_string = std::string("/corpus/documents/") + std::to_string(i);
		H5LTread_dataset_int(file_id,dataset_string.c_str(),this->words[i]);
		// Initialize the other arrays to the correct size.
		this->topics[i] = new int[this->N[i]];
		this->indicators[i] = new int[this->N[i]];
		// Initialize array values.
		for (int j=0; j<this->N[i]; j++) {
			this->topics[i][j] = -1;
			this->indicators[i][j] = -1;
		}
	}

	// Calculate author topic prior. (Scalar because prior is assumed to be symmetric).
	this->alpha_A = std::min(0.1,5.0/this->T_A);
	// Calculate document topic prior. (Scalar because prior is assumed to be symmetric).
	this->alpha_D = std::min(0.1,5.0/this->T_D);

	this->delta_A = 4.889;			// Prior for author topic ratio.
	this->delta_D = 1.222;			// Prior for document topic ratio.

	this->eta = 1;					// Author in corpus prior.

	// Calculate word in topic priors using the stopword_indicator vector.
	this->epsilon = 0.009;			// Stopword parameter.
	this->beta = 0.01;
	int* stopword_indicator = new int[this->V];
	H5LTread_dataset_int(file_id,"/corpus/stopword_indicator",stopword_indicator);
	this->beta_A = new double[this->V];
	this->beta_D = new double[this->V];
	this->N_sw = 0;
	for (int i=0; i<this->V; i++) {
		this->N_sw += (int) stopword_indicator[i];
		this->beta_A[i] = this->beta + this->epsilon*stopword_indicator[i];
		this->beta_D[i] = this->beta - this->epsilon*stopword_indicator[i];
	}

	delete[] stopword_indicator;

	// Close hdf5 file.
	H5Fclose(file_id);


	// Variables for sums needed for probability calculation.
	this->sum_alpha_A = this->alpha_A*this->T_A;
	this->sum_alpha_D = this->alpha_D*this->T_D;
	this->sum_beta_A = this->V*0.01 + this->epsilon*this->N_sw;
	this->sum_beta_D = this->V*0.01 - this->epsilon*this->N_sw;


	// Count variables.
	this->c_d_DA = new Static_Counter(this->D);      				// Count of words assigned to author topics in document d.
	this->c_d_DD = new Static_Counter(this->D);					// Count of words assigned to document topics in document d.
	this->c_dt_DT = new Static_Counter*[this->D];	// Count of words assigned to document topic t in document d.
	// Loop over all documents and initialize values.
	for (int i=0; i<this->D; i++) {
		this->c_dt_DT[i] = new Static_Counter(this->T_D) ;
	}
	this->c_at_AT = new Static_Counter*[this->A];	// Count of author topic t assignments to author a.
	this->c_tw_ATV = new Dynamic_Counter*[this->T_A];	// Count of word w in author topic t.
	this->c_tw_DTV = new Dynamic_Counter*[this->T_D];	// Count of word w in document topic t.

	// Initialize the arrays with maps.
	for (int i=0; i<this->A; i++)
		this->c_at_AT[i] = new Static_Counter(this->T_A);

	for (int i=0; i<this->T_A; i++)
		this->c_tw_ATV[i] = new Dynamic_Counter();

	for (int i=0; i<this->T_D; i++)
		this->c_tw_DTV[i] = new Dynamic_Counter();


	this->c_a_AD = new Static_Counter(this->A);					// Count of documents of author a.
	// Calculate the counter values.
	for (int d=0; d<this->D; d++) {
		this->c_a_AD->inc(this->authors[d]);
	}

	// Distributions
	// Author topic distributions.
	this->theta_at_A = new Distribution*[this->A];
	for (int i=0; i<this->A; i++) this->theta_at_A[i] = new Distribution(this->T_A);

	// Document topic distributions.
	this->theta_dt_D = new Distribution*[this->D];
	for (int i=0; i<this->D; i++) this->theta_dt_D[i] = new Distribution(this->T_D);

	// Author topic word distributions.
	this->phi_tw_A = new Distribution*[this->T_A];
	for (int i=0; i<this->T_A; i++) this->phi_tw_A[i] = new Distribution(this->V);

	// Document topic word distributions.
	this->phi_tw_D = new Distribution*[this->T_D];
	for (int i=0; i<this->T_D; i++) this->phi_tw_D[i] = new Distribution(this->V);

	// Topic ratios.
	this->pi_d = new double[this->D];

	// Author distribution.
	this->chi_a = new Distribution(this->T_D);



	// Construct a lookup table for indices and indicator, topic combinations in form of tuples (y,t). Used for sampling.
	this->N_comb = this->T_A + this->T_D;
	this->lookup_table = new std::map<int,std::pair<int,int> >;	// Store combinations as identifier, topic pairs. identifier in form of bool. identifier = False if document topic, true if author topic.
	// Iterate over document topics.
	for (int t=0; t<this->T_D; t++) {
		this->lookup_table->insert( std::pair<int, std::pair<int,int> > (t, std::pair<int,int>(0,t) ));
	}
	for (int t=0; t<this->T_A; t++) {
		this->lookup_table->insert( std::pair<int, std::pair<int,int> > (t+this->T_D, std::pair<int,int>(1,t) ));
	}

	// Initialize iteration counter.
	this->num_iter = 0;

	// Seed the random number generator.
	this->init_rng();
	// Initialize probability vector.
	for (int i=0; i<this->N_comb; i++) this->p.push_back(1);

	// Set flag for first iteration.
	this->is_first_iteration = true;
	// Set flag for sample_save.
	this->is_chain_group_created = false;

	// Run first iteration to initialize topics and indicators.
	this->iterate(1);
	this->num_iter = 0;
	this->time_elapsed = 0;
	this->is_first_iteration = false;


}

MIChain::~MIChain() {
//	// Delete all arrays on document basis.
//
//	for (int i=0; i<this->A; i++)
//		delete this->c_at_AT[i];
//
//	for (int i=0; i<this->T_A; i++)
//		delete this->c_tw_ATV[i];
//
//	for (int i=0; i<this->T_D; i++)
//		delete this->c_tw_DTV[i];
//
//	for (int i=1; i<this->D; i++) {
//		delete this->c_dt_DT[i];
//		delete[] this->words[i];
//		delete[] this->topics[i];
//		delete[] this->indicators[i];
//	}
//
//	delete[] this->c_dt_DT;
//	//	Delete all arrays on corpus level.
//	delete[] this->words;
//	delete[] this->topics;
//	delete[] this->indicators;
//	delete[] this->N;
//
//	delete this->c_a_AD;
//	delete this->c_d_DA;
//	delete this->c_d_DD;
//
//	delete[] this->c_at_AT;
//	delete[] this->c_tw_ATV;
//	delete[] this->c_tw_DTV;
//
//	delete[] this->beta_A;
//	delete[] this->beta_D;
//
//	delete this->lookup_table;
//
//	delete[] this->theta_at_A;
//	delete[] this->theta_dt_D;
//	delete[] this->phi_tw_A;
//	delete[] this->phi_tw_D;
//	delete[] this->pi_d;
//	delete[] this->chi_a;
}

void MIChain::init_rng() {
	// Seed the boost random number generator engine.
	// Use current time as seed.
	clock_t seed = clock();
	this->rng_seed = seed;
//	this->rng.seed(this->rng_seed);		// Boost rng
	this->generator.seed(this->rng_seed);	// Standard rng
}


void MIChain::sample(int d, int ind){
	// Sample from the probability distribution for i-th word in d-th document.

	// Get the word.
	int w = this->words[d][ind];
	// Get the author of the document.
	int a = this->authors[d];
	// Get the topic indicator.
	int y_old = this->indicators[d][ind];
	// Get the topic.
	int t_old = this->topics[d][ind];

	// Calculate the conditional distribution
	this->calc_prob(w, a ,y_old, t_old, d, ind);

	// Draw from the distribution.
	int r = this->draw();

	// Lookup the new variables.
	int y_new = this->lookup_indicator(r);
	int t_new = this->lookup_topic(r);

	// Update counts.
	this->update_counts(w, d, a, y_old, y_new, t_old, t_new);

	// Update the variables.
	this->indicators[d][ind] = y_new;
	this->topics[d][ind] = t_new;
}

void MIChain::calc_prob(int w, int a, int y_old, int t_old, int d, int ind){
	// Calculate the probability according to Equation 11, Seroussi 2014, Authorship Attribution with Topic Models.

	double b, n1, n2, d1, d2;
	int cnt = 0;

	for (int t=0; t<this->T_D; t++) {		// Loop over document topics.
		// Probability for document topic y = 0.
		// First calculate all the factors needed.
		// b is the term in parentheses.
		b = this->delta_D + this->c_d_DD->at(t);
		// n1 is the first numerator.
		n1 = this->alpha_D + this->c_dt_DT[d]->at(t);
		// n2 is the second numerator.
		n2 = this->beta_D[w] + this->c_tw_DTV[t]->at(w);
		// d1 is the first denominator.
		d1 = this->sum_alpha_D + this->c_dt_DT[d]->sum();
		if (y_old == 0) d1--;	// All counts exclude current index.
		// d2 is the second denominator.
		d2 = this->sum_beta_D + this->c_tw_DTV[t]->sum();
		if (y_old == 0 and t_old == t) d2--;
		// Calculate the probability.
		this->p[cnt++] = b*n1*n2/d1/d2;
	}
	for (int t=0; t<this->T_A; t++) {		// Loop over author topics.
		// Probability for author topic y = 1.
		// First calculate all the factors needed.
		// b is the term in parentheses.
		b = this->delta_A + this->c_d_DA->at(d);
		// n1 is the first numerator.
		n1 = this->alpha_A + this->c_at_AT[a]->at(t);
		// n2 is the second numerator.
		n2 = this->beta_A[w] + this->c_tw_ATV[t]->at(w);
		// d1 is the first denominator.
		d1 = this->sum_alpha_A + this->c_at_AT[a]->sum();
		if (y_old == 1) d1--;	// All counts exclude current index.
		// d2 is the second denominator.
		d2 = this->sum_beta_A + this->c_tw_ATV[t]->sum();
		if (y_old == 1 and t_old == t) d2--;
		// Calculate the probability.
		this->p[cnt++] = b*n1*n2/d1/d2;
	}

	// Normalize the probability.
	double sum = 0;
	for (int i=0; i<this->N_comb; i++) sum += this->p[i];
	for (int i=0; i<this->N_comb; i++) this->p[i] = this->p[i]/sum;
}

//int MIChain::draw_boost() {
	// Draw from the discrete distribution given by weights p.
//	boost::random::discrete_distribution<> dist(p.begin(), p.end());
//	return dist(this->rng);
//}

int MIChain::draw() {
	// Draw from the discrete distribution given by weights p.
	std::discrete_distribution<int> dist (p.begin(), p.end());
	return dist(this->generator);
}

void MIChain::update_counts(int w, int d, int a, int y_old, int y_new, int t_old, int t_new){
	// Update counts.
	// Consider the first iteration.
	if (!this->is_first_iteration) {
		// Lower the counts according to the old variables.
		if (y_old==0) {		// Belonged to document topic.
			this->c_d_DD->dec(d);
			this->c_dt_DT[d]->dec(t_old);
			this->c_tw_DTV[t_old]->dec(w);
		} else {			// Belonged to author topic.
			this->c_d_DA->dec(d);
			this->c_at_AT[a]->dec(t_old);
			this->c_tw_ATV[t_old]->dec(w);
		}
	}
	// Increment to counts according to the new variables.
	if (y_new == 0) {		// Belongs to a document topic.
		this->c_d_DD->inc(d);
		this->c_dt_DT[d]->inc(t_new);
		this->c_tw_DTV[t_new]->inc(w);
	} else {				// Belongs to an author topic.
		this->c_d_DA->inc(d);
		this->c_at_AT[a]->inc(t_new);
		this->c_tw_ATV[t_new]->inc(w);
	}
}

void MIChain::iterate(int count) {
	// Iterate count time over all documents and words.
//	boost::timer t;
	for (int i=0; i < count; i++) {
		// Update iteration counter.
		this->num_iter += 1;
		if (this->num_iter % 10 == 0) {
			std::cout << "Iteration " << this->num_iter << std::endl;
		}
		if (this->num_iter % 20 == 0) this->check_sums();
		for (int d=0; d < this->D; d++) {
//			std::cout << "Processing document " << d <<"." << std::endl;
			for (int ind=0; ind < this->N[d]; ind++) {
				this->sample(d,ind);
			}
		}
	}
//	double time = t.elapsed();
//	this->time_elapsed += time;
//	std::cout << "MIChain ID-" << this->chain_id << " : Ran " << count << " iterations in " << time << "sec. Average: " << time/count << " sec/iteration."  << std::endl;
}

int MIChain::lookup_indicator(int ind) {
	// Return the topic indicator that belongs to the ind-th index in the probability vector.
//	return this->lookup_table->at(ind).first;
	int y;
	if (ind < this->T_D) {
		y = 0;
	} else {
		y = 1;
	}
	return y;
}
int MIChain::lookup_topic(int ind) {
	// Return the topic that belongs to the ind-th index in the probability vector.
	int t;
	if (ind < this->T_D) {
		t = ind;
	} else {
		t= ind - this->T_D;
	}
	return t;
//	return this->lookup_table->at(ind).second;
}


void MIChain::save_sample() {
	// Save counters, words, indicators and topics.

	bool is_verbose = true;
	std::cout << "Saving sample to file." << std::endl;
	std::cout << "Open file." << std::endl;
	// Open the h5 filename.
	hid_t file_id;
	file_id = H5Fopen(this->h5filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);


	// Create group for chain.
	std::string chain_group_name = std::string("chain_") + std::to_string(this->chain_id);
	hid_t chain_group_id;
	if (this->is_chain_group_created) {
		std::cout << "Open existing chain group." << std::endl;
		chain_group_id = H5Gopen(file_id, chain_group_name.c_str(), H5P_DEFAULT);
	}
	// If Group doesn't exist, create it.
	else {
		std::cout << "Create new chain group." << std::endl;
		chain_group_id = H5Gcreate(file_id, chain_group_name.c_str() , H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
		this->is_chain_group_created = true;
	}

	std::cout << "Create group for sample." << std::endl;
	// Create group for sample.
	std::string iteration_group_name = std::string("iteration_") + std::to_string(this->num_iter);
	hid_t iteration_group_id = H5Gcreate(chain_group_id, iteration_group_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	std::cout << "Store meta-data." << std::endl;
	// Store number of iterations as attribute.
	int iteration_buffer[1] = {this->num_iter};
	H5LTset_attribute_int(chain_group_id, iteration_group_name.c_str(), "iteration", iteration_buffer, 1);
	// Store time as attribute.
	double time_buffer[1] = {this->time_elapsed};
	H5LTset_attribute_double(chain_group_id, iteration_group_name.c_str(), "time", time_buffer, 1);
	// Store time as attribute.
	double seed_buffer[1] = {this->rng_seed};
	H5LTset_attribute_double(chain_group_id, iteration_group_name.c_str(), "rng seed", seed_buffer, 1);

	std::cout << "Store counter." << std::endl;
	// Store counter c_d_DA.
	this->c_d_DA->save(iteration_group_id, std::string("c_d_DA"));
	// Store counter c_d_DD.
	this->c_d_DD->save(iteration_group_id, std::string("c_d_DD"));

	// Store counters in c_dt_DT.
	hid_t c_dt_DT_group_id = H5Gcreate(iteration_group_id, "c_dt_DT", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->D; i++) {
		this->c_dt_DT[i]->save(c_dt_DT_group_id, std::to_string(i).c_str());
	}
	H5Gclose(c_dt_DT_group_id);

	// Store counters in c_at_AT.
	hid_t c_at_AT_group_id = H5Gcreate(iteration_group_id, "c_at_AT", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->A; i++) {
		this->c_at_AT[i]->save(c_at_AT_group_id, std::to_string(i).c_str());
	}
	H5Gclose(c_at_AT_group_id);

	// Store counters in c_tw_ATV.
	hid_t c_tw_ATV_group_id = H5Gcreate(iteration_group_id, "c_tw_ATV", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->T_A; i++) {
		this->c_tw_ATV[i]->save(c_tw_ATV_group_id, std::to_string(i).c_str());
	}
	H5Gclose(c_tw_ATV_group_id);

	// Store counters in c_tw_DTV.
	hid_t c_tw_DTV_group_id = H5Gcreate(iteration_group_id, "c_tw_DTV", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->T_D; i++) {
		this->c_tw_DTV[i]->save(c_tw_ATV_group_id, std::to_string(i).c_str());
	}
	H5Gclose(c_tw_DTV_group_id);

	std::cout << "Store indicator." << std::endl;
	// Store indicators.
	hid_t indicators_group_id = H5Gcreate(iteration_group_id, "indicators", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->D; i++) {
		// Create array with dimensions of data.
		hsize_t dims[1] = {this->N[i]};
		// Data buffer
		H5LTmake_dataset(indicators_group_id, std::to_string(i).c_str(), 1, dims, H5T_NATIVE_INT, this->indicators[i]);
	}
	H5Gclose(indicators_group_id);

	std::cout << "Store topics." << std::endl;
	// Store topics.
	hid_t topics_group_id = H5Gcreate(iteration_group_id, "topics", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->D; i++) {
		// Create array with dimensions of data.
		hsize_t dims[1] = {this->N[i]};
		// Data buffer
		H5LTmake_dataset(topics_group_id, std::to_string(i).c_str(), 1, dims, H5T_NATIVE_INT, this->topics[i]);
	}
	H5Gclose(topics_group_id);

	std::cout << "Store distributions." << std::endl;
	// Store distributions.

	hid_t distributions_group_id = H5Gcreate(iteration_group_id, "distributions", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	// Save author topic distributions.
	hid_t author_topic_group_id = H5Gcreate(distributions_group_id, "author topics", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->A; i++) {
		// Store the distribution with index as name.
		this->theta_at_A[i]->save_hdf5(author_topic_group_id, std::to_string(i));
	}
	H5Gclose(author_topic_group_id);


	// Save author topic distributions.
	hid_t document_topic_group_id = H5Gcreate(distributions_group_id, "document topics", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->D; i++) {
		// Store the distribution with index as name.
		this->theta_dt_D[i]->save_hdf5(document_topic_group_id, std::to_string(i));
	}
	H5Gclose(document_topic_group_id);

	// Save author topic word distributions.
	hid_t author_topic_word_group_id = H5Gcreate(distributions_group_id, "author topic words", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->T_A; i++) {
		// Store the distribution with index as name.
		this->phi_tw_A[i]->save_hdf5(document_topic_group_id, std::to_string(i));
	}
	H5Gclose(author_topic_word_group_id);

	// Save author topic word distributions.
	hid_t document_topic_word_group_id = H5Gcreate(distributions_group_id, "document topic words", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	for (int i=0; i<this->T_D; i++) {
		// Store the distribution with index as name.
		this->phi_tw_D[i]->save_hdf5(document_topic_group_id, std::to_string(i));
	}
	H5Gclose(document_topic_word_group_id);

	// Store author distribution.
	this->chi_a->save_hdf5(distributions_group_id, "authors");

	// Store topic ratios.
	// Set data dimensionality.
	int rank = 1;
	// Create array with dimensions of data.
	hsize_t dims[1] = {this->D};
	//	H5LTmake_dataset(file_id, name.c_str(), rank, dims, H5T_NATIVE_INT, buffer);
	H5LTmake_dataset(distributions_group_id, "topic ratio", rank, dims, H5T_NATIVE_DOUBLE, this->pi_d);


	H5Gclose(distributions_group_id);



	H5Gclose(iteration_group_id);
	H5Gclose(chain_group_id);
	H5Fclose(file_id);
}

void MIChain::load_sample(int chain_id, int iteration) {
	// Load counters, words, indicators and topics.

	// Open the h5 filename.
	hid_t file_id;
	file_id = H5Fopen(this->h5filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

	// Open group for chain.
	std::string chain_group_name = std::string("chain_") + std::to_string(chain_id);
	hid_t chain_group_id = H5Gopen(file_id, chain_group_name.c_str(), H5P_DEFAULT);
	if (chain_group_id < 0) {
		std::cerr << "No chain with id " << chain_id << " present in file " << this->h5filename << std::endl;
		return;
	}
	this->chain_id = chain_id;
	this->is_chain_group_created = true;

	// Open group for sample.
	std::string iteration_group_name = std::string("iteration_") + std::to_string(iteration);
	hid_t iteration_group_id = H5Gopen(chain_group_id, iteration_group_name.c_str(), H5P_DEFAULT);
	if (iteration_group_id < 0) {
		std::cerr << "No iteration with number " << iteration << " present in chain with id " << chain_id << " in file " << this->h5filename << std::endl;
		return;
	}
	this->num_iter = iteration;

	// Load time as attribute.
	double time_buffer[1];
	H5LTget_attribute_double(chain_group_id, iteration_group_name.c_str(), "time", time_buffer);
	this->time_elapsed = time_buffer[0];

	// Load counter c_d_DA.
	this->c_d_DA->load(iteration_group_id, std::string("c_d_DA"));
	// Load counter c_d_DD.
	this->c_d_DD->load(iteration_group_id, std::string("c_d_DD"));

	// Load counters in c_dt_DT.
	hid_t c_dt_DT_group_id = H5Gopen(iteration_group_id, "c_dt_DT", H5P_DEFAULT);
	for (int i=0; i<this->D; i++) {
		this->c_dt_DT[i]->load(c_dt_DT_group_id, std::to_string(i).c_str());
	}
	H5Gclose(c_dt_DT_group_id);

	// Load counters in c_at_AT.
	hid_t c_at_AT_group_id = H5Gopen(iteration_group_id, "c_at_AT", H5P_DEFAULT);
	for (int i=0; i<this->A; i++) {
		this->c_at_AT[i]->load(c_at_AT_group_id, std::to_string(i).c_str());
	}
	H5Gclose(c_at_AT_group_id);

	// Load counters in c_tw_ATV.
	hid_t c_tw_ATV_group_id = H5Gopen(iteration_group_id, "c_tw_ATV", H5P_DEFAULT);
	for (int i=0; i<this->T_A; i++) {
		this->c_tw_ATV[i]->load(c_tw_ATV_group_id, std::to_string(i).c_str());
	}
	H5Gclose(c_tw_ATV_group_id);

	// Load counters in c_tw_DTV.
	hid_t c_tw_DTV_group_id = H5Gopen(iteration_group_id, "c_tw_DTV", H5P_DEFAULT);
	for (int i=0; i<this->T_D; i++) {
		this->c_tw_DTV[i]->load(c_tw_ATV_group_id, std::to_string(i).c_str());
	}
	H5Gclose(c_tw_DTV_group_id);

	// Load indicators.
	hid_t indicators_group_id = H5Gopen(iteration_group_id, "indicators", H5P_DEFAULT);
	for (int i=0; i<this->D; i++) {
		H5LTread_dataset_int(indicators_group_id, std::to_string(i).c_str(), this->indicators[i]);
	}
	H5Gclose(indicators_group_id);

	// Load topics.
	hid_t topics_group_id = H5Gopen(iteration_group_id, "topics", H5P_DEFAULT);
	for (int i=0; i<this->D; i++) {
		H5LTread_dataset_int(topics_group_id, std::to_string(i).c_str(), this->topics[i]);
	}
	H5Gclose(topics_group_id);

	H5Gclose(iteration_group_id);
	H5Gclose(chain_group_id);
	H5Fclose(file_id);

}


bool MIChain::check_sums() {
	// Check sums for consistency.
	bool result = true;

	// Number of all words assigned to document and author topics must be equal to total number of words.
	result = result and (this->c_d_DA->sum() + this->c_d_DD->sum() == this->N_words);

	// Number of assignments of a word to document and author topics, summed over all words must be equal to total number of words.
	int sum_c_tw_DTV = 0;
	int sum_c_tw_ATV = 0;
	for (int i=0; i<this->T_A; i++) sum_c_tw_ATV += this->c_tw_ATV[i]->sum();
	for (int i=0; i<this->T_D; i++) sum_c_tw_ATV += this->c_tw_DTV[i]->sum();
	result = result and (sum_c_tw_DTV + sum_c_tw_ATV == this->N_words);

	// Number of assignments to document topics in all documents and number of author topic assignments for all authors must be equal to number of all words.
	int sum_c_at_AT = 0;
	int sum_c_dt_DT = 0;
	for (int i=0; i<this->A; i++) sum_c_at_AT += this->c_at_AT[i]->sum();
	for (int i=0; i<this->D; i++) sum_c_dt_DT += this->c_dt_DT[i]->sum();
	result = result and (sum_c_at_AT + sum_c_dt_DT == this->N_words);

	if (!result) std::cerr << "Sum check is not consistent!" << std::endl;

	return result;
}

void MIChain::calc_distributions() {
	// Calculate the distributions according to Seroussi14.

	// Author topic distributions.
	for (int a=0; a<this->A; a++) {		// For every author...
		for (int t=0; t<this->T_A; t++) {	// For every author topic...
			double p = ( this->alpha_A + this->c_at_AT[a]->at(t) )/( this->sum_alpha_A + this->c_at_AT[a]->sum() );
			this->theta_at_A[a]->set(t,p);
			this->theta_at_A[a]->set_zcp(  this->alpha_A /( this->sum_alpha_A + this->c_at_AT[a]->sum() ) );
		}
	}

	// Document topic distributions.
	for (int d=0; d<this->D; d++) {		// For every document...
		for (int t=0; t<this->T_D; t++) {	// For every document topic...
			double p = ( this->alpha_D + this->c_dt_DT[d]->at(t) )/( this->sum_alpha_D + this->c_dt_DT[d]->sum() );
			this->theta_dt_D[d]->set(t,p);
			this->theta_dt_D[d]->set_zcp( this->alpha_D /( this->sum_alpha_D + this->c_dt_DT[d]->sum() ) );
		}
	}

	// Author topic word distributions.
	for (int t=0; t<this->T_A; t++) {		// For every author topic...
		for (int w=0; w<this->V; w++) {	// For every word...
			double p = ( this->beta_A[w] + this->c_tw_ATV[t]->at(w) ) / ( this->sum_beta_A + this->c_tw_ATV[t]->sum() );
			this->phi_tw_A[t]->set(w,p);
		}
		this->phi_tw_A[t]->set_zcp(  this->beta / ( this->sum_beta_A + this->c_tw_ATV[t]->sum() ) );
		this->phi_tw_A[t]->set_zcp_sw(  this->beta + this->epsilon / ( this->sum_beta_A + this->c_tw_ATV[t]->sum() ) );
	}

	// Document topic word distributions.
	for (int t=0; t<this->T_D; t++) {		// For every document topic...
		for (int w=0; w<this->V; w++) {	// For every word...
			double p = ( this->beta_D[w] + this->c_tw_DTV[t]->at(w) ) / ( this->sum_beta_D + this->c_tw_DTV[t]->sum() );
			this->phi_tw_D[t]->set(w,p);
		}
		this->phi_tw_D[t]->set_zcp(  this->beta / ( this->sum_beta_D + this->c_tw_DTV[t]->sum() ) );
		this->phi_tw_D[t]->set_zcp_sw(  this->beta - this->epsilon / ( this->sum_beta_D + this->c_tw_DTV[t]->sum() ) );
	}


	// Topic ratios.
	for (int d=0; d<this->D; d++) {
		this->pi_d[d] = ( this->delta_A + this->c_d_DA->at(d) )/( this->delta_D + this->delta_A + this->N[d] );
	}

	// Author distribution.
	for (int a=0; a<this->A; a++) {
		double p = (this->eta + this->c_a_AD->at(a)) / ( this->eta*this->A + this->c_a_AD->sum() );
		this->chi_a->set(a,p);
	}
	this->chi_a->set_zcp( this->eta / ( this->eta*this->A + this->c_a_AD->sum() ));

}
