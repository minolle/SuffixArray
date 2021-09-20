#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include "SuffixArrayInducedSorting_bu0.hpp"
// #include "SuffixArrayInducedSorting.hpp"
#include "SuffixArrayNaive.hpp"
#include "parseModes.hpp"
#include "typeChecks.hpp"
#include "LCPArrayNaive.hpp"

template<typename alph=char, typename idxT=size_t>
class SuffixArray;

///find function for terminal call
template<typename alph, typename idxT>
std::pair<idxT,idxT> findInfix(const std::string& inf,
                               const std::string& data_file, 
                               const std::string& sw){
   SuffixArray<alph, idxT> SA;
   return SA.findInfixInF(inf, data_file, sw);
}


template<typename alph, typename idxT>
class SuffixArray{
public:
   
   //find constructor
   SuffixArray(){}
   
   
   SuffixArray(const Modes& modes){
      std::vector<alph> data;
      idxT min = -1;
      idxT max = 0;
      if(!read_data(modes["InFile"], 
                data, 
                parse_flag(modes["FilterNewline"]), 
                min, 
                max)){
         std::cerr << "ERROR : Failed to read input-corpus-file\n";
         exit(1);
      }
      
      if(modes["Algorithm"] == Naive){
         init<SuffixArrayNaive<alph,idxT>>
             (modes["OutFile"], 
              modes["Infix"], 
              SuffixArrayNaive<alph,
                                idxT>(false),
              parse_flag(modes["useLCP"]),
              modes["SubstringWidth"],
              parse_flag(modes["TryCompression"]),
              data,
              max);
      }
      else if(modes["Algorithm"] == NaiveParallel){
         init<SuffixArrayNaive<alph,idxT>>
             (modes["OutFile"], 
              modes["Infix"], 
              SuffixArrayNaive<alph,
                                idxT>(),
              parse_flag(modes["useLCP"]),
              modes["SubstringWidth"],
              parse_flag(modes["TryCompression"]),
              data,
              max);
      }
      else{
         init<SuffixArrayInducedSorting<alph,idxT>>
             (modes["OutFile"], 
              modes["Infix"], 
              SuffixArrayInducedSorting<alph,
                                idxT>(min),
              parse_flag(modes["useLCP"]),
              modes["SubstringWidth"],
              parse_flag(modes["TryCompression"]),
              data,
              max);
      }
   }
   /*
   SuffixArray(const std::string& f_name, 
               const std::string& out_f,
               const std::string& infix,
               auto SuffixArrayAlgorithm, 
               auto useLCP,
               const std::string& sw,
               auto c,
               auto fn){
      
      init(f_name, 
           out_f, 
           SuffixArrayAlgorithm, 
           infix, 
           parse_flag(useLCP),
           sw, 
           parse_flag(c),
           parse_flag(fn));
   }*/
   
   std::pair<idxT,idxT> findInfixInF(const std::string& infix, 
                                     const std::string& f_name,
                                     const std::string& sw) const {
      std::vector<alph> data;
      std::vector<idxT> SA;
      std::vector<std::vector<idxT>> lcp_l;
      std::vector<std::vector<idxT>> lcp_r;
      
      if(read(data, SA, lcp_l, lcp_r, f_name)){
         
         std::pair<idxT,idxT> range = find_all_infix(infix, data, SA, lcp_l, lcp_r);
         
         std::cout << "Pattern ''";
         if(range.first == SA.size()){
            std::cout << infix << "'' Not Found\n";
         }
         else{
            std::cout << infix << "'':\n";
            print_range(range, SA, data, sw);
         }
         
         return range;
      }
      else{
         std::cerr << "ERROR while reading ''" << f_name << "''\n";
         exit(1);
      }
   }
   
   
private:
   template<typename SAAlg>
   void init(const std::string out_f,
            const std::string& infix,
            SAAlg SuffixArrayAlgorithm,
            const bool& lcp,
            const std::string& sw,
            const bool& cmpr,
            const std::vector<alph>& data,
            const idxT& max) const {
      
      checkIndexType<idxT>();
      checkAlphabetType<alph>();
      checkDataSizeIdxT<idxT>(data.size());
      
      std::vector<idxT> SA = SuffixArrayAlgorithm(data, max);
      
      std::vector<std::vector<idxT>> lcp_l;
      std::vector<std::vector<idxT>> lcp_r;
      
      if(lcp){
         LCP<alph,idxT> lcpAlg;
         
         NaiveLCP_LR<alph,idxT>(SA, lcpAlg(data, SA), lcp_l, lcp_r);
      }
      
      if(infix.size()){
         std::pair<idxT,idxT> range = find_all_infix(infix, data, SA, lcp_l, lcp_r);
         
         std::cout << "Pattern ''";
         if(range.first == SA.size()){
            std::cout << infix << "'' Not Found\n";
         }
         else{
            std::cout << infix << "'':\n";
            print_range(range, SA, data, sw);
         }
      }
      else{
         write(data, SA, out_f, lcp_l, lcp_r, cmpr);
         std::cout << "Data wrote to " << out_f << '\n';
      }
      
   }
   
