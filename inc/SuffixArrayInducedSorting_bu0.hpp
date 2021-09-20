#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "string.h"
#include <algorithm>
#include <map>
#include <unordered_map>
#include "typeChecks.hpp"


#include <chrono>

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
   ///4. *std::vector* of type *idxT* holding all counts of the current *alph_cont*
   typedef std::tuple<std::vector<idxT>,
                     std::vector<bool>,
                     std::vector<idxT>,
                     std::vector<idxT>>
           StackElement;
   ///same as StackElement with one differences,
   ///in the very first recursion call the input is a vector of type *alph*. 
   ///We do not know how the different elements of the input-vector are spread. 
   ///Within the recursion, the elements of the input-vector always start at 0 and end at the biggest element n. 
   ///All elements are represented within the range(0, n+1)
   typedef std::tuple<std::vector<alph>,
                     std::vector<bool>,
                     std::vector<idxT>,
                     std::vector<idxT>>
           FirstStackElement;
public:
   ///- *operator()*, takes a *std::vector* of type *alph* as argument
   ///- returns a *std::vector* of type *idxT* (final suffix-array)
   std::vector<idxT> operator()(const std::vector<alph> alph_cont, idxT max) const {
      ///1. the very first call of recursion, an object of type FirstStackElement is created by *toStack* of type *FirstStackElement* and stored.
      ///2. while the size of the summary differs from the size of the alphabet of the summary (every element in the summary is not unique) then the Stack is populated. 
      ///
      ///The size of the current summary and the size of the alphabet is updated in the template-function *toStack* of type *StackElement*
      
//       auto start(std::chrono::steady_clock::now());
      ++max;
      
      std::vector<idxT> new_alph_cont;
      idxT summ_s;
      idxT summ_alph_s = max;
      
      FirstStackElement FSE = toStack<FirstStackElement, alph>(alph_cont, 
                                                               new_alph_cont,
                                                               summ_s,
                                                               summ_alph_s,
                                                               subtract);
//       std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       start = std::chrono::steady_clock::now();
      std::vector<StackElement> Stack;
      std::vector<idxT> summary;
      while(summ_s != summ_alph_s){
         
         Stack.push_back(toStack<StackElement, idxT>(new_alph_cont,
                                                     summary,
                                                     summ_s,
                                                     summ_alph_s));
         new_alph_cont = summary;
      }//while
      
      ///3. If every element of the summary is unique, the population of the Stack stops
      ///and the inverse of the summary is created with *RetSummarySA*
//       std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       auto start=std::chrono::steady_clock::now();
      Stack.shrink_to_fit();
      summary = RetSummarySA(new_alph_cont);
//       std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       start =std::chrono::steady_clock::now();
      ///4. while the size of the Stack is bigger than 0 *fromStack* of type *StackElement* is called on the last element of the Stack and the summary. The summary is updated with value of *fromStack*
      while(Stack.size()){
         summary = fromStack<StackElement, idxT>(Stack.back(), summary);
         Stack.pop_back();
      }//while
//       std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
      ///5. call *fromStack* of type *FirstStackElement* on the stored object of type *FirstStackElement* and the value is returned. (final suffix-array)
      return fromStack<FirstStackElement, alph>(FSE, summary, subtract);
   }
   
   ///constructor sets the invalid index to max of idxT
   SuffixArrayInducedSorting(const idxT& min)
   : invalid(-1), subtract(min)
   {}
   
