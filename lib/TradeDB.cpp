//
// Created by Theofilos on 02-Jan-22.
//

#include "../include/TradeDB.hpp"
#include <cerrno>
#include <ranges>
#include <tuple>
#include <iostream>
#include <ranges>

IDMap TradeDB::id_map;
Lock TradeDB::id_lock;
std::atomic<uint64_t> TradeDB::order_counter = 1;

int TradeDB::Insert(const std::shared_ptr<Entry>& entry) {
    switch (entry->getEntryType()) {
        case BUY:
            {
                if (entry->getEntryId() == 0)
                    entry->setEntryId(++order_counter);

                auto it = buy_map.find(entry->getAssetId());
                if (it == buy_map.end()) {
                    std::tie(it, std::ignore) = buy_map.emplace(entry->getAssetId(), DescPriceSet());
                }
                it->second.insert(entry);

                WriteLock wi_lock(id_lock);
                id_map.emplace(entry->getEntryId(), entry);
            }
            break;
        case SELL:
            {
                if (entry->getEntryId() == 0)
                    entry->setEntryId(++order_counter);

                auto it = sell_map.find(entry->getAssetId());
                if (it == sell_map.end()) {
                    std::tie(it, std::ignore) = sell_map.emplace(entry->getAssetId(), AscPriceSet());
                }
                it->second.insert(entry);

                WriteLock wi_lock(id_lock);
                id_map.emplace(entry->getEntryId(), entry);
            }
            break;
        default:
            return EINVAL;
    }

    std::cout << "Inserted " << *entry << std::endl;

    return 0;
}

int TradeDB::Amend(const std::shared_ptr<Entry>& entry, uint64_t num_stocks, float price) {
    WriteLock wi_lock(id_lock);
    entry->setNumStocks(num_stocks);
    entry->setPrice(price);

    std::cout << "Entry amended to: " << *entry << std::endl;

    return 0;
}

int TradeDB::Delete(const std::shared_ptr<Entry>& entry) {
    auto id = entry->getEntryId();
    switch (entry->getEntryType()) {
        case BUY:
            {
                auto it = buy_map.find(entry->getAssetId());
                if (it == buy_map.end()) {
                    return ERANGE;
                }
                size_t removed = it->second.erase(entry);
                if (removed == 0) {
                    return ERANGE;
                }
                WriteLock wi_lock(id_lock);
                id_map.erase(entry->getEntryId());
            }
            break;
        case SELL:
            {
                auto it = sell_map.find(entry->getAssetId());
                if (it == sell_map.end()) {
                    return ERANGE;
                }
                size_t removed = it->second.erase(entry);
                if (removed == 0) {
                    return ERANGE;
                }
                WriteLock wi_lock(id_lock);
                id_map.erase(entry->getEntryId());
            }
            break;
        default:
            return EINVAL;
    }

    std::cout << "Entry with id: " << id << " got deleted" << std::endl;

    return 0;
}

std::shared_ptr<Entry> TradeDB::findById(const uint64_t entry_id) {
    ReadLock ri_lock(id_lock);
    auto it = id_map.find(entry_id);
    if (it == id_map.end()) {
        return {};
    }

    return it->second; //return shared_entry
}

void TradeDB::ExecuteOrder(const std::shared_ptr<Entry>& entry) {
    switch(entry->getEntryType()) {
        case BUY:
            {
                auto asset = entry->getAssetId();
                if (!sell_map.contains(asset) || sell_map[asset].empty()) {
                    if (entry->getEntryLimitType() != IOC) {
                        Insert(entry);
                    } else {
                        std::cout << "No offers found, cancel order" << std::endl;
                    }
                    return;
                }

                if (entry->getEntryId() == 0)
                    entry->setEntryId(++order_counter);

                auto our_price = entry->getPrice();
                auto our_stocks = entry->getNumStocks();
                for(const auto& sell_entry : sell_map[asset]) { // already sorted by asc price
                    if (our_price >= sell_entry->getPrice()) {
                        auto their_stocks = sell_entry->getNumStocks();
                        if (our_stocks == their_stocks) {
                            CompleteOrder(FULL, entry, sell_entry);
                            entry->setNumStocks(0);
                            Delete(sell_entry);
                            break;
                        } else if (our_stocks < their_stocks) {
                            CompleteOrder(PARTIAL, entry, sell_entry);
                            Amend(sell_entry, their_stocks-our_stocks, sell_entry->getPrice());
                            entry->setNumStocks(0);
                            break;
                        } else { // our_stocks > their_stocks
                            CompleteOrder(PARTIAL, entry, sell_entry);
                            Delete(sell_entry);
                            entry->setNumStocks(our_stocks-their_stocks);
                            our_stocks = entry->getNumStocks();
                        }
                    } else {
                        break;
                    }
                }
                if (entry->getEntryLimitType() != IOC && entry->getNumStocks() > 0) {
                    Insert(entry);
                }
            }
            break;
        case SELL:
        {
            auto asset = entry->getAssetId();
            if (!buy_map.contains(asset) || buy_map[asset].empty()) {
                if (entry->getEntryLimitType() != IOC) {
                    Insert(entry);
                } else {
                    std::cout << "No offers found, cancel order" << std::endl;
                }
                return;
            }

            if (entry->getEntryId() == 0)
                entry->setEntryId(++order_counter);

            auto our_price = entry->getPrice();
            auto our_stocks = entry->getNumStocks();
            auto buy_book = buy_map[asset];
            for(const auto &it : buy_book) { // already sorted by desc price
                auto buy_entry = it;
                if (buy_entry->getPrice() >= our_price) {
                    auto their_stocks = buy_entry->getNumStocks();
                    if (our_stocks == their_stocks) {
                        CompleteOrder(FULL, buy_entry, entry);
                        entry->setNumStocks(0);
                        Delete(buy_entry);
                        break;
                    } else if (our_stocks < their_stocks) {
                        CompleteOrder(PARTIAL, buy_entry, entry);
                        entry->setNumStocks(0);
                        Amend(buy_entry, their_stocks - our_stocks, buy_entry->getPrice());
                        break;
                    } else { // our_stocks > their_stocks
                        CompleteOrder(PARTIAL, entry, buy_entry);
                        Delete(buy_entry);
                        entry->setNumStocks(our_stocks-their_stocks);
                        our_stocks = entry->getNumStocks();
                    }
                } else {
                    break;
                }
            }
            if (entry->getEntryLimitType() != IOC && entry->getNumStocks() > 0) {
                Insert(entry);
            }
        }
            break;
        default:
            break;
    }
}

