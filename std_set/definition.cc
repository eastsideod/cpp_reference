#include <iostream>
#include <memory>
#include <set>
#include <string>  // for string operator <<
#include <algorithm> // for std::next, std::advance

// definitnion ADT and typedef;

struct Item {
  Item(int _index, const std::string &_name) : index(_index), name(_name) {}

  const int index;
  const std::string name;
};

typedef std::shared_ptr<Item> ItemPtr;


struct ItemComparator {
  bool operator() (const Item &lhs, const Item &rhs) const {
    return lhs.index < rhs.index;
  }

  bool operator() (const ItemPtr &lhs, const ItemPtr &rhs) const {
    return lhs->index < rhs->index;
  }

};


int main() {

  std::set<Item, ItemComparator> items = {
      Item(2, "foo2"), Item(3, "foo3"), Item(0, "foo0"), Item(1, "foo2"),
      Item(6, "foo6"), Item(7, "foo7"), Item(4, "foo4"), Item(5, "foo5")};

  std::set<ItemPtr, ItemComparator> ptr_items = {
      std::make_shared<Item>(2, "foo2"), std::make_shared<Item>(3, "foo3"),
      std::make_shared<Item>(0, "foo0"), std::make_shared<Item>(1, "foo2")};

  for (const auto &item: items) {
    std::cout << "index=" << item.index << ", name=" << item.name << std::endl;
  }

  std::cout << "ptr items" << std::endl;
  for (const auto &item: ptr_items) {
    std::cout << "index=" << item->index
              << ", name=" << item->name << std::endl;
  }

  // copy all.
  std::cout << "copy items" << std::endl;
  std::set<Item, ItemComparator> items3(items.begin(), items.end());
    for (const auto &item: items3) {
    std::cout << "index=" << item.index << ", name=" << item.name << std::endl;
  }

  //copy specific range.
  std::cout << "copy range" << std::endl;
  
  
  // same as
  //std::set<Item, ItemComparator>::iterator range = items.begin();
  //std::advance(range, 4);
  //std::set<Item, ItemComparator> items4(
  //    items.begin(), range);
  
  std::set<Item, ItemComparator> items4(
      items.begin(), std::next(items.begin(), 4));
  
  for (const auto &item: items4) {
    std::cout << "index=" << item.index << ", name=" << item.name << std::endl;
  }
}