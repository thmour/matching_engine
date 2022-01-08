//
// Created by Theofilos on 03-Jan-22.
//

#include "../include/Entry.hpp"

#include <utility>

const std::chrono::time_point<std::chrono::system_clock>& Entry::getTimestamp() const {
    return timestamp;
}

float Entry::getPrice() const {
    return price;
}

const std::string& Entry::getAssetId() const {
    return asset_id;
}

const std::string& Entry::getTraderId() const {
    return trader_id;
}

uint64_t Entry::getNumStocks() const {
    return num_stocks;
}

uint64_t Entry::getEntryId() const {
    return entry_id;
}

EntryType Entry::getEntryType() const {
    return entry_type;
}

EntryLimitType Entry::getEntryLimitType() const {
    return entry_limit_type;
}

void Entry::setEntryId(uint64_t entryId) {
    entry_id = entryId;
}

void Entry::setNumStocks(uint64_t numStocks) {
    num_stocks = numStocks;
}

void Entry::setPrice(float value) {
    price = value;
}

std::ostream& operator<<(std::ostream& os, const Entry& entry)
{
    os << "(" << (entry.getEntryType() == 0 ? "Buy" : "Sell") << ", "
       << (entry.getEntryLimitType() == 1 ? "Limit, " : "") << " "
       << "Order ID: " << entry.getEntryId() << ", "
       << "Asset: " << entry.getAssetId() << ", "
       << "Trader: " << entry.getTraderId() << ", "
       << "Number of stocks: " << entry.getNumStocks() << ", "
       << "Price: " << entry.getPrice() << ", "
       << "Timestamp: " << entry.getTimestamp().time_since_epoch().count() << ")";
    return os;
}
