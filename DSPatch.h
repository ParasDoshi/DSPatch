// Implementation definition of the page buffer and signature prediction table
#ifndef __DSPATCH_HH__
#define __DSPATCH_HH__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <math.h>
#include <algorithm>

using namespace std;

#define BIT_PATTERN_LENGTH 8 // IN PAPER THIS IS CONFIGURED TO 64
#define NUM_BIT_PATTERN 3    // NUMBER OF BIT PATTERN FOR ACCP COVP
#define COMPRESSION_FACTOR 1 // PAPER DEFINES THIS AS 2
#define COVP

typedef long long ll;

struct page_data
{
    long prev_pc;
    ll prev_addr;
    long prev_bit_position;
    string bit_pattern;
    int access_count;
    long trigger_local_addr;
};

struct spt_data
{
    string covp;
    string accp;
    string patterns[NUM_BIT_PATTERN];
    int index;
    long access_count;
};

typedef unordered_map<long, page_data> umap_pb;
typedef unordered_map<long, spt_data> umap_spt;

void update_spt(umap_spt &sp_table, umap_pb page_buffer, long page_number);
void update_page_buffer(umap_pb &page_buffer, ll addr, long pc, long page_number, long local_page_addr);
bool trigger_access(umap_pb page_buffer, long page_number);
string get_bit_pattern(umap_spt sp_table, long page_number);
vector<uint64_t> extract_addr(string bit_pattern, long page_number, long local_page_addr);
#endif