private:
   
   template<typename alph_cont_T>
   void make_counts(const std::vector<alph_cont_T>& alph_cont, std::vector<idxT>& counts, const idxT& sub) const {
      for(auto it = alph_cont.begin() ; it != alph_cont.end() ; ++it){
         ++counts[(*it) - sub];
      }
   }
   
   void make_buckets(const std::vector<idxT>& counts, std::vector<idxT>& buckets, const bool& heads) const {
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
   template<typename StackEl, typename alph_cont_T>
   ///takes the last element of the Stack and the current summary as arguments
   std::vector<idxT> fromStack(const StackEl& top, 
                               const std::vector<idxT>& summarySA,
                               const idxT& min=0) const {
      ///1. call *LMSSort* on the top of Stack
      ///2. induce-sort L-Types on result of *LMSSort* with elements of top of stack
      ///3. induce-sort R-Types on result with elements of top of stack
      ///4. return result
      
      std::vector<idxT> tails;
      make_buckets(std::get<3>(top), tails, false);
      std::vector<idxT> heads;
      make_buckets(std::get<3>(top), heads, true);
      
      std::vector<idxT> idc = LMSSort<alph_cont_T>
                                     (std::get<0>(top), 
                                      tails, 
                                      std::get<1>(top), 
                                      summarySA, 
                                      std::get<2>(top),
                                      min);

      LInduceSort(idc, std::get<0>(top), std::get<1>(top), heads, min);
      
      SInduceSort(idc, std::get<0>(top), std::get<1>(top), tails, min);

      return idc;
   }
   
   ///template-function for generating the element to push onto the Stack
   template<typename StackEl, typename alph_cont_T>
   StackEl toStack(std::vector<alph_cont_T> alph_cont, 
                   std::vector<idxT>& summary,
                   idxT& summ_s, 
                   idxT& summ_alph_s,
                   const idxT& min=0) const {
      ///- returns a 4-tuple
      ///- sets *summ_s* and *summ_alph_s* to continue or stop recursion(populating stack)
      ///1. generate boolean L-S-Type-array
      ///2. generate mapper to bucket-heads-tails
      ///3. store all lms-indices
      ///4. guess the SA
      ///5. induce sort L-types
      ///6. induce sort S-types
      ///7. make summary, set *summ_s*, set *summ_alph_s* and set indices-array pointing back to summary
      
//       auto start(std::chrono::steady_clock::now());
      std::vector<bool> STypes = make_STypes<alph_cont_T>(alph_cont);
//       std::cout << "\tmaketypes " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       start =std::chrono::steady_clock::now();
      std::vector<idxT> counts(summ_alph_s-min);
      make_counts(alph_cont, counts, min);
//       std::cout <<"\tmakecounts "<< std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       start=std::chrono::steady_clock::now();
      std::vector<idxT> tails;
      make_buckets(counts, tails, false);
//       std::cout <<"\tmakebuckets "<< std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       start=std::chrono::steady_clock::now();
//       std::unordered_map<idxT, idxT> all_lms;
//       std::vector<idxT> all_lms(alph_cont.size() + 1, 0);
//       all_lms[alph_cont.size()] = ;
      std::vector<idxT> guessedSA = guessSortLMS<alph_cont_T>
                                                (alph_cont, 
                                                STypes, 
                                                tails,
//                                                 all_lms, 
                                                min);
//       std::cout<<"\tguesssort " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       start=std::chrono::steady_clock::now();
      std::vector<idxT> heads;
      make_buckets(counts, heads, true);
//       std::cout<<"\tmakebuckets " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       start=std::chrono::steady_clock::now();
      LInduceSort<alph_cont_T>(guessedSA, alph_cont, STypes, heads, min);
//       std::cout<<"\tlinduce " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       start=std::chrono::steady_clock::now();
      SInduceSort<alph_cont_T>(guessedSA, alph_cont, STypes, tails, min);
//       std::cout<<"\tsinduce" << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
//       start=std::chrono::steady_clock::now();
      std::vector<idxT> summIdc;
      summary = summarizeSA<alph_cont_T>
                           (alph_cont, 
                           guessedSA, 
//                            all_lms, 
                           summ_alph_s,
                           summIdc,
                           STypes);
//       std::cout<<"\tsummarize" << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
      summ_s = summary.size();
      return std::make_tuple(alph_cont, STypes, summIdc, counts);
   }
   
   template<typename alph_cont_T>
   std::vector<bool> make_STypes(std::vector<alph_cont_T> alph_cont) const {
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
   
   ///checks if an index points to a LMS-char
   bool idxIsLMSC(const idxT& i, const std::vector<bool>& STypes) const {
      ///an index represents a LMS-char, if the index is of S-Type and the preceeding index is of L-Type. 0 is never tested.
      return STypes[i] && !STypes[i-1];
   }
   
   ///function for guessing a (not yet fully)sorted suffix array.
   template <typename alph_cont_T>
   std::vector<idxT> guessSortLMS(std::vector<alph_cont_T> alph_cont, 
                                  const std::vector<bool>& STypes,
                                  std::vector<idxT> Tails, 
//                                   std::unordered_map<idxT, idxT>& all_lms,
                                  const idxT& sub) const {
      ///the content of this array is later sorted by *LInduceSort*, *SInduceSort*, *LMSSort* and *LInduceSort*, *SInduceSort* again.
      ///induce-sorts all lms-chars from tails to head.
      std::vector<idxT> guessedSA(alph_cont.size()+1, invalid);
//       idxT lms_bef = invalid;
      idxT i = 1;
      for( ; i < alph_cont.size() ; ++i){
         if(idxIsLMSC(i, STypes)){
            guessedSA[Tails[alph_cont[i] - sub]] = i;
            --(Tails[alph_cont[i] - sub]);
//             if(lms_bef != invalid){
//                all_lms.insert(std::pair<idxT, idxT>(lms_bef, i));
//             }
//             lms_bef = i;
         }
      }
      ///modification, lms-chars/substrings are all calculated and stored for later use
//       all_lms.insert(std::pair<idxT, idxT>(lms_bef, i));
      /*
      std::cout << "LMS---\n";
      for(auto it = all_lms.begin() ; it != all_lms.end() ; ++it){
         std::cout << it->first << ',' << it->second<<'\n';
      }
      std::cout << "------\n";*/
      guessedSA[0] = alph_cont.size();
      return guessedSA;
   }
   
   ///function for one induce-sort-step of L-Type indices
   template <typename alph_cont_T>
   void LInduceSort(std::vector<idxT>& guessedSA,
                    std::vector<alph_cont_T> alph_cont,
                    const std::vector<bool>& STypes,
                    std::vector<idxT>& Heads,
                    const idxT& sub) const {
      ///Use buckets, to bucket-sort L-Type-indices from head to tail
      for(auto i = guessedSA.cbegin() ; i != guessedSA.cend() ; ++i){
         if((*i) != invalid){
            idxT j = (*i) - 1;
            if(j != invalid && !STypes[j]){
               guessedSA[Heads[alph_cont[j] - sub]] = j;
               ++(Heads[alph_cont[j] - sub]);
            }
         }
      }
   }
   
   ///function for one induce-sort-step of S-Type indices
   template <typename alph_cont_T>
   void SInduceSort(std::vector<idxT>& guessedSA,
                    std::vector<alph_cont_T> alph_cont,
                    const std::vector<bool>& STypes,
                    std::vector<idxT>& Tails,
                    const idxT& sub) const {
      ///Use buckets, to bucket-sort S-Type-indices from tail to head
      for(auto i = guessedSA.crbegin() ; i != guessedSA.crend() ; ++i){
         if((*i) != invalid){
            idxT j = (*i) - 1;
            if(j != invalid && STypes[j]){
               guessedSA[Tails[alph_cont[j] - sub]] = j;
               --(Tails[alph_cont[j] - sub]);
            }
         }
      }
   }

   ///function to generate a shorter representation of the input with their corresponding index
   template <typename alph_cont_T>
   std::vector<idxT> summarizeSA(std::vector<alph_cont_T> alph_cont,
                                 const std::vector<idxT>& guessedSA,
//                                  const std::unordered_map<idxT, idxT>& all_lms,
                                 idxT& summ_alph_s,
                                 std::vector<idxT>& summIdc, 
                                 const std::vector<bool>& STypes) const {
      ///- *summ_alph_s*, the alphabet-size of the shorter representation is set
      ///- *summIdc*, indices pointing back are set
      ///- if *summ_alph_s* differs from the size of the returned array, the recursion(population of the Stack) continues
      ///- compares all lms-substrings for uniqueness, depending on those, the summary is generated
      ///- in worst case summary is half the size of input
      
      std::pair<idxT, idxT> lms_bef(guessedSA[0], guessedSA[0]);
      std::vector<idxT> lms(alph_cont.size() + 1, invalid);
      
      idxT cur_name = 0;
      lms[guessedSA[0]] = cur_name;
      
      std::vector<idxT> all_lms(alph_cont.size()+1, 0);
      idxT alllms_bef = alph_cont.size();
      for(idxT i = alph_cont.size() - 1 ; i ; --i){
         if(idxIsLMSC(i, STypes)){
            all_lms[i] = alllms_bef;
            alllms_bef = i;
         }
      }
      
      idxT lms_bef_fir = guessedSA[0], lms_bef_sec = guessedSA[0];
      
      std::vector<idxT> summ;
      summ.reserve(all_lms.size());
      summIdc.reserve(all_lms.size());
      
      for(idxT i = 1 ; i < guessedSA.size() ; ++i){
//          auto lms_pair = all_lms.find(guessedSA[i]);
         idxT lms_i = guessedSA[i];
         if(all_lms[guessedSA[i]]){
            if(!std::equal(alph_cont.begin() + lms_bef_fir, 
                           alph_cont.begin() + lms_bef_sec, 
                           alph_cont.begin() + lms_i, 
                           alph_cont.begin() + all_lms[lms_i])){
               ++cur_name;
            }
            lms[guessedSA[i]] = cur_name;
            lms_bef_fir = lms_i;
            lms_bef_sec = all_lms[lms_i];
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
   template <typename alph_cont_T>
   std::vector<idxT> LMSSort(std::vector<alph_cont_T> alph_cont, 
                             std::vector<idxT> Tails, 
                             const std::vector<bool>& STypes, 
                             const std::vector<idxT>& summarySA, 
                             const std::vector<idxT>& summIdc,
                             const idxT& sub) const {
      
      std::vector<idxT> suffIdc(alph_cont.size()+1, invalid);
      /// Use the summary and the previously stored indices and buckets, to bucket-sort indices from tail to head
      for(idxT i = summarySA.size()-1 ; i > 1 ; --i){
         idxT idx = summIdc[summarySA[i]];
         suffIdc[Tails[alph_cont[idx] - sub]] = idx;
         --(Tails[alph_cont[idx] - sub]);
      }
      /// Sets first element of the array as the empty(smallest) suffix of the previous summary/input-array
      suffIdc[0] = alph_cont.size();
      return suffIdc;
   }
   
   ///invalid index
   const idxT invalid;
   const idxT subtract;
};
