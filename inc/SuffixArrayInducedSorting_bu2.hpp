#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "string.h"
#include <algorithm>
#include <map>
#include <unordered_map>
#include "typeChecks.hpp"

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
   ///4. *std::vector* of type *std::pair* of types *idxT*, mapping the indices of 1. to heads and tails of buckets
   typedef std::tuple<std::vector<idxT>,
                     std::vector<bool>,
                     std::vector<idxT>,
                     std::vector<std::pair<idxT,idxT>>>
           StackElement;
   ///same as StackElement with two differences,
   ///in the very first recursion call the input is a vector of type *alph*. 
   ///We do not know how the different elements of the input-vector are spread. 
   ///Within the recursion, the elements of the input-vector always start at 0 and end at the biggest element n. 
   ///All elements are represented within the range(0, n+1), which is not the case in the very first call.
   ///So the first element of the tuple is a *std::vector* of type *alph* and the fourth element of the tuple is a *std::unordered_map* of type *alph* mapping to type *std::pair* of types *idxT*
   typedef std::tuple<std::vector<alph>,
                     std::vector<bool>,
                     std::vector<idxT>,
                     std::unordered_map<alph, std::pair<idxT,idxT>>>
           FirstStackElement;
public:
   ///- *operator()*, takes a *std::vector* of type *alph* as argument
   ///- returns a *std::vector* of type *idxT* (final suffix-array)
   std::vector<idxT> operator()(const std::vector<alph> alph_cont,const idxT& _) const {
      ///1. the very first call of recursion, an object of type FirstStackElement is created by *toStack* of type *FirstStackElement* and stored.
      ///2. while the size of the summary differs from the size of the alphabet of the summary (every element in the summary is not unique) then the Stack is populated. 
      ///
      ///The size of the current summary and the size of the alphabet is updated in the template-function *toStack* of type *StackElement*
      
      std::vector<idxT> new_alph_cont;
      idxT summ_s;
      idxT summ_alph_s;
      
      std::cout << "summ_alph_s : " << summ_alph_s<< '\n';
      for(auto it = alph_cont.begin() ; it != alph_cont.end() ; ++it){
         std::cout << *it <<',';
      }
      std::cout << '\n';
      
      FirstStackElement FSE = toStack<FirstStackElement>
                                       (alph_cont, 
                                       makeHeadsTailsFirst(),
                                       new_alph_cont,
                                       summ_s,
                                       summ_alph_s);
      std::vector<StackElement> Stack;
      std::vector<idxT> summary;
      while(summ_s != summ_alph_s){
         std::cout << "summ_alph_s : " << summ_alph_s<<'\n';
         for(auto it = new_alph_cont.begin() ; it != new_alph_cont.end() ; ++it){
            std::cout << *it <<',';
         }
         std::cout << '\n';
         
         Stack.push_back(toStack<StackElement>(new_alph_cont,
                                                makeHeadsTailsSecond(),
                                                summary,
                                                summ_s,
                                                summ_alph_s));
         new_alph_cont = summary;
      }//while
      
      ///3. If every element of the summary is unique, the population of the Stack stops
      ///and the inverse of the summary is created with *RetSummarySA*
      Stack.shrink_to_fit();
      summary = RetSummarySA(new_alph_cont);
      
      ///4. while the size of the Stack is bigger than 0 *fromStack* of type *StackElement* is called on the last element of the Stack and the summary. The summary is updated with value of *fromStack*
      while(Stack.size()){
         summary = fromStack<StackElement>(Stack.back(), summary);
         Stack.pop_back();
      }//while
      
      ///5. call *fromStack* of type *FirstStackElement* on the stored object of type *FirstStackElement* and the value is returned. (final suffix-array)
      return fromStack<FirstStackElement>(FSE, summary);
   }
   
   ///constructor sets the invalid index to max of idxT
   SuffixArrayInducedSorting(const idxT& _)
   : invalid(-1)
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
   template<typename StackEl>
   ///takes the last element of the Stack and the current summary as arguments
   std::vector<idxT> fromStack(const StackEl& top, 
                               const std::vector<idxT>& summarySA) const {
      ///1. call *LMSSort* on the top of Stack
      ///2. induce-sort L-Types on result of *LMSSort* with elements of top of stack
      ///3. induce-sort R-Types on result with elements of top of stack
      ///4. return result
      std::vector<idxT> idc = LMSSort(std::get<0>(top), 
                                      std::get<3>(top), 
                                      std::get<1>(top), 
                                      summarySA, 
                                      std::get<2>(top));

      LInduceSort(idc, std::get<0>(top), std::get<1>(top), std::get<3>(top));
      
      SInduceSort(idc, std::get<0>(top), std::get<1>(top), std::get<3>(top));

      return idc;
   }
   
   ///template-function for generating the element to push onto the Stack
   template<typename StackEl>
   StackEl toStack(auto alph_cont, 
                   auto HeadsTailsFO, 
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
      std::vector<bool> STypes = make_STypes(alph_cont);
      
      auto bucketHeadsTails = HeadsTailsFO(alph_cont);
      
      std::unordered_map<idxT, idxT> all_lms;
      
      std::vector<idxT> guessedSA = guessSortLMS(alph_cont, 
                                                STypes, 
                                                bucketHeadsTails,
                                                all_lms);
      
      LInduceSort(guessedSA, alph_cont, STypes, bucketHeadsTails);

      SInduceSort(guessedSA, alph_cont, STypes, bucketHeadsTails);

      std::vector<idxT> summIdc;
      summary = summarizeSA(alph_cont, 
                           guessedSA, 
                           all_lms, 
                           summ_alph_s,
                           summIdc);
      summ_s = summary.size();
      return std::make_tuple(alph_cont, STypes, summIdc, bucketHeadsTails);
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
   
   ///function-object for generating map for bucket-sort in the first step of recursion (using Stack)
   struct makeHeadsTailsFirst{
   public:
      ///- *operator()*, takes the input-vector, a *std::vector* of type *alph* as argument
      ///- uses a *std::map<alph,idxT>* to generate and return a *std::unordered_map<alph,std::pair<idxTidxT>>*
      ///- return val maps elements of type *alph* to head-index and tail-index of the "*alph*-bucket"
      std::unordered_map<alph,std::pair<idxT,idxT>> operator()(std::vector<alph> alph_cont) const {
         std::map<alph,idxT> sizes = getBucketSizesFirst(alph_cont);
         
         std::unordered_map<alph,std::pair<idxT,idxT>> headstails(sizes.size());
         
         idxT buf = 1;
         for(auto k_v = sizes.cbegin() ; k_v != sizes.cend(); ++k_v){
   //          std::cout << "inserting : " << k_v->first <<'\n';
            std::pair<idxT,idxT> val(buf, (buf+(k_v->second)-1));
            buf += k_v->second;
            headstails.insert(std::pair<alph, 
                                       std::pair<idxT,idxT>>(k_v->first, val));
         }
         return headstails;
      }
   private:
      ///- every element of type *alph* is is counted in a *std::map<alph,idxT>*
      ///- *std::map* is used, because the elements of type *alph* must be sorted, to get the exact head-tail-indices of the buckets. Only used once in whole algorithm
      std::map<alph, idxT> getBucketSizesFirst(std::vector<alph> alph_cont) const {
         std::map<alph, idxT> mp;
         for(alph e : alph_cont){
            auto entry = mp.find(e);
            if(entry == mp.end()){
               mp.insert(std::pair<alph, idxT>(e, 1));
            }
            else{
               ++(entry->second);
            }
         }
         return mp;
      }
   };
   
   ///function-object for generating map(as *std::vector*) for bucket-sort in every consecutive step of recursion (using Stack)
   struct makeHeadsTailsSecond{
   public:
      ///- *operator()*, takes the input-vector, a *std::vector* of type *alph* as argument
      ///uses a *std::map<alph,idxT>* to generate and return a *std::vector<std::pair<idxTidxT>>*
      ///- return val maps elements of type *idxT* to head-index and tail-index of the "*idxT*-bucket"
      std::vector<std::pair<idxT,idxT>> operator()(const std::vector<idxT>& alph_cont) const {
         std::vector<idxT> sizes = getBucketSizesSecond(alph_cont);
         std::vector<std::pair<idxT,idxT>> headstails;
         headstails.reserve(sizes.size());
         
         idxT buf = 1;
         for(idxT s : sizes){
            headstails.push_back(std::pair<idxT, idxT>(buf, (buf+s)-1));
            buf += s;
         }
         return headstails;
      }
   private:
      ///- every element of type *idxT* is counted in a *std::vector<idxT>*
      ///- this is possible because every element in the input *alph_cont* is inside a range starting at 0.
      ///- every element in the range is used at least once because of *summarizeSA*
      std::vector<idxT> getBucketSizesSecond(const std::vector<idxT>& alph_cont) const {
         std::vector<idxT> vec;
         for(idxT i : alph_cont){
            if(i < vec.size()){
               ++vec[i];
            }
            else{
               vec.resize(i+1, 0);
               ++vec[i];
            }
         }
         return vec;
      }
   };
   
   
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
                                  auto HeadsTails, 
                                  std::unordered_map<idxT, idxT>& all_lms) const {
      ///the content of this array is later sorted by *LInduceSort*, *SInduceSort*, *LMSSort* and *LInduceSort*, *SInduceSort* again.
      ///induce-sorts all lms-chars from tails to head.
      std::vector<idxT> guessedSA(alph_cont.size()+1, invalid);
      idxT lms_bef = invalid;
      idxT i = 0;
      for( ; i < alph_cont.size() ; ++i){
         if(idxIsLMSC(i, STypes)){
            guessedSA[HeadsTails[alph_cont[i]].second] = i;
            --(HeadsTails[alph_cont[i]].second);
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
                    auto HeadsTails) const {
      ///Use buckets, to bucket-sort L-Type-indices from head to tail
      for(auto i = guessedSA.cbegin() ; i != guessedSA.cend() ; ++i){
         if((*i) != invalid){
            idxT j = (*i) - 1;
            if(j != invalid && !STypes[j]){
               guessedSA[HeadsTails[alph_cont[j]].first] = j;
               ++(HeadsTails[alph_cont[j]].first);
            }
         }
      }
   }
   
   ///function for one induce-sort-step of S-Type indices
   void SInduceSort(std::vector<idxT>& guessedSA,
                    auto alph_cont,
                    const std::vector<bool>& STypes,
                    auto HeadsTails) const {
      ///Use buckets, to bucket-sort S-Type-indices from tail to head
      for(auto i = guessedSA.crbegin() ; i != guessedSA.crend() ; ++i){
         if((*i) != invalid){
            idxT j = (*i) - 1;
            if(j != invalid && STypes[j]){
               guessedSA[HeadsTails[alph_cont[j]].second] = j;
               --(HeadsTails[alph_cont[j]].second);
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
                             auto HeadsTails, 
                             const std::vector<bool>& STypes, 
                             const std::vector<idxT>& summarySA, 
                             const std::vector<idxT>& summIdc) const {
      
      std::vector<idxT> suffIdc(alph_cont.size()+1, invalid);
      /// Use the summary and the previously stored indices and buckets, to bucket-sort indices from tail to head
      for(idxT i = summarySA.size()-1 ; i > 1 ; --i){
         idxT idx = summIdc[summarySA[i]];
         suffIdc[HeadsTails[alph_cont[idx]].second] = idx;
         --(HeadsTails[alph_cont[idx]].second);
      }
      /// Sets first element of the array as the empty(smallest) suffix of the previous summary/input-array
      suffIdc[0] = alph_cont.size();
      return suffIdc;
   }
   
   ///invalid index
   const idxT invalid;
};
