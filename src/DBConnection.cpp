
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
    query("CREATE TABLE IF NOT EXISTS updates (\
        id INT AUTO_INCREMENT PRIMARY KEY, \
        device_id INT, \
        facial_features BLOB(512), \
        time TIMESTAMP DEFAULT CURRENT_TIMESTAMP \
    )", r);
    query("CREATE TABLE IF NOT EXISTS short_term_state (\
        id INT AUTO_INCREMENT PRIMARY KEY, \
        mean_facial_features BLOB(512), \
        last_update_device_id INT, \
        last_update_time TIMESTAMP \
    )", r);

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