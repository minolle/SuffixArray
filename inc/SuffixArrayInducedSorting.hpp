#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "string.h"
#include <algorithm>
#include <map>
#include <unordered_map>
#include <chrono>
#include "typeChecks.hpp"

typedef std::chrono::time_point<std::chrono::steady_clock> timeT;

void stop_time(const timeT& start, const std::string& s=""){
   std::cout <<s<< std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
}

///Template-Function-Object for creating a suffix-array
template<typename alph=char, typename idxT=size_t>
class SuffixArrayInducedSorting{
   ///This is the SA-IS-algorithm for creating the array introduced by G.Nong, S. Zhang and W.H. Chan in "Linear Suffix Array Construction by Almost Pure Induced-Sorting"
   ///
   ///- typename *alph* 
   ///> type of elements of the input-vector
   ///
   ///- typename idxT
   ///> determines type of indices, stored in the final suffix-array
   ///
   /// Using a Stack instead of explicit recursion
private:
   ///every element in the Stack is a 4-tuple
   ///1. *std::vector* of type *idxT*, representing the current suffix-array
   ///2. *std::vector* of type *bool*, representing if a index is S-Type or L-Type
   ///3. *std::vector* of type *idxT*, representing the "pointer-vector", pointing back to the suffix-array in the previous recursion-step(or the previous element in the Stack)
   ///4. *std::vector* of type *idxT*, holds the counts of all elements of *alph_cont*
   typedef std::tuple<std::vector<idxT>,
                     std::vector<bool>,
                     std::vector<idxT>,
                     std::vector<idxT>>
           StackElement;
   ///same as StackElement with the difference, that the input vector is of type *alph* (1.)
   typedef std::tuple<std::vector<alph>,
                     std::vector<bool>,
                     std::vector<idxT>,
                     std::vector<idxT>>
           FirstStackElement;
public:
   ///- *operator()*, takes a *std::vector* of type *alph* as argument
   ///- returns a *std::vector* of type *idxT* (final suffix-array)
   std::vector<idxT> operator()(const std::vector<alph> alph_cont,
                                const idxT& max) const {
      ///1. the very first call of recursion, an object of type FirstStackElement is created by *toStack* of type *FirstStackElement* and stored.
      ///2. while the size of the summary differs from the size of the alphabet of the summary (every element in the summary is not unique) then the Stack is populated. 
      ///
      ///The size of the current summary and the size of the alphabet is updated in the template-function *toStack* of type *StackElement*
      
      auto start(std::chrono::steady_clock::now());
      
      checkIndexType<idxT>();
      checkDataSizeIdxT<idxT>(alph_cont.size());
      
      std::vector<idxT> new_alph_cont;
      idxT summ_s;
      idxT summ_alph_s = max;
      ++summ_alph_s;
      
      std::cout << "summ_alph_s : " << summ_alph_s<< "\tmin"<< subtract <<'\n';
      for(auto it = alph_cont.begin() ; it != alph_cont.end() ; ++it){
         std::cout << *it <<',';
      }
      std::cout << '\n';
      
      FirstStackElement FSE = FirsttoStack(alph_cont,
                                           new_alph_cont,
                                           summ_s,
                                           summ_alph_s);
      std::cout <<"summ_s : " <<summ_s << '\n';
      std::vector<StackElement> Stack;
      std::vector<idxT> summary;
      stop_time(start);
      start = std::chrono::steady_clock::now();
      while(summ_s != summ_alph_s){
         
         std::cout << "summ_alph_s : " << summ_alph_s<<'\n';
         for(auto it = new_alph_cont.begin() ; it != new_alph_cont.end() ; ++it){
            std::cout << *it <<',';
         }
         std::cout << '\n';
         
         Stack.push_back(toStack(new_alph_cont,
                                 summary,
                                 summ_s,
                                 summ_alph_s));
         new_alph_cont = summary;
      }//while
      stop_time(start);
      start = std::chrono::steady_clock::now();
      
      ///3. If every element of the summary is unique, the population of the Stack stops
      ///and the inverse of the summary is created with *RetSummarySA*
      stop_time(start);
      start = std::chrono::steady_clock::now();
      
      Stack.shrink_to_fit();
      summary = RetSummarySA(new_alph_cont);
      
      stop_time(start);
      start = std::chrono::steady_clock::now();
      ///4. while the size of the Stack is bigger than 0 *fromStack* of type *StackElement* is called on the last element of the Stack and the summary. The summary is updated with value of *fromStack*
      stop_time(start);
      std::cout <<"hier?\n";
      start = std::chrono::steady_clock::now();
      while(Stack.size()){
         summary = fromStack(Stack.back(), summary);
         Stack.pop_back();
      }//while
      stop_time(start);
      std::cout <<"hier?\n";
      ///5. call *fromStack* of type *FirstStackElement* on the stored object of type *FirstStackElement* and the value is returned. (final suffix-array)
      return LastfromStack(FSE, summary);
   }
   
