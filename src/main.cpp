
#include <iostream>
#include <vector>

#include "DBConnection.h"
#include "Objects.h"

DBConnection db;

std::vector<LongTermState*> longTermState;
std::vector<ShortTermState*> shortTermState;

int facialMatch(const Update* update, std::vector<LongTermState*> &pool) {
    for (int i = 0; i < pool.size(); i++) {
        float l2norm = 0.0;
        for (int j = 0; j < FACE_VEC_SIZE; j++) {
            l2norm += pow(update->facial_features[j] - pool[i]->facial_features[j], 2);
        }
        l2norm /= FACE_VEC_SIZE;
        // this should be using the variance
        if (l2norm < 0.01) {
            return i;
        }
    }
    return -1;
}

bool processUpdate(const Update* update) {
    // match against who is probably there
    // match against who could be there
    // match against people seen
    // match against people known
    int match = facialMatch(update, longTermState);

    if (match >= 0) {
        
    } else {

    }
    
    // if matched, apply update to short-term state
    // else create short term profile
}

int main() {

    db.connect();
    db.getLongTermState(longTermState);

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