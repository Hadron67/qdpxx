// -*- C++ -*-
/*! \file
 *  \brief A memory based map object 
 */

#ifndef __qdp_map_obj_memory_h__
#define __qdp_map_obj_memory_h__

#include "qdp_map_obj.h"
#include <vector>
#include <tr1/unordered_map>

namespace QDP
{

  //----------------------------------------------------------------------------
  //! A wrapper over maps
  template<typename K, typename V>
  class MapObjectMemory : public MapObject<K,V>
  {
  public:
    //! Default constructor
    MapObjectMemory() {}

    //! Destructor
    ~MapObjectMemory() {}

    //! Insert user data into the metadata database
    int insertUserdata(const std::string& user_data_) {
      user_data = user_data_;
      return 0;
    }
    
    //! Get user user data from the metadata database
    int getUserdata(std::string& user_data_) const {
      user_data_ = user_data;
      return 0;
    }

    //! Insert
    int insert(const K& key, const V& val) {
      int ret = 0;
      BinaryBufferWriter bin;
      write(bin, key);

      const std::string bin_key(bin.str());
      typename MapType_t::iterator iter = src_map.find(bin_key);
      if (iter != src_map.end())
      {
	iter->second = val;
      }
      else
      {
	src_map.insert(std::make_pair(bin_key,val));
      }
      return ret;
    }
			
    //! Getter
    int get(const K& key, V& val) const {
      int ret = 0;
      BinaryBufferWriter bin;
      write(bin, key);

      typename MapType_t::const_iterator iter = src_map.find(bin.str());
      if (iter == src_map.end()) {
	ret = 1;
      }
      else {
	val = iter->second;
      }
      return ret;
    }
			
    //! Erase a key-value
    void erase(const K& key) {
      BinaryBufferWriter bin;
      write(bin, key);

      typename MapType_t::const_iterator iter = src_map.find(bin.str());
      if (iter != src_map.end())
      {
	src_map.erase(iter);
      }
    }
			
    //! Clear the object
    void clear() {src_map.clear();}

    //! Flush out state of object
    void flush() {}

    //! Exists?
    bool exist(const K& key) const {
      BinaryBufferWriter bin;
      write(bin, key);
      return (src_map.find(bin.str()) == src_map.end()) ? false : true;
    }
			
    //! The number of elements
//    size_t size() const {return src_map.size();}
    unsigned int size() const {return static_cast<unsigned long>(src_map.size());}

    //! Dump keys
    void keys(std::vector<K>& _keys) const {
      _keys.resize(0);
      typename MapType_t::const_iterator iter;
      for(iter  = src_map.begin();
	  iter != src_map.end();
	  ++iter)
      {
	BinaryBufferReader bin(iter->first);
	K key;
	read(bin, key);
	_keys.push_back(key);
      }
    }

    //! Dump keys and values
    virtual void keysAndValues(std::vector<K>& _keys, std::vector<V>& _vals) const {
      _keys.resize(0);
      _vals.resize(0);
      typename MapType_t::const_iterator iter;
      for(iter  = src_map.begin();
	  iter != src_map.end();
	  ++iter)
      {
	BinaryBufferReader bin(iter->first);
	K key;
	read(bin, key);
	_keys.push_back(key);
	_vals.push_back(iter->second);
      }
    }

    // Extensions to the basic MapObject
    //! Getter
    const V& operator[](const K& key) const {
      BinaryBufferWriter bin;
      write(bin, key);

      typename MapType_t::const_iterator iter = src_map.find(bin.str());
      if (iter == src_map.end())
      {
	std::cerr << "MapObject: key not found" << std::endl;
	exit(1);
      }

      return iter->second;
    }
			
    //! Setter
    V& operator[](const K& key) {
      BinaryBufferWriter bin;
      write(bin, key);

      typename MapType_t::iterator iter = src_map.find(bin.str());
      if (iter == src_map.end())
      {
	std::cerr << "MapObject: key not found" << std::endl;
	exit(1);
      }

      return iter->second;
    }
			
  protected:  
    //! Map type convenience
    typedef std::tr1::unordered_map<std::string, V> MapType_t;
    
    //! Map of objects
    mutable MapType_t src_map;
    std::string user_data;
  };



  //----------------------------------------------------------------------------
  //! Read a MapObject via xml
  template<typename K, typename V>
  inline
  void read(XMLReader& xml, const std::string& s, MapObjectMemory<K,V>& input)
  {
    XMLReader arraytop(xml, s);

    std::ostringstream error_message;
    std::string elemName = "elem";
  
    int array_size;
    try {
      array_size = arraytop.count(elemName);
    }
    catch( const std::string& e) { 
      error_message << "Exception occurred while counting " << elemName
		    << " during array read " << s << std::endl;
      arraytop.close();
      throw error_message.str();
    }
      
    // Get the elements one by one
    for(int i=0; i < array_size; i++) 
    {
      std::ostringstream element_xpath;

      // Create the query for the element 
      element_xpath << elemName << "[" << (i+1) << "]";

      // recursively try and read the element.
      try {
	XMLReader xml_elem(arraytop, element_xpath.str());

	K key;
	V val;

	read(xml_elem, std::string("Key"), key);
	read(xml_elem, std::string("Val"), val);

	input.insert(key, val);
      } 
      catch (const std::string& e) 
      {
	error_message << "Failed to match element " << i
		      << " of array  " << s << "  with query " << element_xpath.str()
		      << std::endl
		      << "Query returned error: " << e;
	arraytop.close();
	throw error_message.str();
      }
    }

    // Arraytop should self destruct but just to be sure.
    arraytop.close();
  }


  //----------------------------------------------------------------------------
  //! Write a MapObject in xml
  template<typename K, typename V>
  inline
  void write(XMLWriter& xml, const std::string& path, const MapObjectMemory<K,V>& param)
  {
    push(xml, path);

    std::vector<K>  k1;
    std::vector<V>  v1;
    param.keysAndValues(k1,v1);

    for(int i=0; i < k1.size(); ++i)
    {
      push(xml, "elem");
      write(xml, "Key", k1[i]);
      write(xml, "Val", v1[i]);
      pop(xml);
    }

    pop(xml);
  }


} // namespace QDP

#endif
