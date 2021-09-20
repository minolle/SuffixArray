#pragma once
#include <type_traits>

///checks if indexType is an unsigned-type
template<typename idxT>
void checkIndexType(){
   if(std::is_signed<idxT>()){
      std::cerr << "ERROR : Index Type must not be signed!\n";
      exit(1);
   }
}
///checks if AlphabetType is an unsigned-type
template<typename alph>
void checkAlphabetType(){
   if(std::is_signed<alph>()){
      std::cerr << "ERROR : Alphabet Type must not be signed!\n";
      exit(1);
   }
}

///suffix-array default index-type set to unsigned, checks if the biggest index (input-data-byte-count) is smaller than the ''invalid'' index(max of index-type).

template<typename idxT>
void checkDataSizeIdxT(idxT data_size){
   if(data_size >= idxT(-1)){
      std::cerr << "Input has too many positions for index-type(idxT)!!!\n";
   }
}


