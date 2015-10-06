/*
 * MIChain_test.cpp
 *
 *  Created on: Sep 16, 2015
 *      Author: thomas
 */

#include "MIChain.h"

#include "gtest/gtest.h"
#include <math.h>

#include <boost/timer.hpp>

TEST(InitTest, CanInitialize) {
    MIChain* chain = new MIChain("../PAN11Large.h5",0);
    EXPECT_EQ(72, chain->A);		// Check number of authors.
    EXPECT_EQ(9337, chain->D);		// Check number of documents.
    EXPECT_EQ(27101, chain->V);		// Check number of words in vocabulary.
    // Check some entries in the word count vector.
    EXPECT_EQ(chain->N[200], 15);
    EXPECT_EQ(chain->N[4044], 490);
    EXPECT_EQ(chain->N[6490], 31);
    // Check some entries in the author vector.
    EXPECT_EQ(chain->authors[200], 44);
    EXPECT_EQ(chain->authors[3487], 28);
    EXPECT_EQ(chain->authors[7477], 61);
    // Check some words.
    EXPECT_EQ(chain->words[267][31], 13019);
    EXPECT_EQ(chain->words[5572][6], 8613);
    EXPECT_EQ(chain->words[814][40], 10987);
    // Check word in topic priors entries for a stopword and a normal word.
    // Stopword 10304.
    EXPECT_DOUBLE_EQ(0.019, chain->beta_A[10304]);
    EXPECT_DOUBLE_EQ(0.001, chain->beta_D[10304]);
    // Normal word 5008.
    EXPECT_DOUBLE_EQ(0.01, chain->beta_A[5008]);
    EXPECT_DOUBLE_EQ(0.01, chain->beta_D[5008]);
    // Check topic priors.
    EXPECT_DOUBLE_EQ(5.0/90, chain->alpha_A);
    EXPECT_DOUBLE_EQ(0.1, chain->alpha_D);
    // Check cached sums.
    EXPECT_DOUBLE_EQ(5.0, chain->sum_alpha_A);
    EXPECT_DOUBLE_EQ(1.0, chain->sum_alpha_D);
    EXPECT_DOUBLE_EQ(27101*0.01+0.009*500, chain->sum_beta_A);
    EXPECT_DOUBLE_EQ(27101*0.01-0.009*500, chain->sum_beta_D);
    // Increment index 2 of c_dt_DT in document 10 five times.
    for (int i=0; i<5; i++) chain->c_dt_DT[10]->inc(2);
    EXPECT_EQ(5, chain->c_dt_DT[10]->at(2));
    // Check number of identifier and topic combinations.
    EXPECT_EQ(100, chain->N_comb);
    // Check some pairs in lookup table.
    // First: an author topic.
    EXPECT_TRUE(chain->lookup_table->at(25).first);
    EXPECT_EQ(15 ,chain->lookup_table->at(25).second);
    // Second: a document topic.
    EXPECT_FALSE(chain->lookup_table->at(4).first);
    EXPECT_EQ(4 ,chain->lookup_table->at(4).second);
}


TEST(SampleTest, ProbabilitiesPositive_SumTo1) {
    MIChain* chain = new MIChain("../PAN11Large.h5",0);
    int d=15, ind=2;
    // Get the word.
	int w = chain->words[d][ind];
	// Get the author of the document.
	int a = chain->authors[d];
	// Get the topic indicator.
	int y_old = chain->indicators[d][ind];
	// Get the topic.
	int t_old = chain->topics[d][ind];

	chain->calc_prob(w, a ,y_old, t_old, d, ind);

	double sum = 0;
	for (int i=0; i<chain->N_comb; i++) {
		EXPECT_GE(chain->p[i], 0.0);
		sum += chain->p[i];
	}
	EXPECT_LT(1-sum, 1/pow(10,14));
}

TEST(SampleTest, time) {
    MIChain* chain = new MIChain("../PAN11Large.h5",0);
    boost::timer t;
    chain->sample(20,1);
    std::cout << t.elapsed() << " sec elapsed for one sampling." << std::endl;
}

//TEST(IterateTest, time) {
//    MIChain* chain = new MIChain("../PAN11Large.h5",0);
//    chain->iterate(1);
//    boost::timer t;
//    int N = 40;
//    chain->iterate(N);
//    std::cout << t.elapsed()/N << " sec elapsed for one iteration on average." << std::endl;
//}

//TEST(SaveTest, runs) {
//	MIChain* chain = new MIChain("../PAN11Large.h5",0);
//	chain->save_sample();
//	chain->iterate(1);
//	chain->save_sample();
//}

//TEST(LoadTest, runs) {
//	MIChain* chain1 = new MIChain("../PAN11Large.h5",0);
//	chain1->iterate(1);
//	chain1->save_sample();
//	MIChain* chain2 = new MIChain("../PAN11Large.h5",0);
//	chain2->load_sample(0,1);
//	EXPECT_EQ(chain1->indicators[5][2], chain2->indicators[5][2]);
//	EXPECT_EQ(chain1->c_d_DA->at(2), chain2->c_d_DA->at(2));
//	EXPECT_EQ(chain1->num_iter, chain2->num_iter);
//}

//TEST(IterationTest, CounterSum) {
//	MIChain* chain = new MIChain("../PAN11Large.h5",0);
//	for (int i=0; i<5; i++) {
//		chain->iterate(5);
//		EXPECT_TRUE(chain->check_sums());
//	}
//}

TEST(DistTest, runs) {
	MIChain* chain = new MIChain("../PAN11Large.h5",0);
	chain->iterate(1);
	chain->calc_distributions();
}