   ///constructor sets the invalid index to max of idxT
   SuffixArrayInducedSorting(const idxT& min)
   : invalid(-1), subtract(min)
   {}
   
   
private:
   
   ///function for creating the inverse vector of type *idxT* of an input-vector of type *idxT*
   ///first position of the inverse vector has to be the index of the empty suffix of the previous summary
   std::vector<idxT> RetSummarySA(const std::vector<idxT>& summary) const {
      std::vector<idxT> summarySA;
      summarySA.resize(summary.size()+1, summary.size());
      for(idxT i = 0 ; i < summary.size() ; ++i){
         summarySA[summary[i]+1] = i;
      }
      return summarySA;
   }
   
   
   ///template-function for processing the last element of the Stack
   
   ///takes the last element of the Stack and the current summary as arguments
   std::vector<idxT> fromStack(const StackElement& top, 
                               const std::vector<idxT>& summarySA) const {
      ///1. call *LMSSort* on the top of Stack
      ///2. induce-sort L-Types on result of *LMSSort* with elements of top of stack
      ///3. induce-sort R-Types on result with elements of top of stack
      ///4. return result
      std::vector<idxT> tails_bucket;
      std::cout << "frST0\n";
      std::vector<idxT> counts = std::get<3>(top);
      make_buckets(tails_bucket, counts, false);
      std::cout << "frST\n";
      std::vector<idxT> idc = LMSSort(std::get<0>(top), 
                                      tails_bucket, 
                                      std::get<1>(top), 
                                      summarySA, 
                                      std::get<2>(top));
      std::cout << "frST1\n";
      std::vector<idxT> heads_bucket;
      make_buckets(heads_bucket, counts, true);
      std::cout << "frST2\n";
      LInduceSort(idc, std::get<0>(top), std::get<1>(top), heads_bucket);
      std::cout << "frST3\n";
      SInduceSort(idc, std::get<0>(top), std::get<1>(top), tails_bucket);
      std::cout << "frST4\n";
      return idc;
   }
   
   std::vector<idxT> LastfromStack(const FirstStackElement& top, 
                                   const std::vector<idxT>& summarySA) const {
      ///1. call *LMSSort* on the top of Stack
      ///2. induce-sort L-Types on result of *LMSSort* with elements of top of stack
      ///3. induce-sort R-Types on result with elements of top of stack
      ///4. return result
      
      std::vector<idxT> tails_bucket;
      make_buckets(tails_bucket, std::get<3>(top), false);
      
      std::vector<idxT> idc = LMSSort(std::get<0>(top), 
                                      tails_bucket, 
                                      std::get<1>(top), 
                                      summarySA, 
                                      std::get<2>(top),subtract);

      std::vector<idxT> heads_bucket;
      make_buckets(heads_bucket, std::get<3>(top), true);
      LInduceSort(idc, std::get<0>(top), std::get<1>(top), heads_bucket,subtract);
      
      SInduceSort(idc, std::get<0>(top), std::get<1>(top), tails_bucket,subtract);

      return idc;
   }
   
