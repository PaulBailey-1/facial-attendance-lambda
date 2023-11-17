
#include <iostream>

#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/handshake_params.hpp>
#include <boost/mysql/results.hpp>
#include <boost/system/system_error.hpp>
#include <fmt/core.h>

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

    const int face_bytes = FACE_VEC_SIZE * 4;
    boost::mysql::results r;
    query(fmt::format("CREATE TABLE IF NOT EXISTS updates (\
        id INT AUTO_INCREMENT PRIMARY KEY, \
        device_id INT, \
        facial_features BLOB({}), \
        time TIMESTAMP DEFAULT CURRENT_TIMESTAMP \
    )", face_bytes).c_str(), r);
    // needs to include face vec variances
    // needs expected dts between devs for periods
    query(fmt::format("CREATE TABLE IF NOT EXISTS long_term_state (\
        id INT AUTO_INCREMENT PRIMARY KEY, \
        mean_facial_features BLOB({}) \
    )", face_bytes).c_str(), r);
    query(fmt::format("CREATE TABLE IF NOT EXISTS short_term_state (\
        id INT AUTO_INCREMENT PRIMARY KEY NOT NULL, \
        mean_facial_features BLOB(512) NOT NULL, \
        last_update_device_id INT NOT NULL, \
        last_update_time TIMESTAMP UPDATE CURRENT_TIMESTAMP NOT NULL, \
        expected_next_update_device_id INT, \
        expected_next_update_time TIMESTAMP, \
        expected_next_update_time_variance FLOAT(0), \
        long_term_state_key INT, \
        CONSTRAINT FK_lts FOREIGN KEY (long_term_state_key) REFERENCES long_term_state(id) \
    )", face_bytes).c_str(), r);

    printf("Done\n");

}

void DBConnection::getUpdates(std::vector<Update*> &updates) {
    boost::mysql::results result;
    query("SELECT id, device_id, facial_features FROM updates ORDER BY time ASC", result);
    if (!result.empty()) {
        for (const boost::mysql::row_view& row : result.rows()) {
            updates.push_back(new Update(row[0].as_int64(), row[1].as_int64(), row[2].as_blob()));
        }
    }
}

void DBConnection::removeUpdate(int id) {
    try {
        boost::mysql::results result;
        _conn.execute(_conn.prepare_statement("DELETE FROM updates WHERE id=?").bind(id), result);
    }
    catch (const boost::mysql::error_with_diagnostics& err) {
        std::cerr << "Error: " << err.what() << '\n'
            << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
    }
}

void DBConnection::getLongTermState(std::vector<LongTermState*>& states) {
    boost::mysql::results result;
    query("SELECT id, facial_features FROM long_term_state", result);
    if (!result.empty()) {
        for (const boost::mysql::row_view& row : result.rows()) {
            states.push_back(new LongTermState(row[0].as_int64(), row[1].as_blob()));
        }
    }
}

void DBConnection::createShortTermState(const Update* update) {
    try {
        boost::mysql::results result;
        const boost::span<UCHAR> facialFeatures = boost::span<UCHAR>(reinterpret_cast<UCHAR*>(const_cast<float*>(update->facialFeatures.data())), update->facialFeatures.size() * sizeof(float));

        _conn.execute(_conn.prepare_statement(
                "INSERT INTO short_term_state (mean_facial_features, last_update_device_id) 
                    VALUES(?,?)").bind(facialFeatures, update->id), result);
    }
    catch (const boost::mysql::error_with_diagnostics& err) {
        std::cerr << "Error: " << err.what() << '\n'
            << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
    }
}