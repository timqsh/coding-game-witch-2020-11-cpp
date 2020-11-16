#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

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

bool can(vector<int> a, vector<int> b)
{
    for (int i =0; i<4; i++){
        if (a[i] + b[i] < 0){
            return false;
        }
    }
    return true;
}

int minimum(vector<int> a){
    int m = a[0];
    for (int i=1; i<a.size(); i++){
        if (a[i] < m){
            m = a[i];
        }
    }
    return m;
}

bool increasesMinimum(vector<int> inv, vector<int> cast){
    int m = minimum(inv);
    for (int i=1; i<cast.size(); i++){
        if ((cast[i] > 0) && (inv[i] != m)){
            return false;
        }
    }
    return true;
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
            bool canCast = can(inv, casts[i].delta);
            int total = 0;
            for (int j=0; j<4; j++) {
                total += inv[j] + casts[i].delta[j];
            }
            if (!increasesMinimum(inv, casts[i].delta)){
                continue;
            }
            if (canCast and total<=10) {
                castId = casts[i].actionId;
                break;
            }
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