   ///template-function for generating the element to push onto the Stack
   StackElement toStack(auto alph_cont, 
                        std::vector<idxT>& summary,
                        idxT& summ_s, 
                        idxT& summ_alph_s) const {
      ///- returns a 4-tuple
      ///- sets *summ_s* and *summ_alph_s* to continue or stop recursion(populating stack)
      ///1. generate boolean L-S-Type-array
      ///2. generate mapper to bucket-heads-tails
      ///3. store all lms-indices
      ///4. guess the SA
      ///5. induce sort L-types
      ///6. induce sort S-types
      ///7. make summary, set *summ_s*, set *summ_alph_s* and set indices-array pointing back to summary
      auto start = std::chrono::steady_clock::now();
      std::vector<bool> STypes = make_STypes(alph_cont);
      stop_time(start,"\tstypes ");

      start = std::chrono::steady_clock::now();
      std::unordered_map<idxT, idxT> all_lms;
      
      std::vector<idxT> counts(summ_alph_s, 0);
      make_counts(alph_cont, counts);
      
      std::vector<idxT> tails_bucket;
      make_buckets(tails_bucket, counts, false);
      
      std::vector<idxT> guessedSA = guessSortLMS(alph_cont, 
                                                STypes, 
                                                tails_bucket,
                                                all_lms);
      
      stop_time(start,"\tguesssort ");
      
      std::vector<idxT> heads_bucket;
      make_buckets(heads_bucket, counts, true);
      LInduceSort(guessedSA, alph_cont, STypes, heads_bucket);
      stop_time(start,"\tlinduce ");
      
      start = std::chrono::steady_clock::now();
      SInduceSort(guessedSA, alph_cont, STypes, tails_bucket);
      stop_time(start,"\tsinduce ");
      
      start = std::chrono::steady_clock::now();
      std::vector<idxT> summIdc;
      summary = summarizeSA(alph_cont, 
                           guessedSA, 
                           all_lms, 
                           summ_alph_s,
                           summIdc);
      summ_s = summary.size();
      stop_time(start,"\tsumm ");
      return std::make_tuple(alph_cont, STypes, summIdc, counts);
   }
   
   
   
   FirstStackElement FirsttoStack(auto alph_cont, 
                   std::vector<idxT>& summary,
                   idxT& summ_s, 
                   idxT& summ_alph_s) const {
      ///- returns a 4-tuple
      ///- sets *summ_s* and *summ_alph_s* to continue or stop recursion(populating stack)
      ///1. generate boolean L-S-Type-array
      ///2. generate mapper to bucket-heads-tails
      ///3. store all lms-indices
      ///4. guess the SA
      ///5. induce sort L-types
      ///6. induce sort S-types
      ///7. make summary, set *summ_s*, set *summ_alph_s* and set indices-array pointing back to summary
      auto start = std::chrono::steady_clock::now();
      std::vector<bool> STypes = make_STypes(alph_cont);
      stop_time(start,"\tstypes ");

      start = std::chrono::steady_clock::now();
      std::unordered_map<idxT, idxT> all_lms;
      
//       std::cout << summ_alph_s << ','<<subtract<<'\n';
      std::vector<idxT> counts(summ_alph_s - subtract, 0);
      make_counts(alph_cont, counts, subtract);
      
      std::vector<idxT> tails_bucket;
      make_buckets(tails_bucket, counts, false);
      
      std::vector<idxT> guessedSA = guessSortLMS(alph_cont, 
                                                STypes, 
                                                tails_bucket,
                                                all_lms,
                                                subtract);
      
      stop_time(start,"\tguesssort ");
      
      std::vector<idxT> heads_bucket;
      make_buckets(heads_bucket, counts, true);
      LInduceSort(guessedSA, alph_cont, STypes, heads_bucket,
                                                subtract);
      stop_time(start,"\tlinduce ");
      
      start = std::chrono::steady_clock::now();
      SInduceSort(guessedSA, alph_cont, STypes, tails_bucket,
                                                subtract);
      stop_time(start,"\tsinduce ");
      
      start = std::chrono::steady_clock::now();
      std::vector<idxT> summIdc;
      summary = summarizeSA(alph_cont, 
                           guessedSA, 
                           all_lms, 
                           summ_alph_s,
                           summIdc);
      summ_s = summary.size();
      stop_time(start,"\tsumm ");
      return std::make_tuple(alph_cont, STypes, summIdc, counts);
   }
   
   
   std::vector<bool> make_STypes(auto alph_cont) const {
      ///- takes a vector as argument
      ///- returns vector of type *bool*. Every index of the input vector at the output-vector, shows if the index is S-Type or not(L-Type).
      ///- an index i is S-Type, if (argument[i] < argument[i+1]) or 
      ///((argument[i] = argument[i+1]) and output[i+1] is true)
      ///- output[argument.size()] always is true, because this index represents the empty suffix
      ///- output[argument.size()-1] always is false, because the empty(smallest) suffix is always smaller than any other element, that is comparable to it
      std::vector<bool> STypes(alph_cont.size()+1);
      STypes[alph_cont.size()] = true;
      if(alph_cont.size()){
         STypes[alph_cont.size()-1] = false;
         for(idxT i = alph_cont.size()-2 ; i != invalid ; --i){
            if((alph_cont[i] > alph_cont[i+1]) || 
               ((alph_cont[i] == alph_cont[i+1]) && !(STypes[i+1])))
               STypes[i] = false;
            else
               STypes[i] = true;
         }
      }
      return STypes;
   }
   
