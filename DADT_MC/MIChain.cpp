/*
 * MIChain.cpp
 *
 *  Created on: Sep 16, 2015
 *      Author: thomas
 */

#include "MIChain.h"

MIChain::MIChain(std::string h5filename) {
	// Constructor

	// Open the h5 filename.
	hid_t file_id;

	file_id = H5Fopen(h5filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

	// Read number of authors.
	H5LTread_dataset_int(file_id,"/corpus/A",&(this->A));
	// Read number of documents.
	H5LTread_dataset_int(file_id,"/corpus/D",&(this->D));
	// Read number of words in vocabulary.
	H5LTread_dataset_int(file_id,"/corpus/V",&(this->V));
	// Read word count vector.
	this->N = new int[this->D];
	H5LTread_dataset_int(file_id,"/corpus/N",this->N);


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
	int* stopword_indicator = new int[this->V];
	H5LTread_dataset_int(file_id,"/corpus/stopword_indicator",stopword_indicator);
	this->beta_A = new double[this->V];
	this->beta_D = new double[this->V];
	this->N_sw = 0;
	for (int i=0; i<this->V; i++) {
		this->N_sw += (int) stopword_indicator[i];
		this->beta_A[i] = 0.01 + this->epsilon*stopword_indicator[i];
		this->beta_D[i] = 0.01 - this->epsilon*stopword_indicator[i];
	}

	delete[] stopword_indicator;

	// Variables for sums needed for probability calculation.
	this->sum_alpha_A = this->alpha_A*this->T_A;
	this->sum_alpha_D = this->alpha_D*this->T_D;
	this->sum_beta_A = this->V*0.01 + this->epsilon*this->N_sw;
	this->sum_beta_D = this->V*0.01 - this->epsilon*this->N_sw;


	// Count variables.
	this->c_d_DA = new int[this->D];      				// Count of words assigned to author topics in document d.
	this->c_d_DD = new int[this->D];					// Count of words assigned to document topics in document d.
	this->c_dt_DT = new std::map<int,int>*[this->D];	// Count of words assigned to document topic t in document d.
	// Loop over all documents and initialize values.
	for (int i =0; i<this->D; i++) {
		this->c_d_DA[i] = 0;
		this->c_d_DD[i] = 0;
		this->c_dt_DT[i] = new std::map<int,int>;
	}
	this->c_at_AT = new std::map<int,int>*[this->A];	// Count of author topic t assignments to author a.
	this->c_tw_ATV = new std::map<int,int>*[this->T_A];	// Count of word w in author topic t.
	this->c_tw_DTV = new std::map<int,int>*[this->T_D];	// Count of word w in document topic t.

	// Initialize the arrays with maps.
	for (int i=0; i<this->A; i++)
		this->c_at_AT[i] = new std::map<int,int>;

	for (int i=0; i<this->T_A; i++)
		this->c_tw_ATV[i] = new std::map<int,int>;

	for (int i=0; i<this->T_D; i++)
		this->c_tw_DTV[i] = new std::map<int,int>;

	// Construct a lookup table for indices and indicator, topic combinations in form of tuples (y,t). Used for sampling.
	this->N_comb = this->T_A + this->T_D;
	this->lookup = new std::map<int,std::pair<int,int> >;	// Store combinations as identifier, topic pairs. identifier in form of bool. identifier = False if document topic, true if author topic.
	// Iterate over document topics.
	for (int t=0; t<this->T_D; t++) {
		this->lookup->insert( std::pair<int, std::pair<int,int> > (t, std::pair<int,int>(0,t) ));
	}
	for (int t=0; t<this->T_A; t++) {
		this->lookup->insert( std::pair<int, std::pair<int,int> > (t+this->T_D, std::pair<int,int>(1,t) ));
	}

	// Seed the random number generator.
	this->init_rng();
	// Initialize probability vector.
	for (int i=0; i<this->N_comb; i++) this->p.push_back(1);

	// Set flag for first iteration.
	this->is_first_iteration = true;

}

MIChain::~MIChain() {
	// TODO Auto-generated destructor stub
	// Delete all arrays on document basis.


	for (int i=0; i<this->A; i++)
		delete this->c_at_AT[i];

	for (int i=0; i<this->T_A; i++)
		delete this->c_tw_ATV[i];

	for (int i=0; i<this->T_D; i++)
		delete this->c_tw_DTV[i];

	for (int i=1; i<this->D; i++) {
		delete[] this->c_dt_DT[i];
		delete[] this->words[i];
		delete[] this->topics[i];
		delete[] this->indicators[i];
	}

	delete[] this->c_dt_DT;
	//	Delete all arrays on corpus level.
	delete[] this->words;
	delete[] this->topics;
	delete[] this->indicators;
	delete[] this->N;

	delete[] this->c_d_DA;
	delete[] this->c_d_DD;

	delete[] this->c_at_AT;
	delete[] this->c_tw_ATV;
	delete[] this->c_tw_DTV;

	delete[] this->beta_A;
	delete[] this->beta_D;

	delete this->lookup;
}

void MIChain::init_rng() {
	// Seed the boost random number generator engine.
	this->rng.seed(0);
}

int MIChain::sum_map_values(std::map<int,int>* m){
	// Function to sum the values in a std::map<int,int>.
	int sum = 0;
	// Sum over all entries in the map.
	for (std::map<int,int>::iterator it=m->begin(); it!=m->end(); ++it) {
		sum += it->second;
	}
	return sum;
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
	int y_new = this->lookup->at(r).first;
	int t_new = this->lookup->at(r).second;

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
		b = this->delta_D + this->c_d_DD[t];
		// n1 is the first numerator.
		if (this->c_dt_DT[d]->count(t) == 1) n1 = this->alpha_D + this->c_dt_DT[d]->at(t);
		else n1 = this->alpha_D;
		// n2 is the second numerator.
		if (this->c_tw_DTV[t]->count(w) == 1) n2 = this->beta_D[w] + this->c_tw_DTV[t]->at(w);
		else n2 = this->beta_D[w];
		// d1 is the first denominator.
		d1 = this->sum_alpha_D + this->sum_map_values(this->c_dt_DT[d]);
		if (y_old == 0) d1--;	// All counts exclude current index.
		// d2 is the second denominator.
		d2 = this->sum_beta_D + this->sum_map_values(this->c_tw_DTV[t]);
		if (y_old == 0 and t_old == t) d2--;
		// Calculate the probability.
		this->p[cnt++] = b*n1*n2/d1/d2;
	}
	for (int t=0; t<this->T_A; t++) {		// Loop over author topics.
		// Probability for author topic y = 1.
		// First calculate all the factors needed.
		// b is the term in parentheses.
		b = this->delta_A + this->c_d_DA[t];
		// n1 is the first numerator.
		if (this->c_at_AT[a]->count(t) == 1) n1 = this->alpha_A + this->c_at_AT[a]->at(t);
		else n1 = this->alpha_A;
		// n2 is the second numerator.
		if (this->c_tw_ATV[t]->count(w) == 1) n2 = this->beta_A[w] + this->c_tw_ATV[t]->at(w);
		else n2 = this->beta_A[w];
		// d1 is the first denominator.
		d1 = this->sum_alpha_A + this->sum_map_values(this->c_at_AT[a]);
		if (y_old == 1) d1--;	// All counts exclude current index.
		// d2 is the second denominator.
		d2 = this->sum_beta_A + this->sum_map_values(this->c_tw_ATV[t]);
		if (y_old == 1 and t_old == t) d2--;
		// Calculate the probability.
		this->p[cnt++] = b*n1*n2/d1/d2;
	}

	// Normalize the probability.
	double sum = 0;
	for (int i=0; i<this->N_comb; i++) sum += p[i];
	for (int i=0; i<this->N_comb; i++) p[i] = p[i]/sum;
}

