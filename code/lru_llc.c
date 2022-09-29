#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define BLOCK_OFFSET 6

#define L1_SETS 64
#define L1_ASSOC 8

#define L2_SETS 512
#define L2_ASSOC 8

#define L3_SETS 2048
#define L3_ASSOC 16

#define HEART_RATE 1000000

typedef struct cacheline_s {
   char valid;
   unsigned long long tag;
   unsigned long long lru;
} cacheline_t;

void cache_sim (unsigned long long addr, cacheline_t **l1cache, cacheline_t **l2cache, cacheline_t **l3cache, int *l1hit, int *l2hit, int *l3hit)
{
   unsigned long long block_addr = addr >> BLOCK_OFFSET;
   int l1set = block_addr & (L1_SETS - 1);
   int l2set = block_addr & (L2_SETS - 1);
   int l3set = block_addr & (L3_SETS - 1);
   int i, j;

   (*l1hit) = 0;
   (*l2hit) = 0;
   (*l3hit) = 0;

   assert(l1cache);
   assert(l3cache);

   for (i=0; i<L1_ASSOC; i++) {
      if (l1cache[l1set][i].valid && (l1cache[l1set][i].tag == block_addr)) {
         (*l1hit) = 1;
         for (j=0; j<L1_ASSOC; j++) l1cache[l1set][j].lru++;
         l1cache[l1set][i].lru = 0;
         return;
      }
   }

   assert (i == L1_ASSOC);
   for (i=0; i<L1_ASSOC; i++) {
      if (!l1cache[l1set][i].valid) break;
   }
   if (i == L1_ASSOC) {
      unsigned long long maxlru = 0;
      int index = L1_ASSOC;
      for (i=0; i<L1_ASSOC; i++) {
         if (l1cache[l1set][i].lru > maxlru) {
            maxlru = l1cache[l1set][i].lru;
            index = i;
         }
      }
      i = index;
   }
   assert(i < L1_ASSOC);
   l1cache[l1set][i].tag = block_addr;
   l1cache[l1set][i].valid = 1;
   for (j=0; j<L1_ASSOC; j++) l1cache[l1set][j].lru++;
   l1cache[l1set][i].lru = 0;

   if (l2cache) {
      for (i=0; i<L2_ASSOC; i++) {
         if (l2cache[l2set][i].valid && (l2cache[l2set][i].tag == block_addr)) {
            (*l2hit) = 1;
            for (j=0; j<L2_ASSOC; j++) l2cache[l2set][j].lru++;
            l2cache[l2set][i].lru = 0;
            return;
         }
      }

      assert (i == L2_ASSOC);
      for (i=0; i<L2_ASSOC; i++) {
         if (!l2cache[l2set][i].valid) break;
      }
      if (i == L2_ASSOC) {
         unsigned long long maxlru = 0;
         int index = L2_ASSOC;
         for (i=0; i<L2_ASSOC; i++) {
            if (l2cache[l2set][i].lru > maxlru) {
               maxlru = l2cache[l2set][i].lru;
               index = i;
            }
         }
         i = index;
      }
      assert(i < L2_ASSOC);
      l2cache[l2set][i].tag = block_addr;
      l2cache[l2set][i].valid = 1;
      for (j=0; j<L2_ASSOC; j++) l2cache[l2set][j].lru++;
      l2cache[l2set][i].lru = 0;
   }

   for (i=0; i<L3_ASSOC; i++) {
      if (l3cache[l3set][i].valid && (l3cache[l3set][i].tag == block_addr)) {
         (*l3hit) = 1;
         for (j=0; j<L3_ASSOC; j++) l3cache[l3set][j].lru++;
         l3cache[l3set][i].lru = 0;
         return;
      }
   }

   assert (i == L3_ASSOC);
   for (i=0; i<L3_ASSOC; i++) {
      if (!l3cache[l3set][i].valid) break;
   }
   if (i == L3_ASSOC) {
      unsigned long long maxlru = 0;
      int index = L3_ASSOC;
      for (i=0; i<L3_ASSOC; i++) {
         if (l3cache[l3set][i].lru > maxlru) {
            maxlru = l3cache[l3set][i].lru;
            index = i;
         }
      }
      i = index;
   }
   assert(i < L3_ASSOC);
   l3cache[l3set][i].tag = block_addr;
   l3cache[l3set][i].valid = 1;
   for (j=0; j<L3_ASSOC; j++) l3cache[l3set][j].lru++;
   l3cache[l3set][i].lru = 0;
}

