#pragma GCC optimize "O3,omit-frame-pointer,inline"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <deque>
#include <time.h>
#include <iomanip>
#include <cstdlib>

using namespace std;


struct Brew
{
    int actionId;
    array<int, 4> delta;
    int price;
};

struct Cast
{
    int actionId;
    array<int, 4> delta;
    bool _castable;
    bool repeatable;

    bool operator==(const Cast &c) const{
        return actionId==c.actionId && _castable==c._castable;
    }

    bool operator<(const Cast &c) const{
        return actionId<c.actionId
            || (actionId==c.actionId && _castable<c._castable);
    }
};

struct Learn
{
    int actionId;
    array<int, 4> delta;
    int tomeIndex;
    int taxCount;
    bool repeatable;
};

array<int, 4> add(array<int, 4> a, array<int, 4> b)
{
    array<int, 4> res;
    for (int i=0; i<4; i++){
        res[i] = a[i] + b[i];
    }
    return res;
}

bool can(array<int, 4> inv, array<int, 4> spell)
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

bool increasesMinimum(array<int, 4> inv, array<int, 4> cast)
{
    int m = *min_element(inv.begin(), inv.end());
    for (int i=1; i<4; i++){
        if ((cast[i] > 0) && (inv[i] != m)){
            return false;
        }
    }
    return true;
}


struct Witch
{
    array<int, 4> inv;
    array<bool, 64> casts;
    array<bool, 64> castable;

    bool operator==(const Witch &w) const{
        return inv==w.inv && casts==w.casts && castable==w.castable;
    }

    bool operator<(const Witch &w) const{
        return inv<w.inv 
            || (inv==w.inv && casts<w.casts)
            || (inv==w.inv && casts==w.casts && castable<w.castable);
    }

    bool operator!=(const Witch &w) const{
        return not (*this==w);
    }
};

Witch witchCast(Witch w, int castId, array<Cast,64> casts)
{
    auto result = w;
    result.castable[castId] = false;
    result.inv = add(w.inv, casts[castId].delta);
    return result;
}

Witch witchRest(Witch w)
{
    auto result = w;
    for(int i=0; i<result.casts.size(); i++){
        result.castable[i] = true;
    }
    return result;
}

Witch witchLearn(Witch w, Learn l)
{
    auto result = w;

    // add cast to casts global table
    Cast newCast = {63, l.delta, true, l.repeatable};
    result.casts[63] = true;
    result.castable[63] = true;

    w.inv[0] -= l.tomeIndex;
    auto freeSlots = 10 - w.inv[0]-w.inv[1]-w.inv[2]-w.inv[3];
    w.inv[0] += min(l.taxCount, freeSlots);

    return result;
}

bool witchCanLearn(Witch w, Learn l)
{
    auto blues = w.inv[0];
    return blues >= l.tomeIndex;
}

Witch cpWitch(Witch w)
{
    auto result = w;
    return result;
}