   ///function for making the counts of all *alph* in a pre-initiated vector
   void make_counts(auto alph_cont, std::vector<idxT>& counts, const idxT& sub=0) const {
      for(auto it = alph_cont.begin() ; it != alph_cont.end() ; ++it)
         ++counts[(*it) - sub];
   }
   
   void make_buckets(std::vector<idxT>& buckets, 
                     const std::vector<idxT>& counts, 
                     const bool& heads) const {
      
      buckets.reserve(counts.size());
      if(heads){
         idxT sum = 1;
         for(auto it = counts.begin() ; it != counts.end() ; ++it){
            buckets.push_back(sum);
            sum += (*it);
         }
      }
      else{
         idxT sum = 0;
         for(auto it = counts.begin() ; it != counts.end() ; ++it){
            sum += (*it);
            buckets.push_back(sum);
         }
      }
   }
   
   
   
   ///checks if an index points to a LMS-char
   bool idxIsLMSC(const idxT& i, const std::vector<bool>& STypes) const {
      ///an index represents a LMS-char, if the index is of S-Type and the preceeding index is of L-Type. If i is 0, it is not a LMS-char.
      if(i){
         return STypes[i] && !STypes[i-1];
      }
      return false;
   }
   
   ///function for guessing a (not yet fully)sorted suffix array.
   std::vector<idxT> guessSortLMS(auto alph_cont, 
                                  const std::vector<bool>& STypes,
                                  std::vector<idxT> Tails, 
                                  std::unordered_map<idxT, idxT>& all_lms,
                                  const idxT& sub=0) const {
      ///the content of this array is later sorted by *LInduceSort*, *SInduceSort*, *LMSSort* and *LInduceSort*, *SInduceSort* again.
      ///induce-sorts all lms-chars from tails to head.
      std::vector<idxT> guessedSA(alph_cont.size()+1, invalid);
      idxT lms_bef = invalid;
      idxT i = 0;
      for( ; i < alph_cont.size() ; ++i){
         if(idxIsLMSC(i, STypes)){
            guessedSA[Tails[alph_cont[i]-sub]] = i;
//             std::cout << alph_cont[i]-subtract << '\t' << Tails[alph_cont[i]-subtract]<<'\n';
            --(Tails[alph_cont[i]-sub]);
            if(lms_bef != invalid){
               all_lms.insert(std::pair<idxT, idxT>(lms_bef, i));
            }
            lms_bef = i;
         }
      }
      ///modification, lms-chars/substrings are all calculated and stored for later use
      all_lms.insert(std::pair<idxT, idxT>(lms_bef, i));
      
      guessedSA[0] = alph_cont.size();
      return guessedSA;
   }
   
