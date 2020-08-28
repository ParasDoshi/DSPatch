#include "Trace.h"
#include "Cache.h"
#include "DSPatch.h"

#define DSPATCH

extern TraceParser *initTraceParser(const char *mem_file);
extern bool getRequest(TraceParser *mem_trace);

extern Cache *initCache();
extern bool accessBlock(Cache *cache, Request *req, uint64_t access_time);
extern bool insertBlock(Cache *cache, Request *req, uint64_t access_time, uint64_t *wb_addr);

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s %s\n", argv[0], "<mem-file>");

        return 0;
    }

    // Initialize a CPU trace parser
    TraceParser *mem_trace = initTraceParser(argv[1]);

    // Initialize a Cache
    Cache *cache = initCache();

    // Initialize DSPatch Prefetcher
    umap_pb page_buffer;
    umap_spt sp_table;

    // Running the trace
    uint64_t num_of_reqs = 0;
    uint64_t hits = 0;
    uint64_t misses = 0;
    uint64_t num_evicts = 0;

    uint64_t cycles = 0;
    while (getRequest(mem_trace))
    {
        // printf("Processing: %"PRIu64"\n", mem_trace->cur_req->load_or_store_addr);
        // Step one, accessBlock()
        if (accessBlock(cache, mem_trace->cur_req, cycles))
        {
            // Cache hit
            hits++;
        }
        else
        {
            // Cache miss!
            misses++;

#ifdef DSPATCH
            //Determine variables required for PB and SPT
            long page_number = mem_trace->cur_req->load_or_store_addr / (pow(2, 12));
            long local_page_addr = mem_trace->cur_req->load_or_store_addr - page_number * pow(2, 12);
            // cout << "Processing ==> PC: " << mem_trace->cur_req->PC << ", Addr: "
            //      << mem_trace->cur_req->load_or_store_addr << ", Page Number: " << page_number << '\n';

            // // Check if Trigger Access and PB Full
            if (trigger_access(page_buffer, page_number))
            {
                update_spt(sp_table, page_buffer, page_number);
                // Reset the page PB
                page_buffer.erase(page_number);
                string bit_pattern = get_bit_pattern(sp_table, page_number);
                // cout<< "Bit Pattern: " << bit_pattern << endl;

                // Extract vector of memory accesses based on bit pattern and current trigger rotation
                vector<uint64_t> prefetch_addr = extract_addr(bit_pattern, page_number, local_page_addr);

                //Insert data in cache
                uint64_t wb_addr;

                for (int i = 0; i < prefetch_addr.size(); ++i)
                {
                    // Make request objects
                    Request prefetch_data;
                    prefetch_data.load_or_store_addr = prefetch_addr[i];
                    prefetch_data.req_type = LOAD;
                    if (insertBlock(cache, &prefetch_data, cycles, &wb_addr))
                    {
                        num_evicts++;
                        //                printf("Evicted: %"PRIu64"\n", wb_addr);
                    }
                }
            }

            // Update Page Buffer
            update_page_buffer(page_buffer,
                               mem_trace->cur_req->load_or_store_addr,
                               mem_trace->cur_req->PC,
                               page_number,
                               local_page_addr);

#else

            // Step two, insertBlock()
            //    printf("Inserting: %"PRIu64"\n", mem_trace->cur_req->load_or_store_addr);
            uint64_t wb_addr;
            if (insertBlock(cache, mem_trace->cur_req, cycles, &wb_addr))
            {
                num_evicts++;
                //                printf("Evicted: %"PRIu64"\n", wb_addr);
            }
#endif
        }

        ++num_of_reqs;
        ++cycles;
    }

    double hit_rate = (double)hits / ((double)hits + (double)misses);
    printf("Hit rate: %lf%%\n", hit_rate * 100);
}
