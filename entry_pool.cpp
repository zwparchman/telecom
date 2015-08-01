#include "entry_pool.h"
#include <bitset>
uint64_t entry_pool::get_phone_number( size_t i ){
  return tag[i]>>(1+2+7+5);
}

uint8_t entry_pool::get_service_area_code( size_t i ){
  return (tag[i]>>(1+2+7)) &0b11111;
}

bool entry_pool::is_active( size_t i){
  return tag[i] & 1;
}

uint8_t entry_pool::get_prefrences(size_t i){
  uint8_t ret= (tag[i]>>(1+2) & 0b111'1111);
  return ret;
}

uint8_t entry_pool::get_phone_type(size_t i){
  return (tag[i]>>1) & 0b11;
}

void entry_pool::self_sort(){
  sorted = true;
  __gnu_parallel::sort(tag.begin(), tag.end());
}

size_t entry_pool::getNumber_index( uint64_t goal ){
  if( !sorted ) self_sort();

  const int shift = 1+2+7+5;
  auto where = std::lower_bound( tag.begin(), tag.end(), goal << shift);

  if(where == tag.end()) return size();

  size_t ret = where - tag.begin();

  if( get_phone_number( ret) == goal ){
    return ret;
  } else {
    return size();
  }

}

bool entry_pool::has_number( uint64_t phoneNumber ){
  return getNumber_index(phoneNumber) != size();
}

entry_pool::entry_pool( size_t to_reserve ){
  tag.reserve(to_reserve);
}

void entry_pool::add( uint64_t _phoneNumber,
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

char entry_pool::get_ops_type(size_t i){
  return ( (tag[i] & 1)?'A':'D');
}

size_t entry_pool::size() const {
  return tag.size();
}

std::string entry_pool::prefrences_to_string( uint8_t pref){
  const std::bitset<8> set(pref);
  if( pref == 0 ){
    return "";
  }

  std::cout<<set.to_string()<<std::endl;

  std::string ret="";
  for( int i=0; i<7; i++){
    if( set[i] ){
      ret+=std::to_string(1+i)+"#";
    }
  }
  ret.erase( ret.end()-1, ret.end());

  return ret;
}


void entry_pool::merge( entry_pool &a, entry_pool &b){
  if( !a.sorted) a.self_sort();
  if( !b.sorted) b.self_sort();
  ao::uvector<uint64_t> c;
  c.resize( a.tag.size() + b.tag.size());
  __gnu_parallel::merge(
      a.tag.begin(), a.tag.end(), 
      b.tag.begin(), b.tag.end(),
      c.begin());

  a.sorted=true;
  a.tag = move(c);
}