   ///function for one induce-sort-step of L-Type indices
   void LInduceSort(std::vector<idxT>& guessedSA,
                    auto alph_cont,
                    const std::vector<bool>& STypes,
                    std::vector<idxT>& Heads,
                                  const idxT& sub=0) const {
      ///Use buckets, to bucket-sort L-Type-indices from head to tail
      for(auto i = guessedSA.cbegin() ; i != guessedSA.cend() ; ++i){
         if((*i) != invalid){
            idxT j = (*i) - 1;
            if(j != invalid && !STypes[j]){
               guessedSA[Heads[alph_cont[j]-sub]] = j;
               ++(Heads[alph_cont[j]]);
            }
         }
      }
   }
   
   ///function for one induce-sort-step of S-Type indices
   void SInduceSort(std::vector<idxT>& guessedSA,
                    auto alph_cont,
                    const std::vector<bool>& STypes,
                    std::vector<idxT>& Tails,
                                  const idxT& sub=0) const {
      ///Use buckets, to bucket-sort S-Type-indices from tail to head
      for(auto i = guessedSA.crbegin() ; i != guessedSA.crend() ; ++i){
         if((*i) != invalid){
            idxT j = (*i) - 1;
            if(j != invalid && STypes[j]){
               guessedSA[Tails[alph_cont[j]-sub]] = j;
               --(Tails[alph_cont[j]]);
            }
         }
      }
   }

   ///function to generate a shorter representation of the input with their corresponding index
   std::vector<idxT> summarizeSA(auto alph_cont,
                                 const std::vector<idxT>& guessedSA,
                                 const std::unordered_map<idxT, idxT>& all_lms,
                                 idxT& summ_alph_s,
                                 std::vector<idxT>& summIdc) const {
      ///- *summ_alph_s*, the alphabet-size of the shorter representation is set
      ///- *summIdc*, indices pointing back are set
      ///- if *summ_alph_s* differs from the size of the returned array, the recursion(population of the Stack) continues
      ///- compares all lms-substrings for uniqueness, depending on those, the summary is generated
      ///- in worst case summary is half the size of input
      
      std::pair<idxT, idxT> lms_bef(guessedSA[0], guessedSA[0]);
      std::vector<idxT> lms(alph_cont.size() + 1, invalid);
      
      idxT cur_name = 0;
      lms[guessedSA[0]] = cur_name;
      
      std::vector<idxT> summ;
      summ.reserve(all_lms.size());
      summIdc.reserve(all_lms.size());
      
      for(idxT i = 1 ; i < guessedSA.size() ; ++i){
         auto lms_pair = all_lms.find(guessedSA[i]);
         if(lms_pair != all_lms.end()){
            if(!std::equal(alph_cont.begin() + lms_bef.first, 
                           alph_cont.begin() + lms_bef.second, 
                           alph_cont.begin() + lms_pair->first, 
                           alph_cont.begin() + lms_pair->second)){
               ++cur_name;
            }
            lms[guessedSA[i]] = cur_name;
            lms_bef = *lms_pair;
         }
      }

      for(idxT i = 0 ; i < lms.size() ; ++i){
         if(lms[i] != invalid){
            summ.push_back(lms[i]);
            summIdc.push_back(i);
         }
      }
      summ_alph_s = cur_name+1;
      return summ;
   }
   
   ///function for induce sorting lms-indices into the correct position, after population of stack ended
   std::vector<idxT> LMSSort(auto alph_cont, 
                             std::vector<idxT> Tails, 
                             const std::vector<bool>& STypes, 
                             const std::vector<idxT>& summarySA, 
                             const std::vector<idxT>& summIdc,
                                  const idxT& sub=0) const {
      
      std::vector<idxT> suffIdc(alph_cont.size()+1, invalid);
      /// Use the summary and the previously stored indices and buckets, to bucket-sort indices from tail to head
      for(idxT i = summarySA.size()-1 ; i > 1 ; --i){
         idxT idx = summIdc[summarySA[i]];
         suffIdc[Tails[alph_cont[idx]-sub]] = idx;
         --(Tails[alph_cont[idx]-sub]);
      }
      /// Sets first element of the array as the empty(smallest) suffix of the previous summary/input-array
      suffIdc[0] = alph_cont.size();
      return suffIdc;
   }
   
   ///invalid index
   const idxT invalid;
   const idxT subtract;
};
