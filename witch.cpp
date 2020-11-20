#pragma GCC optimize "O3,omit-frame-pointer,inline"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <deque>
#include <iomanip>
#include <cstdlib>
#include <bitset>
#include <chrono>

using namespace std;

inline double currentMs() {
    return chrono::duration<double>(chrono::steady_clock::now().time_since_epoch()).count() * 1000;
}

struct Brew
{
    int16_t actionId;
    array<int16_t, 4> delta;
    int price;
};

struct Cast
{
    int16_t actionId;
    array<int16_t, 4> delta;
    bool _castable;
    bool repeatable;
};

struct Learn
{
    int16_t actionId;
    array<int16_t, 4> delta;
    int tomeIndex;
    int taxCount;
    bool repeatable;
};

inline array<int16_t, 4> add(const array<int16_t, 4>& a, const array<int16_t, 4>& b)
{
    array<int16_t, 4> res;
    for (int16_t i=0; i<4; i++){
        res[i] = a[i] + b[i];
    }
    return res;
}

inline bool can(const array<int16_t, 4>& inv, const array<int16_t, 4>& spell)
{
    // 1. Enough ingredients 
    for (int16_t i =0; i<4; i++){
        if (inv[i] + spell[i] < 0){
            return false;
        }
    }
    // 2. Enough space
    int16_t total = 0;
    for (int16_t i=0; i<4; i++) {
        total += inv[i] + spell[i];
    }
    return total <= 10;
}

bool increasesMinimum(array<int16_t, 4> inv, array<int16_t, 4> cast)
{
    int16_t m = *min_element(inv.begin(), inv.end());
    for (int16_t i=1; i<4; i++){
        if ((cast[i] > 0) && (inv[i] != m)){
            return false;
        }
    }
    return true;
}

enum ActionType {aBrew, aLearn, aCast, aRest};

struct Action
{
    inline Action() = default;
    inline Action(Action const&) = default;
    inline Action(Action&&) = default;
    inline Action& operator=(Action const&) = default;
    inline Action& operator=(Action&&) = default;

    ActionType type;
    int16_t id;
    int16_t times;
};

struct Witch
{
    inline Witch() = default;
    inline Witch(Witch const&) = default;
    inline Witch(Witch&&) = default;
    inline Witch& operator=(Witch const&) = default;
    inline Witch& operator=(Witch&&) = default;

    array<int16_t, 4> inv;
    uint64_t castsMask;
    uint64_t castableMask;
    int16_t score;
    int16_t turns;
    bitset<6> brewsRemaining;
    Action action;

    inline bool operator==(const Witch &w) const{
        return inv==w.inv 
            && castsMask==w.castsMask 
            && castableMask==w.castableMask
            && score==w.score
            // && turns==w.turns
            && brewsRemaining==w.brewsRemaining
            ;
    }

    inline bool operator!=(const Witch &w) const{
        return not (*this==w);
    }
};

namespace std{
template<>
struct hash<Witch>
{
    inline std::size_t operator()(const Witch& w) const
    {
        std::size_t seed = 0;

        for (auto e: w.inv) {
            seed ^= hash<int>{}(e) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }
        seed ^= hash<uint64_t>{}(w.castsMask) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= hash<uint64_t>{}(w.castableMask) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= hash<uint64_t>{}(w.score) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        // seed ^= hash<uint64_t>{}(w.turns) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        seed ^= hash<bitset<6>>{}(w.brewsRemaining) + 0x9e3779b9 + (seed<<6) + (seed>>2);

        return seed;
    };
};
}

inline bool witchCanLearn(const Witch& w, const Learn& l)
{
    auto blues = w.inv[0];
    return blues >= l.tomeIndex;
}

