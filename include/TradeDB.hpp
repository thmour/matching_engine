//
// Created by Theofilos on 02-Jan-22.
//

#ifndef MATCHING_ENGINE_TRADEDB_HPP
#define MATCHING_ENGINE_TRADEDB_HPP

#include <map>
#include <set>
#include <string>
#include <semaphore>
#include <atomic>
#include <memory>
#include <mutex>
#include <tuple>
#include <shared_mutex>

#include "../include/Entry.hpp"

typedef std::shared_mutex Lock;
typedef std::unique_lock<Lock> WriteLock;
typedef std::shared_lock<Lock> ReadLock;

struct compareByDescPriceDescStocks
{
    bool operator()(const std::shared_ptr<Entry>& lhs, const std::shared_ptr<Entry>& rhs) const
    {
        return std::tuple(rhs->getPrice(), rhs->getNumStocks(), lhs->getEntryId()) <
               std::tuple(lhs->getPrice(), lhs->getNumStocks(), rhs->getEntryId());
    }
};

struct compareByAscPriceDescStocks
{
    bool operator()(const std::shared_ptr<Entry>& lhs, const std::shared_ptr<Entry>& rhs) const
    {
        return std::tuple(lhs->getPrice(), rhs->getNumStocks(), lhs->getEntryId()) <
               std::tuple(rhs->getPrice(), lhs->getNumStocks(), rhs->getEntryId());
    }
};

struct compareByTime
{
    bool operator()(const std::shared_ptr<Entry>& lhs, const std::shared_ptr<Entry>& rhs) const
    {
        return lhs->getEntryId() < rhs->getEntryId();
    }
};

typedef std::set<std::shared_ptr<Entry>, compareByDescPriceDescStocks> DescPriceSet;
typedef std::set<std::shared_ptr<Entry>, compareByAscPriceDescStocks> AscPriceSet;
typedef std::map<uint64_t, std::shared_ptr<Entry>> IDMap;

enum TradeCompletionType { FULL, PARTIAL };

class TradeDB {
public:
    int Insert(const std::shared_ptr<Entry>& entry);
    int Amend(std::shared_ptr<Entry> entry, uint64_t num_stocks, float price);
    int Delete(const std::shared_ptr<Entry>& entry);
    void ExecuteOrder(const std::shared_ptr<Entry>& entry);
    static void CompleteOrder(TradeCompletionType tct, const std::shared_ptr<Entry>& buy_entry,
                              const std::shared_ptr<Entry>& sell_entry);

    static std::shared_ptr<Entry> findById(uint64_t entry_id);

    void topOfTheBook(const std::string& asset_id);
    void priceDepths(const std::string& asset_id);
    void publicTradeTicks(const std::string& asset_id);

private:
    static std::atomic<uint64_t> order_counter;

    static IDMap id_map;
    static Lock id_lock;
    std::map<std::string, DescPriceSet> buy_map;
    std::map<std::string, AscPriceSet> sell_map;
};

#endif //MATCHING_ENGINE_TRADEDB_HPP
