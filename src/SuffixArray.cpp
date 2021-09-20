#include "../inc/SuffixArray.hpp"
#include <chrono>




/**
 * @mainpage
 * 
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
 */



///AlphabetType
//the input is stored temporarily in SuffixArray as vector of type AlphabetType
//IF *alph* SET TO A DATA-TYPE,THAT IS SMALLER THAN UNSIGNED AND LCP IS USED, THERE IS A SEGMENTATION FAULT WITH MY TEST-CORPUS!!
//PROBALY MULTIPLE ENCODINGS IN THE TEST-CORPUS
typedef unsigned char AlphabetType;

///IndexType
//determines the type of indices, that are stored in the final suffix-array
typedef unsigned IndexType;


/*
void stop_print_time(std::chrono::steady_clock start){
   
}
*/

int main(int argc, char** argv){
   auto start(std::chrono::steady_clock::now());
   ///in "parseModes.hpp"
   Modes modes = Modes(argc, argv);

   if(modes.shortInput() || !modes["Infix"].size()){
      ///in SuffixArray.hpp
      SuffixArray<AlphabetType, IndexType> SA(modes);
   }
   else{
      if(!modes["InFile"].size()){
         std::cerr << "ERROR : SuffixArray previously stored data-file ''-i'' not specified!\n";
         exit(1);
      }
      else{
         ///in SuffixArray.hpp
         findInfix<AlphabetType, IndexType>(modes["Infix"], 
                                            modes["InFile"], 
                                            modes["SubstringWidth"]);
      }
   }

   std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
}




