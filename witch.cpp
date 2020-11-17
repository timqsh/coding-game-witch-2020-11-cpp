// #pragma GCC optimize "O3,omit-frame-pointer,inline"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <deque>
#include <iomanip>
#include <chrono>

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

    bool operator==(const Cast &c) const{
        return actionId==c.actionId && castable==c.castable;
    }

    bool operator<(const Cast &c) const{
        return actionId<c.actionId
            || (actionId==c.actionId && castable<c.castable);
    }
};

struct Learn
{
    int actionId;
    vector<int> delta;
    int tomeIndex;
    int taxCount;
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
    vector<Cast> casts;

    bool operator==(const Witch &w) const{
        return inv==w.inv && casts==w.casts;
    }

    bool operator<(const Witch &w) const{
        return inv<w.inv 
            || (inv==w.inv && casts<w.casts);
    }

    bool operator!=(const Witch &w) const{
        return not (*this==w);
    }
};

Witch witchCast(Witch w, Cast c)
{
    auto result = w;
    result.inv = add(w.inv, c.delta);
    for(int i=0; i<result.casts.size(); i++){
        if (result.casts[i].actionId == c.actionId){
            result.casts[i].castable = false;
        }
    }
    return result;
}

Witch witchRest(Witch w)
{
    auto result = w;
    for(int i=0; i<result.casts.size(); i++){
        result.casts[i].castable = true;
    }
    return result;
}

vector<string> bfs(Witch startWitch, vector<Brew> brews, chrono::steady_clock::time_point timeStart, bool timeControl = true)
{
    vector<string> result;
    map<Witch, Witch> prev;
    prev.insert(make_pair(startWitch, startWitch));
    map<Witch, string> actions;
    actions.insert(make_pair(startWitch, "Start"));
    deque<Witch> queue;
    queue.push_back(startWitch);
    int iterations = 0;
    while (!queue.empty() > 0){
        if (timeControl){
            auto currentTime = chrono::steady_clock::now();
            chrono::duration<float> duration = currentTime - timeStart;
            auto nanoseconds = duration.count();
            if (nanoseconds > 40000){
                break;
            }
        }
        iterations++;
        Witch currentWitch = queue[0];
        queue.pop_front();

        for (int i=0; i<brews.size(); i++){
            auto brew = brews[i];
            if (can(currentWitch.inv, brew.delta)){
                while (currentWitch != startWitch){
                    result.push_back(actions[currentWitch]+" nodes:"+to_string(prev.size()));
                    auto it = prev.find(currentWitch);
                    if (it == prev.end()){
                        throw runtime_error("key in prev not found");
                    }
                    currentWitch = it->second;
                }
                result.push_back("Start"); // len = turns + 1
                return result;
            }
        }

        for (auto cast : currentWitch.casts){
            if (not cast.castable){
                continue;
            }
            if (not can(currentWitch.inv, cast.delta)){
                continue;
            }
            auto newWitch = witchCast(currentWitch, cast);
            queue.push_back(newWitch);
            prev.insert(make_pair(newWitch, currentWitch));
            actions.insert(make_pair(newWitch, "CAST " + to_string(cast.actionId) + " 1 "));
        }

        auto newWitch = witchRest(currentWitch);
        if (prev.find(newWitch) == prev.end()){
            queue.push_back(newWitch);
            prev.insert(make_pair(newWitch, currentWitch));
            actions.insert(make_pair(newWitch, "REST "));
        }
    }
    return result; // not fount -> empty
}


void prod()
{

    // game loop
    while (1) {
        int actionCount; // the number of spells and recipes in play
        int actionId; // the unique ID of this spell or recipe
        int priciestBrewActionId;
        int maxPrice = 0;

        vector<Cast> casts;
        vector<Brew> brews;
        vector<Learn> learns;

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
            } else if (actionType == "LEARN") {
                struct Learn learn = {actionId, vector<int>{delta0, delta1, delta2, delta3}, tomeIndex, taxCount};
                learns.push_back(learn);
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

        auto t0 = chrono::steady_clock::now();

        // 0. Learn
        if (casts.size() < 10){
            Learn first_tome;
            for(auto elem: learns){
                if (elem.tomeIndex==0){
                    first_tome = elem;
                }
            }
            cout << "LEARN " << first_tome.actionId << endl;
            continue;
        }

        // 1. Can brew.
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

        // 2. BFS
        Witch myWitch;
        myWitch.inv = inv;
        myWitch.casts = casts;
        auto result = bfs(myWitch, brews, t0);
        auto t1 = chrono::steady_clock::now();
        chrono::duration<float> duration = t1 - t0;
        auto nanoseconds = duration.count();
        if (result.size()>0){
            if (result.size()==1){
                throw runtime_error("Bfs returned path with len=1, shoed have brewed before...");
            }
            auto firstMove = result[result.size()-2];
            cout << firstMove << " T-" + to_string(result.size()) 
                + " " << setprecision(2) << nanoseconds/1000;
            cout << endl;
            continue;
        }

        // 3. Cast increases minimum
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

        // 4. Rest
        cout << "REST" << endl;

        // in the first league: BREW <id> | WAIT; later: BREW <id> | CAST <id> [<times>] | LEARN <id> | REST | WAIT
        // cout << "CAST " << casts[0].actionId << endl;
        // cout << "BREW " << priciestBrewActionId << endl;
    }
}

void printWithes(vector<string> actions)
{
    for(auto a:actions){
        cout << a << endl;
    }
}

void test()
{
    Witch startWitch = {
        {3,0,0,0}, 
        {
            {111,{2,0,0,0}, true},
            {222,{-1,1,0,0}, true},
            {333,{0,-1,1,0}, true},
            {444,{0,0,-1,1}, true}
        }
    };
    vector<Brew> brews = {{777, {0,0,0,-4}, 100500}};
    auto result = bfs(startWitch, brews, chrono::steady_clock::now(), false);
    printWithes(result);
}

int main()
{
    prod();
    // test();
}