   std::pair<idxT,idxT> find_all_infix(const std::string& infix_in, 
                                       const std::vector<alph>& data,
                                       const std::vector<idxT>& SA,
                                       const std::vector<std::vector<idxT>>& lcp_l,
                                       const std::vector<std::vector<idxT>>& lcp_r) const {
//       auto start(std::chrono::steady_clock::now());
      std::vector<alph> infix;
      infix.reserve(infix_in.size());
      for(alph e : infix_in)
         infix.push_back(e);
      
      if(lcp_l.size()){
         std::pair<idxT,idxT> range = lower_bound(data, SA, lcp_l, lcp_r, infix);
//          std::cout<< "Only finding time : " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
         return range;
      }
      else{
         auto it = std::lower_bound(SA.begin(), 
                                    SA.end(), 
                                    infix, 
                                    CompSeq(&(*(data.begin())),
                                             &(*(data.end()))));
         idxT idx = it - SA.begin();
         if(idx == SA.size()){
//             std::cout<< "Only finding time : " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
            return std::pair<idxT, idxT>(SA.size(), SA.size());
         }
         idxT d = 0;
         for(idxT i = 0 ; i < infix.size() && SA[idx]+i < data.size()  ; ++i){
            if(data[SA[idx] + i] == infix[i])
               ++d;
            else{
//                std::cout<< "Only finding time : " << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
               return std::pair<idxT,idxT>(SA.size(),SA.size());
            }
         }
         if(d != infix.size()){
//             std::cout << "Only finding time : "<< std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
            return std::pair<idxT,idxT>(SA.size(),SA.size());
         }
         
         idxT sec = idx+1;
         for( ; sec < SA.size() ; ++sec){
            if(!equal(d, infix, data.cbegin() + SA[sec], data.cend())){
               break;
            }
         }
//          std::cout << "Only finding time : " <<std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count() << "s\n";
         return std::pair<idxT, idxT>(idx, sec);
      }
   }
   
   bool is_number(const std::string& s) const {
      auto it = s.begin();
      while(it != s.end() && std::isdigit(*it)) 
         ++it;
      return !s.empty() && it == s.end();
   }
   
   bool equal(const idxT& d, 
              const std::vector<alph>& infix, 
              typename std::vector<alph>::const_iterator data_beg, 
              typename std::vector<alph>::const_iterator data_end) const {
      idxT depth = 0;
      while(depth < d && depth < infix.size() && data_beg + depth != data_end){
         if(infix[depth] == *(data_beg+depth))
            ++depth;
         else 
            return false;
      }
      return true;
   }
   
   bool read_data(const std::string& f_name,
                  std::vector<alph>& data,
                  const bool& filt_endline,
                  idxT& min,
                  idxT& max) const {
      ///IF *filt_endline* IS SET TO TRUE && *alph* SET TO A DATA-TYPE,THAT IS SMALLER THAN UNSIGNED && LCP IS USED, THERE IS A SEGMENTATION FAULT WITH MY TEST-CORPUS!!
      
      std::ifstream stream(f_name);
      if(!stream.is_open()){
         std::cerr << "ERROR : Failed to read data\n";
         return false;
      }
//       if(filt_endline){
//          std::string buf;
//          while(std::getline(stream, buf)){
//             data.reserve(buf.size());
//             for(auto i = buf.begin() ; i != buf.end() ; ++i){
//                data.push_back(*i);
//             }
//          }
//       }
//       else{
      if(filt_endline){
         for(std::istreambuf_iterator<char> i(stream), e ; i != e ; ++i){
            if((*i) != '\n'){
               
               if((alph)(*i) < min)
                  min = (alph)(idxT)(*i);
               if((alph)(*i) > max)
                  max = (alph)(idxT)(*i);
               
               data.push_back(*i);
            }
         }
      }
      else{
         for(std::istreambuf_iterator<char> i(stream), e ; i != e ; ++i){
            if((alph)(*i) < min)
               min = (alph)(*i);
            if((alph)(*i) > max)
               max = (alph)(*i);
            
            data.push_back(*i);
         }
      }
//       }
      data.shrink_to_fit();
      stream.close();
      
      if(!data.size()){
         std::cerr << "ERROR : No data in file\n";
         return false;
      }
         
      if(data.size() < 2){
         std::cerr << "ERROR : data file empty or contains only one character\n";
         return false;
      }
      return true;
   }
   
   
   struct CompSeq{
   public:
      CompSeq(const alph* alph_cont_beg, 
              const alph* alph_cont_end)
      
