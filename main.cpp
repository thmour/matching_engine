#include <iostream>
#include <future>

#include "include/TradeDB.hpp"
#include "include/ThreadPool.hpp"

int main() {
    ThreadPool tp{};
    TradeDB tdb[tp.num_threads];

    int input = -1;
    while(input != 0) {
        std::cout << "Please enter a command to process:" << std::endl
                  << "  0. Exit program" << std::endl
                  << "  1. Insert order" << std::endl
                  << "  2. Amend order" << std::endl
                  << "  3. Delete order" << std::endl
                  << "  4. Top of the book" << std::endl
                  << "  5. Price depths" << std::endl
                  << "  5. Trade ticks" << std::endl
                  << std::endl << "Your input (0-3): ";
        std::cin >> input;
        std::cout << std::endl;
        switch (input) {
            case 1:
                {
                    int entry_type, entry_limit_type;
                    uint64_t num_stocks;
                    float price;
                    std::string asset_id, trader_id;
                    std::cout << std::endl << "Enter the order details" << std::endl
                              << std::endl << "Enter the order type" << std::endl
                              << std::endl << "Your input (0:Buy,1:Sell): ";
                    std::cin >> entry_type;
                    if (entry_type < 0 || entry_type > 1) {
                        std::cout << "Invalid input" << std::endl;
                        break;
                    }

                    std::cout << std::endl << "Enter the order limit type" << std::endl
                              << std::endl << "Your input (0:Normal,1:GFD,2:IOC): ";
                    std::cin >> entry_limit_type;
                    if (entry_limit_type < 0 || entry_limit_type > 2) {
                        std::cout << "Invalid input" << std::endl;
                        break;
                    }

                    std::cout << std::endl << "Enter the asset name" << std::endl
                              << "Your input: ";
                    std::cin >> asset_id;

                    std::cout << std::endl << "Enter the trader name" << std::endl
                              << "Your input: ";
                    std::cin >> trader_id;

                    std::cout << std::endl << "Enter the number of stocks" << std::endl
                              << "Your input: ";
                    std::cin >> num_stocks;
                    if (num_stocks <= 0) {
                        std::cout << "Invalid input, stocks must be greater than 1" << std::endl;
                        break;
                    }

                    std::cout << std::endl << "Enter the price per stock" << std::endl
                              << "Your input: ";
                    std::cin >> price;
                    if (price <= 0) {
                        std::cout << "Invalid input, price must be greater than 1" << std::endl;
                        break;
                    }

                    auto entry = std::make_shared<Entry>(
                            static_cast<EntryType>(entry_type),
                            static_cast<EntryLimitType>(entry_limit_type),
                            0, asset_id, trader_id, num_stocks, price
                    );
                    tp.add_job(
                        [&tdb, entry] (int tid) {
                            tdb[tid].ExecuteOrder(entry);
                        }, entry->getAssetId()
                    );
                }
                break;
            case 2:
                {
                    uint64_t entry_id;
                    std::cout << "Enter the order id to be updated" << std::endl
                              << std::endl << "Your input (order id): ";
                    std::cin >> entry_id;
                    std::cout << std::endl;
                    if (entry_id == 0) {
                        std::cout << "Invalid entry" << std::endl;
                        break;
                    }
                    auto entry = TradeDB::findById(entry_id);
                    if (entry == nullptr) {
                        std::cout << "Entry not found" << std::endl;
                        break;
                    }

                    float price;
                    uint64_t num_stocks;
                    std::cout << std::endl << "Enter a new price (0 to skip, current value: "
                              << entry->getPrice() << "): " << std::endl;
                    std::cin >> price;
                    std::cout << std::endl;
                    if (price <= 0) {
                        price = entry->getPrice();
                    }
                    std::cout << std::endl << "Enter a new stock number (0 to skip, current value: "
                              << entry->getNumStocks() << "): " << std::endl;
                    std::cin >> num_stocks;
                    if (num_stocks <= 0) {
                        num_stocks = entry->getNumStocks();
                    }
                    tp.add_job(
                        [entry, num_stocks, price](int thread_id) {
                            TradeDB::Amend(entry, num_stocks, price);
                        }, entry->getAssetId()
                    );
                }
                break;
            case 3:
                {
                    uint64_t entry_id;
                    std::cout << "Enter the order id to be deleted" << std::endl
                              << "Your input: ";
                    std::cin >> entry_id;
                    std::cout << std::endl;
                    if (entry_id == 0) {
                        std::cout << "Invalid entry" << std::endl;
                        break;
                    }
                    auto entry = TradeDB::findById(entry_id);
                    if (entry == nullptr) {
                        std::cout << "Entry not found" << std::endl;
                    }

                    std::promise<int> p;
                    std::future<int> r = p.get_future();
                    tp.add_job(
                        [&tdb, entry, &p](int thread_id) {
                            p.set_value(tdb[thread_id].Delete(entry));
                        }, entry->getAssetId()
                    );
                    if (r.get() != 0) {
                        std::cout << "Error deleting entry" << std::endl;
                        break;
                    }
                }
                break;
            case 4:
                {
                    std::string asset_id;
                    std::cout << "Enter the asset id to get the top orders" << std::endl
                              << "Your input: ";
                    std::cin >> asset_id;
                    std::cout << std::endl;
                    tp.add_job(
                        [&tdb, &asset_id](int thread_id) {
                            tdb[thread_id].topOfTheBook(asset_id);
                        }, asset_id
                    );
                }
                break;
            case 5:
                {
                    std::string asset_id;
                    std::cout << "Enter the asset id to get the price depths" << std::endl
                              << "Your input: ";
                    std::cin >> asset_id;
                    std::cout << std::endl;
                    tp.add_job(
                        [&tdb, &asset_id](int thread_id) {
                            tdb[thread_id].priceDepths(asset_id);
                        }, asset_id
                    );
                }
                break;
            case 6:
                {
                    std::string asset_id;
                    std::cout << "Enter the asset id to get the trade ticks" << std::endl
                              << "Your input: ";
                    std::cin >> asset_id;
                    std::cout << std::endl;
                    tp.add_job(
                        [&tdb, &asset_id](int thread_id) {
                            tdb[thread_id].publicTradeTicks(asset_id);
                        }, asset_id
                    );
                }
                break;
            default:
                input = 0;
        }
        std::cout << std::endl;
    }

    tp.terminate();

    return 0;
}