int main (int argc, char **argv)
{
   cacheline_t **l1cache, **l2cache, **l3cache;
   int l1hit, l2hit, l3hit;
   unsigned long long l1hits = 0, l2hits = 0, l3hits = 0;
   unsigned long long rl1hits = 0, rl2hits = 0, rl3hits = 0;
   unsigned long long il1hits = 0, il2hits = 0, il3hits = 0;
   unsigned long long addr, count = 0, rcount = 0, icount = 0;
   unsigned id;
   FILE *fp;
   int i, j;
   unsigned long long ir_start, ir_num, ir_end;
   int ir_size;

   if (argc != 5) {
      printf("Need trace file name, irregular start addr, irregular num elements, irregular element size. Aborting ...\n");
      exit(0);
   }

   l1cache = (cacheline_t **)malloc(L1_SETS*sizeof(cacheline_t *));
   assert(l1cache);
   for (i=0; i<L1_SETS; i++) {
      l1cache[i] = (cacheline_t*)malloc(L1_ASSOC*sizeof(cacheline_t));
      assert(l1cache[i]);
      for (j=0; j<L1_ASSOC; j++) l1cache[i][j].valid = 0;
   }

   l2cache = (cacheline_t **)malloc(L2_SETS*sizeof(cacheline_t *));
   assert(l2cache);
   for (i=0; i<L2_SETS; i++) {
      l2cache[i] = (cacheline_t*)malloc(L2_ASSOC*sizeof(cacheline_t));
      assert(l2cache[i]);
      for (j=0; j<L2_ASSOC; j++) l2cache[i][j].valid = 0;
   }

   l3cache = (cacheline_t **)malloc(L3_SETS*sizeof(cacheline_t *));
   assert(l3cache);
   for (i=0; i<L3_SETS; i++) {
      l3cache[i] = (cacheline_t*)malloc(L3_ASSOC*sizeof(cacheline_t));
      assert(l3cache[i]);
      for (j=0; j<L3_ASSOC; j++) l3cache[i][j].valid = 0;
   }

   ir_start = atoll(argv[2]);
   ir_num = atoll(argv[3]);
   ir_size = atoi(argv[4]);
   ir_end = ir_start + ir_num*ir_size;

   fp = fopen(argv[1], "r");

   while (!feof(fp)) {
      int x = fscanf(fp, "%u %llu", &id, &addr);
      if ((x == EOF) || (x == 0)) break;
      cache_sim (addr, l1cache, l2cache, l3cache, &l1hit, &l2hit, &l3hit);
      l1hits += l1hit;
      l2hits += l2hit;
      l3hits += l3hit;
      count++;
      if ((count % HEART_RATE) == 0) {
         printf(".");
         fflush(stdout);
      }
      if ((addr >= ir_start) && (addr < ir_end)) {
         il1hits += l1hit;
         il2hits += l2hit;
         il3hits += l3hit;
         icount++;
      }
      else {
         rl1hits += l1hit;
         rl2hits += l2hit;
         rl3hits += l3hit;
         rcount++;
      }
   }
   fclose (fp);

   printf("\nACCESS COUNT: %llu\n", count);
   printf("L1 HITS: %llu, L2 HITS: %llu, L3 HITS: %llu\n", l1hits, l2hits, l3hits);
   printf("L1 MISS: %llu, L2 MISS: %llu, L3 MISS: %llu\n", count - l1hits, count - l1hits - l2hits, count - l1hits - l2hits - l3hits);
   printf("L1 MR: %.2lf%%, L2 MR: %.2lf%%, L3 MR: %.2lf%%\n", (100.0*(count - l1hits))/count, (100.0*(count - l1hits - l2hits))/count, (100.0*(count - l1hits - l2hits - l3hits))/count);
   printf("\nREGULAR ACCESS COUNT: %llu\n", rcount);
   printf("L1 REGULAR HITS: %llu, L2 REGULAR HITS: %llu, L3 REGULAR HITS: %llu\n", rl1hits, rl2hits, rl3hits);
   printf("L1 REGULAR MISS: %llu, L2 REGULAR MISS: %llu, L3 REGULAR MISS: %llu\n", rcount - rl1hits, rcount - rl1hits - rl2hits, rcount - rl1hits - rl2hits - rl3hits);
   printf("L1 REGULAR MR: %.2lf%%, L2 REGULAR MR: %.2lf%%, L3 REGULAR MR: %.2lf%%\n", (100.0*(rcount - rl1hits))/rcount, (100.0*(rcount - rl1hits - rl2hits))/rcount, (100.0*(rcount - rl1hits - rl2hits - rl3hits))/rcount);
   printf("\nIRREGULAR ACCESS COUNT: %llu\n", icount);
   printf("L1 IRREGULAR HITS: %llu, L2 IRREGULAR HITS: %llu, L3 IRREGULAR HITS: %llu\n", il1hits, il2hits, il3hits);
   printf("L1 IRREGULAR MISS: %llu, L2 IRREGULAR MISS: %llu, L3 IRREGULAR MISS: %llu\n", icount - il1hits, icount - il1hits - il2hits, icount - il1hits - il2hits - il3hits);
   printf("L1 IRREGULAR MR: %.2lf%%, L2 IRREGULAR MR: %.2lf%%, L3 IRREGULAR MR: %.2lf%%\n", (100.0*(icount - il1hits))/icount, (100.0*(icount - il1hits - il2hits))/icount, (100.0*(icount - il1hits - il2hits - il3hits))/icount);
   return 0;
}