      {
         cont_beg = alph_cont_beg;
         cont_end = alph_cont_end;
      }
      
      bool operator()(const idxT& SAidx, const std::vector<alph>& inf) const {
         return std::lexicographical_compare(cont_beg+SAidx, cont_end,
                                             inf.begin(), inf.end(),
                                             CompLess());
      }
      
   private:
      
      struct CompLess{
      public:
         bool operator()(const alph& cSizet, const char& c) const {
            return cSizet < (alph)c;
         }
      private:
      };
      
      const alph* cont_beg;
      const alph* cont_end;
   };
   
      
   std::string idxTToHexStr(const idxT& i, const idxT& w) const {
      std::stringstream buf;
      buf << std::setw(w) << std::setfill('0') << std::hex << i;
      return buf.str();
   }
   
   idxT HexStrTToIdxT(const std::string& hexstr) const {
      std::istringstream buf(hexstr);
      idxT idx;
      buf >> std::hex >> idx;
      return idx;
   }
   ///function binary read all pre computed data
   bool read(std::vector<alph>& data,
             std::vector<idxT>& SA,
             std::vector<std::vector<idxT>>& lcp_l,
             std::vector<std::vector<idxT>>& lcp_r,
             const std::string& f_name) const {
      std::ifstream strm_in(f_name, std::ios::binary);
      if(strm_in.is_open()){
         bool cmpr;
         strm_in.read(reinterpret_cast<char*>(&cmpr), sizeof(cmpr));
         
         idxT data_size;
         strm_in.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
         
         data.resize(data_size, alph());
         
         strm_in.read(reinterpret_cast<char*>(&data[0]), data_size*sizeof(alph));
         
         ++data_size;
         idxT max_idx_size;
         if(cmpr){
            SA.reserve(data_size);
            strm_in.read(reinterpret_cast<char*>(&max_idx_size), sizeof(max_idx_size));
            
            std::string hexstr="";
            while(strm_in >> std::setw(max_idx_size) >> hexstr){
               SA.push_back(HexStrTToIdxT(hexstr));
            }
//             SA.shrink_to_fit();
         }
         else{
            SA.resize(data_size, idxT());
            strm_in.read(reinterpret_cast<char*>(&SA[0]), (data_size) * sizeof(idxT));
         }
         read_lcp_lr(strm_in,
                    lcp_l,
                    lcp_r,
                    cmpr,
                    data_size,
                    max_idx_size);
         
         return true;
      }
      return false;
   }
   ///function read lcp-lr-array
   void read_lcp_lr(std::istream& strm_in,
                    std::vector<std::vector<idxT>>& lcp_l,
                    std::vector<std::vector<idxT>>& lcp_r,
                    const bool& cmpr,
                    const idxT& data_size,
                    const idxT& max_idx_size) const {
      bool use_lcp;
      strm_in.read(reinterpret_cast<char*>(&use_lcp), sizeof(use_lcp));
      if(use_lcp){
         
         lcp_l.reserve(data_size);
         lcp_r.reserve(data_size);
         
         idxT s;
         for(idxT i = 0 ; i < data_size ; ++i){
            if(cmpr){
               strm_in.read(reinterpret_cast<char*>(&s), sizeof(idxT));
               std::vector<idxT> l;
               l.reserve(s);
               std::string hexstr="";
               for(idxT j = 0 ; j < s ; ++j){
                  strm_in >> std::setw(max_idx_size) >> hexstr;
                  l.push_back(HexStrTToIdxT(hexstr));
               }
               lcp_l.push_back(l);
               
               strm_in.read(reinterpret_cast<char*>(&s), sizeof(idxT));
               std::vector<idxT> r;
               r.reserve(s);
               for(idxT j = 0 ; j < s ; ++j){
                  hexstr="";
                  strm_in >> std::setw(max_idx_size) >> hexstr;
                  r.push_back(HexStrTToIdxT(hexstr));
               }
               lcp_r.push_back(r);
            }
            else{
               strm_in.read(reinterpret_cast<char*>(&s), sizeof(idxT));
               std::vector<idxT> l(s);
               strm_in.read(reinterpret_cast<char*>(&(*(l.begin()))), sizeof(idxT) * s);
               lcp_l.push_back(l);
               
               strm_in.read(reinterpret_cast<char*>(&s), sizeof(idxT));
               std::vector<idxT> r(s);
               strm_in.read(reinterpret_cast<char*>(&(*(r.begin()))), sizeof(idxT) * s);
               lcp_r.push_back(r);
            }
         }
         
      }
   }
   
