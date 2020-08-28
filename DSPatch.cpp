// Implementation of the page buffer and signature prediction table
#include "DSPatch.h"

using namespace std;

void update_spt(umap_spt &sp_table, umap_pb page_buffer, long page_number)
{
    int new_ind = (sp_table[page_number].index + 1) % NUM_BIT_PATTERN;
    sp_table[page_number].index = new_ind;
    sp_table[page_number].patterns[new_ind] = page_buffer[page_number].bit_pattern;
    sp_table[page_number].access_count++;
    // Perform Trigger Rotation Here #TODO
    rotate(sp_table[page_number].patterns[new_ind].begin(),
           sp_table[page_number].patterns[new_ind].begin() + page_buffer[page_number].trigger_local_addr,
           sp_table[page_number].patterns[new_ind].end());
    // For-loop bitwise AND for every string for ACCP
    string and_res;
    for (int i = 0; i < sp_table[page_number].patterns[new_ind].size(); i++)
    {
        int curr_bit = 1;
        for (int j = 0; j < NUM_BIT_PATTERN; j++)
        {
            curr_bit = curr_bit & (sp_table[page_number].patterns[j][i] - '0');
        }
        and_res += (curr_bit + '0');
    }
    sp_table[page_number].accp = and_res;
    string or_res;
    for (int i = 0; i < sp_table[page_number].patterns[new_ind].size(); i++)
    {
        int curr_bit = 0;
        for (int j = 0; j < NUM_BIT_PATTERN; j++)
        {
            curr_bit = curr_bit | (sp_table[page_number].patterns[j][i] - '0');
        }
        or_res += (curr_bit + '0');
    }
    sp_table[page_number].covp = or_res;
}

void update_page_buffer(umap_pb &page_buffer, ll addr, long pc, long page_number, long local_page_addr)
{
    // Compression Factor must be applied to the local page address
    local_page_addr = local_page_addr / COMPRESSION_FACTOR;
    // check if page map exists
    if (page_buffer.find(page_number) == page_buffer.end())
    {
        // initialize the mapping
        page_data new_page;
        new_page.trigger_local_addr = local_page_addr;
        new_page.prev_pc = pc;
        new_page.prev_addr = addr;
        new_page.prev_bit_position = local_page_addr;
        new_page.bit_pattern = string(pow(2, 12) / COMPRESSION_FACTOR, '0'); // ONLY KEEP COMPRESSION_FACTOR 1 OR DIVISIBLE BY 2
        new_page.access_count = 1;
        new_page.bit_pattern.replace(local_page_addr, 1, "1");
        page_buffer[page_number] = new_page;
    }
    else
    {
        // update the mapping
        long delta = (addr - page_buffer[page_number].prev_addr) / COMPRESSION_FACTOR;
        long new_bit_postion = (page_buffer[page_number].prev_bit_position + delta);
        page_buffer[page_number].bit_pattern.replace(new_bit_postion, 1, "1");
        page_buffer[page_number].prev_bit_position = new_bit_postion;
        page_buffer[page_number].prev_pc = pc;
        page_buffer[page_number].prev_addr = addr;
        page_buffer[page_number].access_count++;
    }
}

bool trigger_access(umap_pb page_buffer, long page_number)
{
    // Check if Trigger Access and PB Full
    return (page_buffer.find(page_number) != page_buffer.end()) && (page_buffer[page_number].access_count >= BIT_PATTERN_LENGTH);
}

string get_bit_pattern(umap_spt sp_table, long page_number)
{
// Decide if you want covp or accp
#ifdef COVP
    return sp_table[page_number].covp;
#endif
    return sp_table[page_number].accp;
}

vector<uint64_t> extract_addr(string bit_pattern, long page_number, long local_page_addr)
{
    // This time we must do a right rotation to adjust for the local_page_addr
    rotate(bit_pattern.begin(),
           bit_pattern.begin() + bit_pattern.size() - local_page_addr,
           bit_pattern.end());
    vector<uint64_t> prefetch_addr;
    for (int i = 0; i < bit_pattern.size(); i++)
    {
        if (bit_pattern.at(i) == '1')
        {
            uint64_t addr = i * (COMPRESSION_FACTOR) + page_number * pow(2, 12);
            prefetch_addr.push_back(addr);
        }
    }
    return prefetch_addr;
}
