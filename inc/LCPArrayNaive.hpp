#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>


///class for creating the lcp-array
template<typename alph, typename idxT>
class LCP{
public:
   LCP(){}
   
   std::vector<idxT> operator()(const std::vector<alph>& data,
                                const std::vector<idxT>& SA) const {
      return make_lcp(data, SA);
   }
   
private:
   ///creates the lcp-array. It computes the longest-common-prefix for neighbouring sorted substrings
   std::vector<idxT> make_lcp(const std::vector<alph>& data,
                              const std::vector<idxT>& SA) const {
      std::vector<idxT> lcp(SA.size(),0),
                        invSA = make_inverse(SA, data.size());
      ///iterates over indices of data
      ///compares two neighbouring indices of SA, getting the neighbour via the inverted suffix-array
      for(idxT i = 0, h = 0 ; i < data.size() ; ++i){
            std::equal(data.cbegin()+i+h, 
                       data.cend(), 
                       data.cbegin()+SA[invSA[i]+1]+h, 
                       Compare(&h));
            lcp[invSA[i]] = h;
            if(h) --h;
      }
      lcp.pop_back();
      return lcp;
   }
   
   ///creates inverse of suffix-array
   std::vector<idxT> make_inverse(const std::vector<idxT>& SA,
                                  const idxT& last_idx) const {
      std::vector<idxT> invSA(SA.size(), last_idx);
      for(idxT i = 1 ; i < SA.size() ; ++i){
         invSA[SA[i]] = i;
      }
      return invSA;
   }
   
   ///Compare
   struct Compare{
   public:
      Compare(idxT* i)
      : i_ptr(i)
      {}
      ///counts up, if characters are equal
      bool operator()(const alph& a, const alph& b) const {
         if(a == b){
            ++(*i_ptr);
            return true;
         }
         else return false;
      }
      idxT* i_ptr;
   private:
   };
};



///class for naively creating the LCP-LR-array(s)
template<typename alph, typename idxT>
class NaiveLCP_LR{
   ///modifies two given arrays (*lcp_l* and *lcp_r*). 
   ///At each index of *lcp_l* and *lcp_r* there is a inner-vector, 
   ///the indices of this inner-vector represent the depth of the common-prefix,
   ///inside these inner-vectors, there are the indices to which the
   ///common prefixes reach.
public:
   NaiveLCP_LR(const std::vector<idxT>& SA,
               const std::vector<idxT>& lcp,
               std::vector<std::vector<idxT>>& lcp_l,
               std::vector<std::vector<idxT>>& lcp_r){
      make_lcp_lr(SA,lcp, lcp_l, lcp_r);
   }
   
   
private:
   
   
   ///modifies *lcp_l* and *lcp_r* arrays.
   void make_lcp_lr(const std::vector<idxT>& SA,
                    const std::vector<idxT>& lcp,
                    std::vector<std::vector<idxT>>& lcp_l,
                    std::vector<std::vector<idxT>>& lcp_r) const {
      ///*lcp_l* and *lcp_r* at position 0 have no common-prefix, because it is the empty suffix
      lcp_l.resize(SA.size(), std::vector<idxT>());
      
      lcp_r.resize(SA.size(), std::vector<idxT>());
         
      lcp_l[0].reserve(1);
      lcp_l[0].push_back(0);
      lcp_r[0].reserve(1);
      lcp_r[0].push_back(0);
      
      idxT i;
      for(i = 1 ; i < SA.size() - 1; ++i){
         idxT lcp_val_r = lcp[i];
         idxT lcp_val_l = lcp[i-1];
         
         make_lcp_lr_max(i, lcp_l, lcp_r, lcp_val_l, lcp_val_r);
         
         make_lcp_r(i, lcp_r, lcp_val_r, lcp);
         
         make_lcp_l(i, lcp_l, lcp_val_l, lcp);
      }
      idxT lcp_val_l = lcp[i-1];
      lcp_l[i].resize(lcp_val_l, i);
      
      make_lcp_l(i, lcp_l, lcp_val_l, lcp);
   }
   
