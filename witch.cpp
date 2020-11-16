#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <deque>

using namespace std;


struct Brew
{
    int actionId;
    vector<int> delta;
    int price;
};

struct Cast
{
    int actionId;
    vector<int> delta;
    bool castable;
};

vector<int> add(vector<int> a, vector<int> b)
{
    vector<int> res(4);
    for (int i=0; i<4; i++){
        res[i] = a[i] + b[i];
    }
    return res;
}

bool can(vector<int> inv, vector<int> spell)
{
    // 1. Enough ingredients 
    for (int i =0; i<4; i++){
        if (inv[i] + spell[i] < 0){
            return false;
        }
    }
    // 2. Enough space
    int total = 0;
    for (int i=0; i<4; i++) {
        total += inv[i] + spell[i];
    }
    return total <= 10;
}

bool increasesMinimum(vector<int> inv, vector<int> cast)
{
    int m = *min_element(inv.begin(), inv.end());
    for (int i=1; i<cast.size(); i++){
        if ((cast[i] > 0) && (inv[i] != m)){
            return false;
        }
    }
    return true;
}


struct Witch
{
    vector<int> inv;
    vector<int> castId;
    vector<bool> castable;

    bool operator==(const Witch &w) const{
        return inv==w.inv && castId==w.castId && castable==w.castable;
    }

    bool operator<(const Witch &w) const{
        return inv<w.inv 
            || (inv==w.inv && castId<w.castId) 
            || (inv==w.inv && castId==w.castId && castable<w.castable);
    }

    bool operator!=(const Witch &w) const{
        return !(inv==w.inv && castId==w.castId && castable==w.castable);
    }
};

Witch witchCast(Witch w, Cast c)
{
    auto result = w;
    result.inv = add(w.inv, c.delta);
    result.castable[c.actionId] = false; // store by actionId
    return result;
}

vector<Witch> bfs(Witch startWitch, vector<Cast> casts, vector<Brew> brews)
{
    vector<Witch> result;
    map<Witch, Witch> prev;
    prev.insert(make_pair(startWitch, startWitch));
    deque<Witch> queue;
    queue.push_back(startWitch);
    while (!queue.empty() > 0){
        Witch currentWitch = queue[0];
        queue.pop_front();

        for (int i=0; i<brews.size(); i++){
            auto brew = brews[i];
            if (can(currentWitch.inv, brew.delta)){
                while (currentWitch != startWitch){
                    result.push_back(currentWitch);
                    auto it = prev.find(currentWitch);
                    if (it == prev.end()){
                        throw runtime_error("key in prev not found");
                    }
                    currentWitch = it->second;
                }
                result.push_back(startWitch); // len = turns + 1
                return result;
            }
        }

        for (int i=0; i<casts.size(); i++){
            auto cast = casts[i];
            if (!currentWitch.castable[cast.actionId]){ // store by actionId
                continue;
            }
            if (!can(currentWitch.inv, cast.delta)){
                continue;
            }
            auto newWitch = witchCast(currentWitch, cast);
            queue.push_back(newWitch);
            prev.insert(make_pair(newWitch, currentWitch));
        }
    }
    return result; // not fount -> empty
}

int main()
{

    // game loop
    while (1) {
        int actionCount; // the number of spells and recipes in play
        int actionId; // the unique ID of this spell or recipe
        int priciestBrewActionId;
        int maxPrice = 0;

        vector<Cast> casts;
        vector<Brew> brews;

        cin >> actionCount; cin.ignore();
        for (int i = 0; i < actionCount; i++) {
            string actionType; // in the first league: BREW; later: CAST, OPPONENT_CAST, LEARN, BREW
            int delta0; // tier-0 ingredient change
            int delta1; // tier-1 ingredient change
            int delta2; // tier-2 ingredient change
            int delta3; // tier-3 ingredient change
            int price; // the price in rupees if this is a potion
            int tomeIndex; // in the first two leagues: always 0; later: the index in the tome if this is a tome spell, equal to the read-ahead tax; For brews, this is the value of the current urgency bonus
            int taxCount; // in the first two leagues: always 0; later: the amount of taxed tier-0 ingredients you gain from learning this spell; For brews, this is how many times you can still gain an urgency bonus
            bool castable; // in the first league: always 0; later: 1 if this is a castable player spell
            bool repeatable; // for the first two leagues: always 0; later: 1 if this is a repeatable player spell
            cin >> actionId >> actionType >> delta0 >> delta1 >> delta2 >> delta3 >> price >> tomeIndex >> taxCount >> castable >> repeatable; cin.ignore();
            if (price > maxPrice) {
                maxPrice = price;
                priciestBrewActionId = actionId;
            }
            if (actionType == "CAST"){
                struct Cast cast = {actionId, vector<int>{delta0, delta1, delta2, delta3}, castable};
                casts.push_back(cast);
            } else if (actionType == "BREW") {
                struct Brew brew = {actionId, vector<int>{delta0, delta1, delta2, delta3}, price};
                brews.push_back(brew);
            }
        }

        vector<int> inv;
        for (int i = 0; i < 2; i++) {
            int inv0; // tier-0 ingredients in inventory
            int inv1;
            int inv2;
            int inv3;
            int score; // amount of rupees
            cin >> inv0 >> inv1 >> inv2 >> inv3 >> score; cin.ignore();
            if (i == 0){
                inv = vector<int>{inv0, inv1, inv2, inv3};
            }
        }

        int brewId = -1;
        for (int i = 0; i < brews.size(); i++) {
            bool canBrew = can(inv, brews[i].delta);
            if (canBrew) {
                brewId = brews[i].actionId;
                break;
            }
        }
        if (brewId > -1) {
            cout << "BREW " << brewId << endl;
            continue;
        }

        int castId = -1;
        for (int i = 0; i < casts.size(); i++) {
            cerr << casts[i].castable << " " << casts[i].actionId << endl;
            if (!casts[i].castable) {
                continue;
            }
            if (!can(inv, casts[i].delta)){
                continue;
            }
            if (!increasesMinimum(inv, casts[i].delta)){
                continue;
            }
            castId = casts[i].actionId;
            break;
        }
        if (castId > -1) {
            cout << "CAST " << castId << endl;
            continue;
        }

        cout << "REST" << endl;

        // in the first league: BREW <id> | WAIT; later: BREW <id> | CAST <id> [<times>] | LEARN <id> | REST | WAIT
        // cout << "CAST " << casts[0].actionId << endl;
        // cout << "BREW " << priciestBrewActionId << endl;
    }
}
