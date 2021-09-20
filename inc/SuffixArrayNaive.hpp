#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "typeChecks.hpp"

#include "tbb/parallel_sort.h"

#include "tbb/task_scheduler_init.h"


///Template-Function-Object for creating a suffix-array
template<typename alph=char, typename idxT=size_t>
class SuffixArrayNaive{
///This is the naive version for creating the array,
///there is a serial version (using **STL** *std::sort*) 
///and a parallel version (using **TBB** *tbb::parallel_sort*)
///
///- typename *alph* 
///> is required for using Comp, which uses *std::lexicographical_compare*
///>
///> pointers of type *alph\** are stored within Comp.
///
///- typename idxT
///> determines type of indices, stored in the final suffix-array
private:
struct Comp;
public:
   ///the constructor takes one optional boolean argument, if true, parallel-version is used, if false, serial version is used
   SuffixArrayNaive(const bool& p=true)
   : p_mode(p)
   {}
   ///- *operator()*, takes a *std::vector* of type *alph* as argument
   ///- returns a *std::vector* of type *idxT* (final suffix-array)
   
   ///last two arguments dont matter here
   std::vector<idxT> operator()(const std::vector<alph>& data_in, const idxT& _=0){
      checkIndexType<idxT>();
      checkDataSizeIdxT<idxT>(data_in.size());
      
      if(data_in.size() > idxT(-1)){}
      std::vector<idxT> SA = make_SA(&(*(data_in.begin())),
                                       &(*(data_in.end())));
      return SA;
   }
   
private:
   ///- *make_SA* takes pointers to the beginning and the end of the input-vector as arguments
   ///- returns a *std::vector* of type *idxT* (final suffix-array)
   std::vector<idxT> make_SA(const alph* begin, 
                              const alph* end) const {
      ///1. initialize suffix-array
      std::vector<idxT> indices;
      indices.reserve(end-begin+1);
      
      indices.push_back(end-begin);
      
      for(idxT i = 0 ; i < end-begin ; ++i){
         indices.push_back(i);
      }
//       indices.resize(end-begin+1, 0);
      indices[0] = end-begin;
//       std::iota(indices.begin()+1, indices.end(), 0);
      ///2. sort suffix-array with Comp
      if(p_mode){
         tbb::task_scheduler_init tbb_init;
         
         tbb::parallel_sort(indices.begin()+1, 
                           indices.end(), 
                           Comp(begin, end));
      }
      else{
         std::sort(indices.begin()+1, 
                  indices.end(), 
                  Comp(begin, end));   
      }
      ///3. Sets first element of the suffix-array as the empty(smallest) suffix
      return indices;
   }
   
   ///Compare fuction-object
   struct Comp{
   public:
      ///constructor takes pointers to the beginning and the end of the input-vector as arguments and stores them in *beg_corp* and *end_corp*
      Comp(const alph* beg, const alph* end){
         beg_corp = beg;
         end_corp = end;
      }
      ///- *operator()*, takes indices of type *idxT* as arguments
      ///- returns true, if suffix starting at *beg_corp* + idx1 is smaller than suffix starting at *beg_corp* + idx2, using *std::lexicographical_compare*
      bool operator()(const idxT& idx1, 
                      const idxT& idx2) const {
         return std::lexicographical_compare(beg_corp+idx1, end_corp,
                                             beg_corp+idx2, end_corp);
      }
   private:
      ///beginning of input-vector
      const alph* beg_corp;
      ///end of input-vector
      const alph* end_corp;
   };
   
   ///Parallel mode off or on
   const bool p_mode;
};





