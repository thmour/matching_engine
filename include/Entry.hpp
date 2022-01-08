//
// Created by Theofilos on 03-Jan-22.
//

#ifndef MATCHING_ENGINE_ENTRY_HPP
#define MATCHING_ENGINE_ENTRY_HPP

#include <string>
#include <chrono>
#include <ostream>

enum EntryType { BUY=0, SELL=1 };
enum EntryLimitType { NONE=0, GFD=1, IOC=2 };

class Entry {
private:
    EntryType entry_type;
    EntryLimitType entry_limit_type;
    uint64_t entry_id;
    std::string asset_id;
    std::string trader_id;
    uint64_t num_stocks;
    float price;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    friend std::ostream& operator<<(std::ostream& os, const Entry& entry);
public:
    explicit Entry(EntryType entryType = BUY, EntryLimitType entryLimitType = NONE,
          uint64_t entryId = 0, std::string assetId = "", std::string traderId = "",
          uint64_t numStocks = 0, float price = 0) :
            entry_type(entryType),
            entry_limit_type(entryLimitType),
            entry_id(entryId),
            asset_id(std::move(assetId)),
            trader_id(std::move(traderId)),
            num_stocks(numStocks), price(price),
            timestamp(std::chrono::system_clock::now()) {}
    //Getters
    [[nodiscard]] EntryType getEntryType() const;
    [[nodiscard]] EntryLimitType getEntryLimitType() const;
    [[nodiscard]] const std::chrono::time_point<std::chrono::system_clock> &getTimestamp() const;
    [[nodiscard]] const std::string& getAssetId() const;
    [[nodiscard]] const std::string& getTraderId() const;
    [[nodiscard]] uint64_t getEntryId() const;
    [[nodiscard]] float getPrice() const;
    [[nodiscard]] uint64_t getNumStocks() const;

    //Setters
    void setEntryId(uint64_t entryId);
    void setNumStocks(uint64_t numStocks);
    void setPrice(float value);
};

#endif //MATCHING_ENGINE_ENTRY_HPP