   ///function for binary write of data and suffix-array, optionally lcp-lr-array
   void write(const std::vector<alph>& data,
              const std::vector<idxT>& SA, 
              const std::string& f_name,
              const std::vector<std::vector<idxT>>& lcp_l,
              const std::vector<std::vector<idxT>>& lcp_r,
              const bool& cmpr=false) const {
      std::ofstream out(f_name, std::ios::binary);
      idxT data_size = data.size();
      
      out.write((char*)&cmpr, sizeof(bool));
      
      out.write((char*)&data_size, sizeof(idxT));
      
      out.write((char*)(&(*(data.begin()))), sizeof(alph) * data_size);
      
      if(cmpr){
         writeCompSA(out,
                     SA, 
                     data_size,
                     lcp_l,
                     lcp_r);
      }
      else{
         bin_writeSA(out,
                     &(*(SA.begin())), 
                     data_size,
                     lcp_l,
                     lcp_r);
      }
      out.close();
   }
   
   ///function for binary write of suffix-array
   void bin_writeSA(std::ostream& out,
                    const idxT* SA_beg,
                    const idxT& data_size,
                    const std::vector<std::vector<idxT>>& lcp_l,
                    const std::vector<std::vector<idxT>>& lcp_r) const {
      
      out.write((char*)SA_beg, sizeof(idxT) * (data_size+1));
      
      bool use_lcp = lcp_l.size() ? true : false;
      out.write((char*)&use_lcp, sizeof(bool));
      
      for(idxT i = 0 ; i < lcp_l.size() ; ++i){
         
         idxT s = lcp_l[i].size();
         out.write((char*)&s, sizeof(idxT));
         
         out.write((char*)&(*(lcp_l[i].begin())), sizeof(idxT)*s);
         
         s = lcp_r[i].size();
         out.write((char*)&s, sizeof(idxT));
         
         out.write((char*)&(*(lcp_r[i].begin())), sizeof(idxT)*s);
      }
   }
   ///function for binary write suffix-array(indices stored as hexadecimal strings)
   void writeCompSA(std::ostream& out,
                    const std::vector<idxT>& SA, 
                    const idxT& data_size,
                    const std::vector<std::vector<idxT>>& lcp_l,
                    const std::vector<std::vector<idxT>>& lcp_r) const {
      ///if *idxT* is set to *size_t*, compression works(on my system..), about 1/5 smaller
      idxT max_idx_size = idxTToHexStr(data_size, 0).size();
      out.write((char*)&max_idx_size, sizeof(idxT));
      
      for(idxT i : SA){
         out << std::setw(max_idx_size) << std::setfill('0')
             << std::hex << i;
      }
      bool use_lcp = lcp_l.size() ? true : false;
      out.write((char*)&use_lcp, sizeof(bool));
      for(idxT i = 0 ; i < lcp_l.size() ; ++i){
         
         idxT s = lcp_l[i].size();
         out.write((char*)&s, sizeof(idxT));
         
         for(idxT j : lcp_l[i]){
            out << std::setw(max_idx_size) << std::setfill('0')
               << std::hex << j;
         }
         
         s = lcp_r[i].size();
         out.write((char*)&s, sizeof(idxT));
         
         for(idxT j : lcp_r[i]){
            out << std::setw(max_idx_size) << std::setfill('0')
               << std::hex << j;
         }
         
      }
   }
   

   
   
   void print_range(const std::pair<idxT,idxT>& range, 
                    const std::vector<idxT>& SA, 
                    const std::vector<alph> data, 
                    const std::string& sw) const {
      std::cout << "Suffix-array-indices-range :\n\t("
                  << range.first << ", " <<range.second << ")\nString-indices:\n";
      
      unsigned subwidth = 0;
      if(is_number(sw)){
         subwidth = std::stoul(sw);
      }
      for(idxT i = range.first ; i < range.second ; ++i){
         std::cout << '\t' << SA[i];
         if(subwidth){
            std::cout << " :\t";
//             unsigned s = subwidth;
            for(unsigned j = 0 ; j < subwidth && SA[i]+j < data.size() ; ++j){
               if(data[SA[i]+j] == '\n')
                  std::cout << "\\n";
               else
                  std::cout << (char)data[SA[i]+j];
            }
            std::cout << '\n';
         }
         else 
            std::cout << '\n';
      }
   }
   
};















