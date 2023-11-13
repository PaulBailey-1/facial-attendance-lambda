
#include <iostream>

#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/handshake_params.hpp>
#include <boost/mysql/results.hpp>
#include <boost/system/system_error.hpp>

#include "DBConnection.h"

DBConnection::DBConnection() : _ssl_ctx(boost::asio::ssl::context::tls_client), _conn(_ctx, _ssl_ctx) {}

DBConnection::~DBConnection() {
    _conn.close();
}

bool DBConnection::connect() {
    static bool logged = false;
    try {
        boost::asio::ip::tcp::resolver resolver(_ctx.get_executor());
        auto endpoints = resolver.resolve("127.0.0.1", boost::mysql::default_port_string);
        boost::mysql::handshake_params params("root", "", "test", boost::mysql::handshake_params::default_collation, boost::mysql::ssl_mode::enable);

        if (!logged) std::cout << "Connecting to mysql server at " << endpoints.begin()->endpoint() << " ... " << std::endl;

        _conn.connect(*endpoints.begin(), params);
    }
    catch (const boost::mysql::error_with_diagnostics& err) {
        std::cerr << "Error: " << err.what() << '\n'
            << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
        return false;
    }
    catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        return false;
    }
    if (!logged) { logged = true; std::cout << "Connected\n"; }
    return true;
}

bool DBConnection::query(const char* sql, boost::mysql::results &result) {
    try {
        _conn.execute(sql, result);
    }
    catch (const boost::mysql::error_with_diagnostics& err) {
        std::cerr << "Error: " << err.what() << '\n'
            << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
        return false;
    }
    catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        return false;
    }
    return true;
}

void DBConnection::createTables() {

    printf("Checking tables...\n");

    boost::mysql::results r;
    query("CREATE TABLE IF NOT EXISTS Updates (\
        device_id INT, \
        facial_features BLOB(512), \
        time TIMESTAMP DEFAULT CURRENT_TIMESTAMP \
    )", r);

    printf("Done\n");

}
//
//void DBConnection::getEntities(std::vector<Entity*> &vec) {
//
//    printf("Loading entities...\n");
//
//    try {
//        boost::mysql::results result;
//        _conn.execute("SELECT id, facial_features from Students", result);
//        if (!result.empty()) {
//            for (const boost::mysql::row_view& row : result.rows()) {
//                int id = row.at(0).as_int64();
//
//                std::vector<int> schedule;
//                boost::mysql::results scheduleResult;
//                _conn.execute(_conn.prepare_statement("SELECT room_id FROM Schedules WHERE student_id=? ORDER BY period ASC").bind(id), scheduleResult);
//                for (const boost::mysql::row_view& scheduleRow : scheduleResult.rows()) {
//                    schedule.push_back(scheduleRow[0].as_int64());
//                }
//
//                vec.push_back(new Entity(id, row.at(1).as_blob(), schedule));
//            }
//        }
//    }
//    catch (const boost::mysql::error_with_diagnostics& err) {
//        std::cerr << "Error: " << err.what() << '\n'
//            << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
//    }
//
//    printf("Done\n");
//
//}
//
//void DBConnection::pushUpdate(int devId, const boost::span<UCHAR> facialFeatures) {
//    try {
//        boost::mysql::results result;
//        _conn.execute(_conn.prepare_statement("INSERT INTO Updates (device_id, facial_features) VALUES(?, ?)").bind(devId, facialFeatures), result);
//    }
//    catch (const boost::mysql::error_with_diagnostics& err) {
//        std::cerr << "Error: " << err.what() << '\n'
//            << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
//    }
//}