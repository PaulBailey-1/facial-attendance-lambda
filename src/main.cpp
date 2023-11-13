#include <iostream>

#include "DBConnection.h"

int main() {

    DBConnection db;
    db.connect();

    while (1) {}
    
    return 0;
}