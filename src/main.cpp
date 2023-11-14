
#include <iostream>
#include <vector>

#include "DBConnection.h"
#include "Update.h"

DBConnection db;

bool processUpdate(Update* update) {

}

int main() {

    db.connect();

    std::vector<Update*> updates;
    while (1) {
        db.getUpdates(updates);
        if (updates.size() == 0) break;
        for (auto i = updates.begin(); i != updates.end(); i++) {
            if (processUpdate(*i)) {
                db.removeUpdate((*i)->id);
                delete *i;
                updates.erase(i);
            }
        }
    }
    
    return 0;
}