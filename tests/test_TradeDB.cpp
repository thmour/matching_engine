//
// Created by Theofilos on 09-Jan-22.
//

#include <iostream>
#include <cassert>

#include "../include/TradeDB.hpp"

void test_insert() {
    int r;
    TradeDB tdb;
    auto entry = std::make_shared<Entry>(BUY, NONE, 0, "Company1", "User1", 50, 0.9);
    r = tdb.Insert(entry);
    assert(r == 0);
    assert(tdb.findById(entry->getEntryId()) != nullptr);
}

void test_amend() {
    int r;
    TradeDB tdb;
    auto entry = std::make_shared<Entry>(BUY, NONE, 0, "Company1", "User1", 50, 0.9);
    r = tdb.Insert(entry);
    assert(r == 0);

    r = TradeDB::Amend(entry, 40, 0.95f);
    assert(r == 0);

    assert(entry->getNumStocks() == 40);
    assert(entry->getPrice() == 0.95f);
}

void test_delete() {
    int r;
    TradeDB tdb;
    auto entry = std::make_shared<Entry>(BUY, NONE, 0, "Company1", "User1", 50, 0.9);
    r = tdb.Insert(entry);
    assert(r == 0);
    r = tdb.Delete(entry);
    assert(r == 0);
    assert(tdb.findById(entry->getEntryId()) == nullptr);
}

void test_matching_full() {
    TradeDB tdb;
    auto entry1 = std::make_shared<Entry>(BUY, NONE, 0, "Company1", "User1", 50, 0.9);
    auto entry2 = std::make_shared<Entry>(BUY, NONE, 0, "Company1", "User2", 100, 0.9);
    auto entry3 = std::make_shared<Entry>(SELL, NONE, 0, "Company1", "User3", 100, 0.9);
    tdb.ExecuteOrder(entry1);
    assert(tdb.findById(entry1->getEntryId()) != nullptr);
    tdb.ExecuteOrder(entry2);
    assert(tdb.findById(entry1->getEntryId()) != nullptr);
    tdb.ExecuteOrder(entry3);
    assert(tdb.findById(entry1->getEntryId()) != nullptr);
    assert(tdb.findById(entry2->getEntryId()) == nullptr);
    assert(tdb.findById(entry3->getEntryId()) == nullptr);
}

void test_matching_full2() {
    TradeDB tdb;
    auto entry1 = std::make_shared<Entry>(BUY, NONE, 0, "Company1", "User1", 25, 0.9);
    auto entry2 = std::make_shared<Entry>(BUY, NONE, 0, "Company1", "User2", 75, 0.9);
    auto entry3 = std::make_shared<Entry>(SELL, NONE, 0, "Company1", "User3", 100, 0.9);
    tdb.ExecuteOrder(entry1);
    assert(tdb.findById(entry1->getEntryId()) != nullptr);
    tdb.ExecuteOrder(entry2);
    assert(tdb.findById(entry1->getEntryId()) != nullptr);
    tdb.ExecuteOrder(entry3);
    assert(tdb.findById(entry1->getEntryId()) == nullptr);
    assert(tdb.findById(entry2->getEntryId()) == nullptr);
    assert(tdb.findById(entry3->getEntryId()) == nullptr);
}

void test_matching_ioc() {
    TradeDB tdb;
    auto entry1 = std::make_shared<Entry>(BUY, NONE, 0, "Company1", "User1", 25, 0.9);
    auto entry2 = std::make_shared<Entry>(SELL, IOC, 0, "Company1", "User2", 75, 1.9);
    tdb.ExecuteOrder(entry1);
    assert(tdb.findById(entry1->getEntryId()) != nullptr);
    tdb.ExecuteOrder(entry2);
    assert(tdb.findById(entry1->getEntryId()) != nullptr);
    assert(tdb.findById(entry2->getEntryId()) == nullptr);
}

int main() {
    std::cout << "Init tests" << std::endl;
    test_insert();
    test_amend();
    test_delete();
    test_matching_full();
    test_matching_full2();
    test_matching_ioc();
    std::cout << "End tests" << std::endl;

    return 0;
}