int MIChain::draw() {
	// Draw from the discrete distribution given by weights p.
	boost::random::discrete_distribution<> dist(p.begin(), p.end());
	return dist(this->rng);
}

void MIChain::update_counts(int w, int d, int a, int y_old, int y_new, int t_old, int t_new){
	// Update counts.
	// Consider the first iteration.
	if (!this->is_first_iteration) {
		// Lower the counts according to the old variables.
		if (y_old==0) {		// Belonged to document topic.
			this->c_d_DD[d] -= 1;
			this->c_dt_DT[d]->at(t_old) -= 1;
			this->c_tw_DTV[t_old]->at(w) -= 1;
		} else {			// Belonged to author topic.
			this->c_d_DA[a] -= 1;
			this->c_at_AT[a]->at(t_old) -= 1;
			this->c_tw_ATV[t_old]->at(w) -= 1;
		}
	}
	// Increment to counts according to the new variables.
	if (y_new == 0) {		// Belongs to a document topic.
		this->c_d_DD[d] += 1;

		if (this->c_dt_DT[d]->count(t_new) == 1) this->c_dt_DT[d]->at(t_new) += 1;
		else this->c_dt_DT[d]->insert(std::pair<int,int>(t_new, 1));

		if (this->c_tw_DTV[t_new]->count(w) == 1) this->c_tw_DTV[t_new]->at(w) +=1;
		else this->c_tw_DTV[t_new]->insert(std::pair<int,int>(w, 1));
	} else {				// Belongs to an author topic.
		this->c_d_DA[d] += 1;

		if (this->c_at_AT[a]->count(t_new) == 1) this->c_at_AT[a]->at(t_new) += 1;
		else this->c_at_AT[a]->insert(std::pair<int,int>(t_new, 1));

		if (this->c_tw_ATV[t_new]->count(w) == 1) this->c_tw_ATV[t_new]->at(w) +=1;
		else this->c_tw_ATV[t_new]->insert(std::pair<int,int>(w, 1));
	}
}

void MIChain::iterate(int count) {
	// Iterate count time over all documents and words.
	for (int i=0; i < count; i++) {
		for (int d=0; d < this->D; d++) {
			std::cout << "Processing document " << d <<"." << std::endl;
			for (int ind=0; ind < this->N[d]; ind++) {
				this->sample(d,ind);
			}
		}
	}
}