vector<string> bfs(Witch startWitch, array<Cast, 64> casts, vector<Brew> brews, vector<Learn> learns, time_t timeStart, bool timeControl)
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
        if (timeControl and (difftime(clock(), timeStart) > 39000)){
            break;
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

        if (iterations==1){
            int learnsCount = 0;
            for(auto learn:learns){
                if (witchCanLearn(currentWitch, learn)){
                    learnsCount ++;
                    int castsLearnIndex = 64 - learnsCount;
                    
                    //witchLearn
                    auto newWitch = currentWitch;
                    casts[castsLearnIndex].actionId = castsLearnIndex;
                    casts[castsLearnIndex].delta = learn.delta;
                    casts[castsLearnIndex].repeatable = learn.repeatable;
                    newWitch.casts[castsLearnIndex] = true;
                    newWitch.castable[castsLearnIndex] = true;

                    if (prev.find(newWitch) == prev.end()){
                        queue.push_back(newWitch);
                        prev.insert(make_pair(newWitch, currentWitch));
                        actions.insert(make_pair(newWitch, 
                            "LEARN " + to_string(learn.actionId) + " +LEARNING!"));
                    }
                }
            }
        }

        for (int i=0; i<64; i++){
            if (not currentWitch.casts[i]) {
                continue;
            }
            if (not currentWitch.castable[i]){
                continue;
            }
            auto cast = casts[i];
            if (not can(currentWitch.inv, cast.delta)){
                continue;
            }

            //withCast
            auto newWitch = currentWitch;
            newWitch.castable[i] = false;
            newWitch.inv = add(newWitch.inv, cast.delta);

            if (prev.find(newWitch) == prev.end()){
                queue.push_back(newWitch);
                prev.insert(make_pair(newWitch, currentWitch));
                actions.insert(make_pair(newWitch, "CAST " + to_string(cast.actionId) + " 1 "));
            }
            if (cast.repeatable){
                int times = 1;
                while (can(newWitch.inv, cast.delta)){
                    times++;

                    //withCast
                    newWitch = cpWitch(newWitch);
                    newWitch.castable[i] = false;
                    newWitch.inv = add(newWitch.inv, cast.delta);

                    if (prev.find(newWitch) == prev.end()){
                        queue.push_back(newWitch);
                        prev.insert(make_pair(newWitch, currentWitch));
                        actions.insert(make_pair(newWitch, 
                            "CAST " + to_string(cast.actionId) + " " + to_string(times) + " MULTICAST! "));
                    }
                }
            }
        }

        //witchRest
        auto newWitch = currentWitch;
        for (int i=0; i<64; i++) {
            newWitch.castable[i] = true;
        }

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
    bool debug;
    auto debug_env = getenv("DEBUG");
    if (debug_env==NULL){
        debug = false;
    } else {
        debug = (string)debug_env == (string)"true";
        cerr << "DEBUG=true" << endl;
    }

    // game loop
    int turn = 0;
    while (1) {
        turn ++;
        int actionCount; // the number of spells and recipes in play
        int actionId; // the unique ID of this spell or recipe
        int priciestBrewActionId;
        int maxPrice = 0;

        array<Cast, 64> casts;
        int castsSize = 0;
        for (int i=0; i<64; i++) {
            casts[i].actionId = -1;
            casts[i]._castable = false;
            casts[i].delta = {0,0,0,0};
        }
        vector<Brew> brews;
        vector<Learn> learns;

        cin >> actionCount; cin.ignore();
        if (actionCount == 9999){
            break; //DEBUG
        }
        cerr << actionCount << endl;
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
            cerr << actionId << " " << actionType << " " << delta0 << " " << delta1 << " " << delta2 << " " << delta3 << " " << price << " " << tomeIndex << " " << taxCount << " " << castable << " " << repeatable << endl;
            if (price > maxPrice) {
                maxPrice = price;
                priciestBrewActionId = actionId;
            }
            if (actionType == "CAST"){
                struct Cast cast = {actionId, array<int, 4>{delta0, delta1, delta2, delta3}, castable, repeatable};
                casts[i] = cast;
                castsSize ++;
            } else if (actionType == "BREW") {
                struct Brew brew = {actionId, array<int, 4>{delta0, delta1, delta2, delta3}, price};
                brews.push_back(brew);
            } else if (actionType == "LEARN") {
                struct Learn learn = {actionId, array<int, 4>{delta0, delta1, delta2, delta3}, 
                    tomeIndex, taxCount, repeatable};
                learns.push_back(learn);
            }
        }

        array<int, 4> inv;
        for (int i = 0; i < 2; i++) {
            int inv0; // tier-0 ingredients in inventory
            int inv1;
            int inv2;
            int inv3;
            int score; // amount of rupees
            cin >> inv0 >> inv1 >> inv2 >> inv3 >> score; cin.ignore();
            cerr << inv0 << " " << inv1 << " " << inv2 << " " << inv3 << " " << score << endl;
            if (i == 0){
                inv = array<int, 4>{inv0, inv1, inv2, inv3};
            }
        }

        auto start = clock();

        // 0. Learn
        if ((castsSize < 12) and (not debug)){
            Learn first_tome;
            for(auto elem: learns){
                if (elem.tomeIndex==0){
                    first_tome = elem;
                }
            }
            cout << "LEARN " << first_tome.actionId << " start learning" << endl;
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
            cout << "BREW " << brewId << " BREW!" << endl;
            continue;
        }

        // 2. BFS
        Witch myWitch;
        myWitch.inv = inv;
        for (int i=0; i<64; i++) {
            myWitch.casts[i] = casts[i].actionId > -1;
            myWitch.castable[i] = casts[i]._castable;
        }
        auto result = bfs(myWitch, casts, brews, learns, start, (not debug));
        auto end = clock();
        auto elapsed = difftime(end, start);
        if (result.size()>0){
            for(auto r:result){
                cerr << "# path - " << r << endl;
            }
            if (result.size()==1){
                throw runtime_error("Bfs returned path with len=1, shoed have brewed before...");
            }
            auto firstMove = result[result.size()-2];
            cout << firstMove << " T-" + to_string(result.size()-1) 
                + " " << setprecision(2) << elapsed/1000 << "ms";
            cout << endl;
            continue;
        }

        // 3. Cast increases minimum
        int castId = -1;
        for (int i = 0; i < casts.size(); i++) {
            if (!casts[i]._castable) {
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
            cout << "CAST " << castId << " 1 increase minimum " 
                << setprecision(2) << elapsed/1000 << "ms";
            cout << endl;
            continue;
        }

        // 4. Rest
        cout << "REST just rest" << setprecision(2) << elapsed/1000 << "ms";
        cout << endl;

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

// void test()
// {
//     Witch startWitch = {
//         {3,0,0,0}, 
//         {
//             {111,{2,0,0,0}, true},
//             {222,{-1,1,0,0}, true},
//             {333,{0,-1,1,0}, true},
//             {444,{0,0,-1,1}, true}
//         }
//     };
//     vector<Brew> brews = {{777, {0,0,0,-4}, 100500}};
//     vector<Learn> learns = {};
//     auto result = bfs(startWitch, brews, learns, clock(), false);
//     printWithes(result);
// }

int main()
{
    prod();
    // test();
}