void TradeDB::CompleteOrder(TradeCompletionType tct, const std::shared_ptr<Entry>& buy_entry,
                            const std::shared_ptr<Entry>& sell_entry) {
    std::cout << "completed " << (tct == PARTIAL ? "partial" : "full") << " trade between "
              << *buy_entry << " and " << *sell_entry << std::endl;
}

void TradeDB::topOfTheBook(const std::string& asset_id) {
    if (buy_map.contains(asset_id) && !buy_map[asset_id].empty()) {
        const auto& top_buy = *buy_map[asset_id].cbegin(); // first elem because desc
        std::cout << "Top buy for asset " << asset_id <<  " is " << *top_buy << std::endl;
    } else {
        std::cout << "No buy orders found for this asset" << std::endl;
    }
    if (sell_map.contains(asset_id) && !sell_map[asset_id].empty()) {
        const auto& top_sell = *sell_map[asset_id].crbegin(); // last elem, because asc
        std::cout << "Top sell for asset " << asset_id <<  " is " << *top_sell << std::endl;
    } else {
        std::cout << "No sell orders found for this asset" << std::endl;
    }
}

void TradeDB::priceDepths(const std::string &asset_id) {
    if (buy_map.contains(asset_id) && !buy_map[asset_id].empty()) {
        std::cout << "Price depth for asset: " << std::endl;
        std::cout << "Buy depth:" << std::endl;
        for(const auto &it : buy_map[asset_id]) {
            std::cout << "\t" << *it << std::endl;
        }
    } else {
        std::cout << "No buy orders found for this asset" << std::endl;
    }
    if (sell_map.contains(asset_id) && !sell_map[asset_id].empty()) {
        std::cout << "Sell depth:" << std::endl;
        for(const auto &it : std::ranges::reverse_view(buy_map[asset_id])) {
            std::cout << "\t" << *it << std::endl;
        }
    } else {
        std::cout << "No sell orders found for this asset" << std::endl;
    }
}

void TradeDB::publicTradeTicks(const std::string &asset_id) {
    if (buy_map.contains(asset_id) && !buy_map[asset_id].empty()) {
        std::set<std::shared_ptr<Entry>, compareByTime> t_set;
        std::cout << "Public buy ticks for asset " << asset_id << std::endl;
        for(const auto& entry_ptr : buy_map[asset_id]) {
            t_set.insert(entry_ptr);
        }
        float prev_price = 0.f;
        for(const auto& entry_ptr : t_set) {
            auto curr_price = entry_ptr->getPrice();
            std::cout << "(" << curr_price - prev_price << ", "
                      << entry_ptr->getTimestamp().time_since_epoch().count() << ")" << std::endl;
            prev_price = curr_price;
        }
    } else {
        std::cout << "No buy orders found for this asset" << std::endl;
    }
    if (sell_map.contains(asset_id) && !sell_map[asset_id].empty()) {
        std::set<std::shared_ptr<Entry>, compareByTime> t_set;
        std::cout << "Public sell ticks for asset " << asset_id << std::endl;
        for(const auto& entry_ptr : sell_map[asset_id]) {
            t_set.insert(entry_ptr);
        }
        float prev_price = 0.f;
        for(const auto& entry_ptr : t_set) {
            auto curr_price = entry_ptr->getPrice();
            std::cout << "(" << curr_price - prev_price << ", "
                      << entry_ptr->getTimestamp().time_since_epoch().count() << ")" << std::endl;
            prev_price = curr_price;
        }
    } else {
        std::cout << "No buy orders found for this asset" << std::endl;
    }
}

