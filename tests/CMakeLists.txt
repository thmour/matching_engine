add_executable(test_TradeDB test_TradeDB.cpp)
target_link_libraries(test_TradeDB PRIVATE TradeDB)

add_test(test_1 test_TradeDB)