   void make_lcp_lr_max(const idxT& i,  
                       std::vector<std::vector<idxT>>& lcp_l,
                       std::vector<std::vector<idxT>>& lcp_r,
                       const idxT& lcp_val_l,
                       const idxT& lcp_val_r) const {
      
      lcp_l[i].resize(lcp_val_l, i-1);
      
      lcp_r[i].resize(lcp_val_r, i+1);
   }
   
   void make_lcp_r(const idxT& i,
                   std::vector<std::vector<idxT>>& lcp_r,
                   const idxT& lcp_val_r,
                   const std::vector<idxT>& lcp) const {
      idxT min = lcp_val_r;
      idxT cur_lcp_val_r;
      for(idxT j = i+1; j < lcp.size() && lcp[j] ; ++j){
         cur_lcp_val_r = lcp[j];
         if(cur_lcp_val_r < min) 
            min = cur_lcp_val_r;
         
         if(cur_lcp_val_r > lcp_val_r)
            cur_lcp_val_r = lcp_val_r;
         
         std::fill(lcp_r[i].begin(), lcp_r[i].begin() + min, j+1);
      }
   }
   
   void make_lcp_l(const idxT& i,
                   std::vector<std::vector<idxT>>& lcp_l,
                   const idxT& lcp_val_l,
                   const std::vector<idxT>& lcp) const {
      idxT min = lcp_val_l;
      idxT cur_lcp_val_l;
      for(idxT j = i-1 ; j != idxT(-1) && lcp[j] ; --j){
         cur_lcp_val_l = lcp[j];
         if(cur_lcp_val_l < min)
            min = cur_lcp_val_l;
         
         if(cur_lcp_val_l > lcp_val_l)
            cur_lcp_val_l = lcp_val_l;
         
         std::fill(lcp_l[i].begin(), lcp_l[i].begin() + min, j);
      }
   }
};

template<typename alph, typename idxT>
struct Find_if_unequal{
public:
   Find_if_unequal(idxT* idx_buf, bool* small)
   :idx(idx_buf), smaller(small)
   {}
   
   bool operator()(const alph& a, const alph& b){
      if(a < b){
         *smaller = true;
         return false;
      }
      else if(a > b){
         *smaller = false;
         return false;
      }
      else{
         ++(*idx);
         return true;
      }
   }
private:
   idxT* idx;
   bool* smaller;
};



template<typename alph, typename idxT>
std::pair<idxT,idxT> lower_bound(const std::vector<alph>& data, 
                                 const std::vector<idxT> SA, 
                                 const std::vector<std::vector<idxT>>& lcp_l,
                                 const std::vector<std::vector<idxT>>& lcp_r,
                                 const std::vector<alph>& val){
   idxT count = SA.size();
   idxT step;
   idxT depth_bef = 0;
//    bool smaller;
   bool smaller = false;
   idxT fir = 0;
   idxT i;
   
   while(count){
      i = fir;
      step = count/2;
      i += step;
      
      idxT depth = 0;
      std::equal(data.cbegin() + SA[i] + depth_bef, 
                 data.cend(), 
                 val.cbegin() + depth_bef,
                 Find_if_unequal<alph, idxT>(&depth, &smaller));
      if(depth){
         depth_bef += depth;
         if(depth_bef == val.size()){
            idxT range_l;
            idxT range_r;
            
            if(depth_bef-1 < lcp_l[i].size())
               range_l = lcp_l[i][depth_bef-1];
            else
               range_l = i;
            
            if(depth_bef-1 < lcp_r[i].size())
               range_r = lcp_r[i][depth_bef-1]+1;
            else
               range_r = i+1;
            
            return std::pair<idxT,idxT>(range_l, range_r);
            
         }
         fir = depth_bef-1 < lcp_l[i].size() ? 
               lcp_l[i][depth_bef-1] : 
               i;
         
         count = (depth_bef-1 < lcp_r[i].size() ? 
                  lcp_r[i][depth_bef-1]+1 : 
                  i + 1)
                - fir;
      }
      else{
         if(smaller){
            fir = ++i;
            count -= (step+1);
         }
         else{
            count = step;
         }
      }
   }
   return std::pair<idxT,idxT>(SA.size(),SA.size());
}