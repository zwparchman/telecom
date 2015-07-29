
#ifndef  entry_pool_INC
#define  entry_pool_INC
#include "uvector.h"
#include <parallel/algorithm>
#include <iostream>


struct entry_pool{
  bool sorted=false;
  ao::uvector<uint64_t> tag; 
  //15 unused bits
  //next 34 bits for phone number
  ///next 5 bits for area code
  //next 7 bits for prefrences
  //next 2 bits for phone type
  //next bit for Active, deactivated

  uint64_t get_phone_number( size_t i ){
    return tag[i]>>(1+2+7+5);
  }

  uint8_t get_service_area_code( size_t i ){
    return (tag[i]>>(1+2+7)) &0b11111;
  }

  bool is_active( size_t i){
    return tag[i] & 1;
  }

  uint8_t get_prefrences(size_t i){
    return (tag[i]>>(1+2)) & 0b1111111;
  }

  uint8_t get_phone_type(size_t i){
    return (tag[i]>>1) & 0b11;
  }

  //slow, do better
  size_t getNumber_index( uint64_t goal ){
    for( size_t i=0; i<tag.size(); i++){
      if( get_phone_number(i) == goal ){
        return i;
      }
    }
    return tag.size();
  }

  bool has_number( uint64_t phoneNumber ){
    const int shift = 1+2+5+7;
    phoneNumber <<= shift;
    return true;

    /*
    return std::binary_search( 
        tag.begin(),
        tag.end(),
        []( uint64_t *a, uint64_t *b){
          return (*a>>shift) < (*b>>shift);
        });
  */
  }

  entry_pool( int to_reserve ){
    tag.reserve(to_reserve);
  }

  void add( uint64_t _phoneNumber,
      uint8_t _service_area_code,
      uint8_t _prefrences,
      char _opstype,
      uint8_t _phone_type ){

    uint64_t incoming=0;

    sorted=false;
    incoming |= _phoneNumber;
    incoming <<= 5;
    incoming |= _service_area_code;
    incoming <<= 7;
    incoming |= _prefrences;
    incoming <<= 2;
    incoming |= _phone_type;
    incoming <<= 1;
    incoming |= _opstype == 'A';

    tag.push_back( incoming );
  }

  static void merge( entry_pool &a, entry_pool &b){
    if( !a.sorted) __gnu_parallel::sort(a.tag.begin(), a.tag.end());
    if( !b.sorted) __gnu_parallel::sort(b.tag.begin(), b.tag.end());
    ao::uvector<uint64_t> c;
    c.reserve( a.tag.size() + b.tag.size());
    __gnu_parallel::merge(
        a.tag.begin(), a.tag.end(), 
        b.tag.begin(), b.tag.end(),
        c.begin());

    a.sorted=true;
    a.tag = move(c);
  }
};


#endif   /* ----- #ifndef entry_pool_INC  ----- */
