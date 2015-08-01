
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

  uint64_t get_phone_number( size_t i );

  uint8_t get_service_area_code( size_t i );

  bool is_active( size_t i);

  uint8_t get_prefrences(size_t i);

  uint8_t get_phone_type(size_t i);

  //slow, do better
  size_t getNumber_index( uint64_t goal );

  bool has_number( uint64_t phoneNumber );

  void self_sort();

  entry_pool( size_t to_reserve );

  void add( uint64_t _phoneNumber,
      uint8_t _service_area_code,
      uint8_t _prefrences,
      char _opstype,
      uint8_t _phone_type );

  char get_ops_type(size_t i);

  size_t size() const ;

  static std::string prefrences_to_string( uint8_t pref);

  static void merge( entry_pool &a, entry_pool &b);
};


#endif   /* ----- #ifndef entry_pool_INC  ----- */
