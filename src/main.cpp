#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

struct sKid
{
    std::vector<int> vSatis; // satisfaction derived from each candy type
    std::vector<int> vAward; // amount of each candy awarded
    int satisLeft;           // minimum total satisfaction, remaining
    int universal;           // universal award

    sKid()
        : universal(0)
    {
    }
    void award(int candy, int amount);
    int awardUniversal();
    bool isSatisfied() const
    {
        return satisLeft <= 0;
    }
    void display(int index) const;
};

struct sAwardValue
{
    int myCandy;
    int myKid;
    int mySatis;
    sAwardValue(int c, int k, int s)
        : myCandy(c), myKid(k), mySatis(s)
    {
    }
};

struct sProblem
{
    std::vector<sKid> myKids;
    std::vector<int> myCandy; // quantity of available candy
    std::vector<sAwardValue> myValue;
    int myTotalUniversal;

    // generate example problem posted to https://stackoverflow.com/q/79267233/16582
    void gen1();

    void solve();

    void display();

private:
    void gen(
        const std::vector<int> &quantity,
        const std::vector<std::vector<int>> &vKidSatis,
        const std::vector<int> vKidMin);
    void sortAwardValues();
    void awardFavorite();
    void awardSeconds();
    void awardUniversal();
    void award(int kid, int candy, int amount);
};

void sKid::award(int candy, int amount)
{
    vAward[candy] += amount;
    satisLeft -= vSatis[candy] * amount;
}
int sKid::awardUniversal()
{
    if (satisLeft <= 0)
        return 0;

    universal = satisLeft;
    satisLeft = 0;
    return universal;
}

void sProblem::gen1()
{
    /*
    q = [5, 2, 1]
s = [
  [2, 3, 1],
  [1, 2, 3]
]
c = [15, 10]
    */

    int kidCount = 2;
    int candyCount = 3; // does not include universal

    std::vector<int> quantity{5, 2, 1};
    std::vector<std::vector<int>> vKidSatis{{2, 3, 1}, {1, 2, 3}};
    std::vector<int> vKidMin{15, 10};

    gen(quantity, vKidSatis, vKidMin);
}
void sProblem::gen(
    const std::vector<int> &quantity,
    const std::vector<std::vector<int>> &vKidSatis,
    const std::vector<int> vKidMin)
{
    myCandy = quantity;
    for (int kid = 0; kid < vKidMin.size(); kid++)
    {
        sKid child;
        for (int candy = 0; candy < vKidSatis[0].size(); candy++)
            child.vSatis.push_back(vKidSatis[kid][candy]);
        child.satisLeft = vKidMin[kid];
        child.vAward.resize(vKidSatis[0].size());
        myKids.push_back(child);
    }
}

void sProblem::award(int kid, int candy, int amount)
{
    myKids[kid].award(candy, amount);
    myCandy[candy] -= amount;
}

void sProblem::solve()
{
    /*
     - Sort q[i][j] into decreasing value
     - LOOP i,j over sorted q[i][j]
        - Award s[j] of i to kid j up the the lessor of remaining i or c[j]
        - IF i exhausted
           BREAK out of LOOP i,j
     - ENDLOOP i,j
     - LOOP over J
        - IF J has NOT reached c[j]
           - LOOP i over sorted q[i][j]
              - IF q[i][j] > 1
                 - Award s[j] of i to kid j up the the lessor of remaining i or c[j]
              - IF c[j] reached
                  - BREAK out of LOOP i
            - IF c[j] reached
               - Break out of LOOP J
     - LOOP over J
        - IF c[j] not reached
             - AWARD J enough "universal" to reach c[J]
     - STOP
    */

    sortAwardValues();
    awardFavorite();
    awardSeconds();
    awardUniversal();
}

void sProblem::sortAwardValues()
{
    myValue.clear();
    for (int kid = 0; kid < myKids.size(); kid++)
    {
        for (int candy = 0; candy < myCandy.size(); candy++)
            myValue.emplace_back(candy, kid, myKids[kid].vSatis[candy]);
    }
    std::sort(myValue.begin(), myValue.end(),
              [](const sAwardValue &a, const sAwardValue &b)
              {
                  return a.mySatis > b.mySatis;
              });
}

void sProblem::awardFavorite()
{
    for (auto &value : myValue)
    {
        int remaingCandy = myCandy[value.myCandy];
        int dissatisfied = myKids[value.myKid].satisLeft;
        if (remaingCandy > 0)
        {
            if (dissatisfied > 0)
            {
                int amount = remaingCandy;
                if (amount > dissatisfied)
                    amount = dissatisfied;
                award(value.myKid, value.myCandy, amount);
            }
        }
    }
}
void sProblem::awardSeconds()
{
    //  for( auto& value : myValue )
    //  {

    //  }
}

void sProblem::awardUniversal()
{
    myTotalUniversal = 0;
    for (auto &kid : myKids)
        myTotalUniversal += kid.awardUniversal();
}

void sKid::display(int index) const
{
    std::cout << "Kid " << index;
    if (satisLeft <= 0)
        std::cout << " satisfied ";
    else
        std::cout << " please, sir, can I have more ";
    std::cout << "candies: ";
    for (int a : vAward)
    {
        std::cout << a << " ";
    }

    std::cout << " universal " << universal
              << "\n";
}

void sProblem::display()
{
    for (int kid = 0; kid < myKids.size(); kid++)
    {
        myKids[kid].display(kid);
    }
    std::cout << "Total Universal candies awarded "
        << myTotalUniversal << "\n";
}

main()
{
    sProblem theProblem;
    theProblem.gen1();
    theProblem.solve();
    theProblem.display();
    return 0;
}
