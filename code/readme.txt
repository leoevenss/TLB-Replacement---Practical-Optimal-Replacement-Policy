Compilation:

gcc -O3 lru_llc.c -o lru_llc
gcc -O3 lru_tlb.c -o lru_tlb
gcc -O3 opt_llc.c -o opt_llc
gcc -O3 opt_tlb.c -o opt_tlb
gcc -O3 topt_llc.c -o topt_llc
gcc -O3 topt_tlb.c -o topt_tlb
gcc -O3 popt_inter_llc.c -o popt_inter_llc
gcc -O3 popt_inter_tlb.c -o popt_inter_tlb
gcc -O3 popt_inter_intra_llc.c -o popt_inter_intra_llc
gcc -O3 popt_inter_intra_tlb.c -o popt_inter_intra_tlb
gcc -O3 popt_inter_intra_se_llc.c -o popt_inter_intra_se_llc
gcc -O3 popt_inter_intra_se_tlb.c -o popt_inter_intra_se_tlb

To run:

To see the command line arguments, just run the executable without any command line argument e.g., ./lru_tlb and then after getting the command line
arguments you can run it with command line arguments.