Author : Mino Lee Sasse

Written on : ElementaryOS Loki (base Ubuntu 16.04?)


For installation, compilation and documentation run "make" in this directory

For installation of thread-building-blocks-library run:
make installation

For compilation of c++ scripts run:
make installation

For documentation via doxygen run:
make installation


This project implements the SA-IS-algorithm for creating a 
suffix-array introduced by G.Nong, S. Zhang and W.H. Chan in 
"Linear Suffix Array Construction by Almost Pure Induced-Sorting"

There is also a naive implementation, using the STL sorting-algorithm
and a naive parallel implementation, using the thread-building-blocks-
library.

Additionally there is a naive implementation for a LCP_LR-Array. But it runs
very slowly and takes up too much space, so only use it for very small
corpora. The binary search should be improved to m+log(n) but even 
reading the stored data takes up too much time. 
(and STL lower_bound is faster than my own lcp-lr-lower_bound-version...)


For information on usage, please run:
man ./SuffixArray.man