vector<string> bfs(
    Witch startWitch,
    array<Cast, 64> casts,
    vector<Brew> brews,
    vector<Learn> learns,
    double timeStart,
    bool timeControl,
    unordered_map<Witch, Witch>& prev,
    deque<Witch>& queue
)
{
    vector<string> result;
    prev.insert(make_pair(startWitch, startWitch));
    queue.push_back(startWitch);
    int iterations = 0;

    int maxScore = 0;
    int minTurns = 99999;
    int maxTurns = 0;
    Witch maxWitch;

    while (!queue.empty() > 0){
        iterations++;
        Witch currentWitch = queue[0];
        queue.pop_front();
        if (timeControl and (currentMs() - timeStart > 45)){     
            break;
        }
        maxTurns = currentWitch.turns + 1;

        for (int i=0; i<brews.size(); i++){
            auto brew = brews[i];
            if (not currentWitch.brewsRemaining.test(i)) {
                continue;
            }
            if (not can(currentWitch.inv, brew.delta)){
                continue;
            }

            auto newWitch = currentWitch;
            newWitch.inv = add(newWitch.inv, brew.delta);
            newWitch.turns++;
            newWitch.score += brew.price;
            newWitch.brewsRemaining.reset(i);
            newWitch.action = Action{aBrew, brew.actionId, 0};

            if (prev.find(newWitch) == prev.end()){
                queue.push_back(newWitch);
                prev.insert(make_pair(newWitch, currentWitch));

                if (newWitch.score>maxScore || newWitch.score==maxScore && newWitch.turns<minTurns) {
                    maxScore = newWitch.score;
                    maxWitch = newWitch;
                    minTurns = newWitch.turns;
                }
            }
        }

        if (iterations==1){
            int learnsCount = 0;
            for(auto learn:learns){
                if (witchCanLearn(currentWitch, learn)){
                    learnsCount ++;
                    int castsLearnIndex = 63 - learnsCount;
                    
                    auto newWitch = currentWitch;
                    casts[castsLearnIndex].actionId = 256-learnsCount;
                    casts[castsLearnIndex].delta = learn.delta;
                    casts[castsLearnIndex].repeatable = learn.repeatable;
                    newWitch.castsMask |= (1ull<<castsLearnIndex);
                    newWitch.inv[0] -= learn.tomeIndex;
                    auto freeSlots = 10 - newWitch.inv[0]-newWitch.inv[1]-newWitch.inv[2]-newWitch.inv[3];
                    newWitch.inv[0] += min(learn.taxCount, freeSlots);
                    newWitch.turns++;
                    newWitch.action = Action{aLearn, learn.actionId, 0};

                    if (prev.find(newWitch) == prev.end()){
                        queue.push_back(newWitch);
                        prev.insert(make_pair(newWitch, currentWitch));

                        if (newWitch.score>maxScore || newWitch.score==maxScore && newWitch.turns<minTurns) {
                            maxScore = newWitch.score;
                            maxWitch = newWitch;
                            minTurns = newWitch.turns;
                        }
                    }
                }
            }
        }

        for (int i=0; i<64; i++){
            if (!(currentWitch.castsMask & (1ull<<i))) {
                continue;
            }
            if (!(currentWitch.castableMask & (1ull<<i))){
                continue;
            }
            auto cast = casts[i];
            if (not can(currentWitch.inv, cast.delta)){
                continue;
            }

            auto newWitch = currentWitch;
            newWitch.castableMask &= ~(1ull<<i);
            newWitch.inv = add(newWitch.inv, cast.delta);
            newWitch.turns++;
            newWitch.action = Action{aCast, cast.actionId, 1};

            if (prev.find(newWitch) == prev.end()){
                queue.push_back(newWitch);
                prev.insert(make_pair(newWitch, currentWitch));

                if (newWitch.score>maxScore || newWitch.score==maxScore && newWitch.turns<minTurns) {
                    maxScore = newWitch.score;
                    maxWitch = newWitch;
                    minTurns = newWitch.turns;
                }
            }
            if (cast.repeatable){
                int16_t times = 1;
                while (can(newWitch.inv, cast.delta)){
                    times++;

                    newWitch.castableMask &= ~(1ull<<i);
                    newWitch.inv = add(newWitch.inv, cast.delta);
                    newWitch.action = Action{aCast, cast.actionId, times};

                    if (prev.find(newWitch) == prev.end()){
                        queue.push_back(newWitch);
                        prev.insert(make_pair(newWitch, currentWitch));

                        if (newWitch.score>maxScore || newWitch.score==maxScore && newWitch.turns<minTurns) {
                            maxScore = newWitch.score;
                            maxWitch = newWitch;
                            minTurns = newWitch.turns;
                        }
                    }
                }
            }
        }

        auto newWitch = currentWitch;
        newWitch.castableMask = 9223372036854775807ull;
        newWitch.turns++;
        newWitch.action = Action{aRest, 0, 0};

        if (prev.find(newWitch) == prev.end()){
            queue.push_back(newWitch);
            prev.insert(make_pair(newWitch, currentWitch));

            if (newWitch.score>maxScore || newWitch.score==maxScore && newWitch.turns<minTurns) {
                maxScore = newWitch.score;
                maxWitch = newWitch;
                minTurns = newWitch.turns;
            }
        }
    }
    cerr << "# queue:" << queue.size() << " dict:" << prev.size() << endl;
    if (maxScore <= 0) {
        return result; // not fount -> empty
    }

    while (maxWitch != startWitch){
        auto a = maxWitch.action;
        string resultStr = "";
        if (a.type == aBrew) {
            resultStr += "BREW " + to_string(a.id);
        } else if (a.type == aLearn) {
            resultStr += "LEARN " + to_string(a.id);
        } else if (a.type == aCast) {
            resultStr += "CAST " + to_string(a.id) + " " + to_string(a.times);
        } else if (a.type == aRest) {
            resultStr += "REST";
        }
        resultStr += " ";
        result.push_back(resultStr
            + " score:" + to_string(maxScore) 
            + " nodes:" + to_string(prev.size())
            + " max turns:" + to_string(maxTurns));
        auto it = prev.find(maxWitch);
        if (it == prev.end()){
            throw runtime_error("key in prev not found");
        }
        maxWitch = it->second;
    }
    result.push_back("Start"); // len = turns + 1
    return result;
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

    unordered_map<Witch, Witch> prev;
    deque<Witch> queue;

    // game loop
    int turn = 0;
    while (1) {

        turn ++;
        int actionCount; // the number of spells and recipes in play
        int16_t actionId; // the unique ID of this spell or recipe
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
        auto start = currentMs();
        if (actionCount == 9999){
            break; //DEBUG
        }
        cerr << actionCount << endl;
        for (int i = 0; i < actionCount; i++) {
            string actionType; // in the first league: BREW; later: CAST, OPPONENT_CAST, LEARN, BREW
            int16_t delta0; // tier-0 ingredient change
            int16_t delta1; // tier-1 ingredient change
            int16_t delta2; // tier-2 ingredient change
            int16_t delta3; // tier-3 ingredient change
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
                struct Cast cast = {actionId, array<int16_t, 4>{delta0, delta1, delta2, delta3}, castable, repeatable};
                casts[castsSize] = cast;
                castsSize++;
            } else if (actionType == "BREW") {
                struct Brew brew = {actionId, array<int16_t, 4>{delta0, delta1, delta2, delta3}, price};
                brews.push_back(brew);
            } else if (actionType == "LEARN") {
                struct Learn learn = {actionId, array<int16_t, 4>{delta0, delta1, delta2, delta3}, 
                    tomeIndex, taxCount, repeatable};
                learns.push_back(learn);
            }
        }

        array<int16_t, 4> inv;
        for (int i = 0; i < 2; i++) {
            int16_t inv0; // tier-0 ingredients in inventory
            int16_t inv1;
            int16_t inv2;
            int16_t inv3;
            int score; // amount of rupees
            cin >> inv0 >> inv1 >> inv2 >> inv3 >> score; cin.ignore();
            cerr << inv0 << " " << inv1 << " " << inv2 << " " << inv3 << " " << score << endl;
            if (i == 0){
                inv = array<int16_t, 4>{inv0, inv1, inv2, inv3};
            }
        }

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
        // int brewId = -1;
        // for (int i = 0; i < brews.size(); i++) {
        //     bool canBrew = can(inv, brews[i].delta);
        //     if (canBrew) {
        //         brewId = brews[i].actionId;
        //         break;
        //     }
        // }
        // if (brewId > -1) {
        //     cout << "BREW " << brewId << " BREW!" << endl;
        //     continue;
        // }

        // 2. BFS
        Witch myWitch;
        myWitch.inv = inv;
        myWitch.castsMask = 0;
        myWitch.castableMask = 9223372036854775807ull;
        myWitch.score = 0;
        myWitch.turns = 0;
        myWitch.brewsRemaining.set();
        for (int i=0; i<64; i++) {
            if (casts[i].actionId > -1) {
                myWitch.castsMask |= (1ull<<i);
                if (not casts[i]._castable) {
                    myWitch.castableMask &= ~(1ull<<i);
                }
            }
        }

        prev.clear();
        queue.clear();
        auto result = bfs(myWitch, casts, brews, learns, start, (not debug), prev, queue);

        auto elapsed = currentMs() - start;
        if (result.size()>0){
            for(auto r:result){
                cerr << "# path - " << r << endl;
            }
            if (result.size()==1){
                throw runtime_error("Bfs returned path with len=1, shoed have brewed before...");
            }
            auto firstMove = result[result.size()-2];
            cout << firstMove << " T-" + to_string(result.size()-1) 
                + " " << setprecision(2) << elapsed << "ms";
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

    }
}

int main()
{
    prod();
    // test();
}
