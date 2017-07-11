#include <iostream>
#include <string>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>


namespace item_indices {

enum Indices {
  kIndex,
  kType
};

// empty index type inform
struct Index {};
struct Type {};


struct IndexComparator { 
  bool operator()(const uint64_t &lhs, const uint64_t &rhs) const {
    return lhs < rhs;
  }

  bool operator()(const std::string &lhs, const std::string &rhs) const {
    return lhs < rhs;
  }
};

}  // namespace item_indices


struct Item {
  Item(const uint64_t _index, const std::string &_type)
      : index(_index), type(_type) {
  }

  uint64_t index;
  std::string type;
  uint64_t entity1;
  std::string entity2;
};


typedef boost::multi_index::tag<item_indices::Index> IndexTag;
typedef boost::multi_index::tag<item_indices::Type> TypeTag;

typedef boost::multi_index::member<Item, uint64_t, &Item::index>
    MultiContainerMemberIndex;

typedef boost::multi_index::member<Item, std::string, &Item::type>
    MultiContainerMemberType;

typedef boost::multi_index_container<
    Item, boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            IndexTag, MultiContainerMemberIndex,
            item_indices::IndexComparator>,
        boost::multi_index::ordered_unique<
            TypeTag, MultiContainerMemberType,
            item_indices::IndexComparator>
    >
> ItemMultiContainer;


typedef ItemMultiContainer::nth_index<item_indices::kIndex>::type
    ItemMultiConIndexAccessor;

typedef ItemMultiContainer::nth_index_iterator<item_indices::kIndex>::type
    ItemMultiConIndexIterator;

typedef ItemMultiContainer::nth_index<item_indices::kType>::type
    ItemMultiConTypeAccessor;

typedef ItemMultiContainer::nth_index_iterator<item_indices::kType>::type
    ItemMultiConTypeIterator;


int main() {
  ItemMultiContainer container;

  container.insert(Item(4, "type4"));
  container.insert(Item(2, "type2"));
  container.insert(Item(3, "type3"));
  container.insert(Item(1, "type0"));
  container.insert(Item(0, "type1"));

  {
    const ItemMultiConIndexAccessor &accessor1 =
        container.get<item_indices::kIndex>();
    
    ItemMultiConIndexIterator it = accessor1.begin();

    std::cout << "index access" << std::endl;
    for (; it != accessor1.end(); ++it) {
      std::cout << "index=" << it->index
                << ", type=" << it->type
                << std::endl;
    }
  }

  {
    const ItemMultiConTypeAccessor &accessor2 =
        container.get<item_indices::kType>();
    
    ItemMultiConTypeIterator it = accessor2.begin();

    std::cout << "type access" << std::endl;
    for (; it != accessor2.end(); ++it) {
      std::cout << "index=" << it->index
                << ", type=" << it->type
                << std::endl;
    }
  }

  return 0;
}