#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "cSolutionSpaceExplorer.h"

struct sKid
{
    std::vector<int> vSatis; // satisfaction derived from each candy type
    std::vector<int> vAward; // amount of each candy awarded
    int satisLeft;           // minimum total satisfaction, remaining
    int universal;           // universal award

    sKid();                            // CTOR
    void award(int candy, int amount); // award candy in a specified amount
    int awardUniversal();              // award enough universal to satisfy
    bool isSatisfied() const;          // true if satisfied
    void display(int index) const;     // display final result
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
    std::vector<std::string> myMilpDesign;

    // the solution explorer
    cSolutionSpaceExplorer ssex;

    // generate example problem posted to https://stackoverflow.com/q/79267233/16582
    void gen1();

    // generate TID1 problem
    // https://github.com/JamesBremner/so79267233/issues/1
    void genTID1();

    // read file
    void read(const std::string fname);

    void solve();
    void milp();
    void SolutionSpaceExplorerGenerator();

    void display();

private:
    void gen(
        const std::vector<int> &quantity,
        const std::vector<std::vector<int>> &vKidSatis,
        const std::vector<int> vKidMin);
    void sortAwardValues();
    void awardFavorite();
    void awardUniversal();
    void award(int kid, int candy, int amount);
    bool allSatisfied();
    bool allgone();

    // generate MILP objective and constraints
    void milpDesign();
    void ssex_CandyConstraints();
    void ssex_SatisfyConstraints();
    void ssex_variables();
    void ssex_consts();
    void ssex_objective();
};

sKid::sKid()
    : universal(0)
{
}

bool sKid::isSatisfied() const
{
    return satisLeft <= 0;
}
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

void sProblem::genTID1()
{
    std::vector<int> quantity{1, 1};
    std::vector<std::vector<int>> vKidSatis{{15, 15}, {10, 1}};
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
 - LOOP until all satisfied or no more candies
    - LOOP i,j over sorted q[i][j]
       - Award s[j] of i to kid j up the the lessor of remaining i or c[j]
        - IF i exhausted
           - BREAK out of LOOP i,j
    - ENDLOOP i,j
    - IF all satisfied or no more candies
        - BREAK out of loop
 - ENDLOOP until all satisfied or no more candies

 - LOOP over J
    - IF c[j] not reached
         - AWARD J enough "universal" to reach c[J]
 - STOP
    */

    sortAwardValues();
    awardFavorite();
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
    while (true)
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
        if (allSatisfied())
            break;
        if (allgone())
            break;
    }
}

void sProblem::awardUniversal()
{
    myTotalUniversal = 0;
    for (auto &kid : myKids)
        myTotalUniversal += kid.awardUniversal();
}

bool sProblem::allSatisfied()
{
    for (auto &kid : myKids)
        if (!kid.isSatisfied())
            return false;
    return true;
}
bool sProblem::allgone()
{
    for (int candy : myCandy)
        if (candy > 0)
            return false;
    return true;
}

void sProblem::milp()
{
    milpDesign();
}

void sProblem::milpDesign()
{

    std::stringstream ss;
    ss << "Maximize ";
    for (int kid = 0; kid < myKids.size(); kid++)
    {
        auto sk = std::to_string(kid);
        for (int candy = 0; candy < myCandy.size(); candy++)
        {
            auto sc = std::to_string(candy);
            if (!(kid == 0 && candy == 0))
                ss << " + ";
            ss << "f" << sc << sk
               << " * "
               << std::to_string(myKids[kid].vSatis[candy]);
        }
    }
    myMilpDesign.push_back(ss.str());
    ss.str("");

    // constraints for candy quantities
    for (int candy = 0; candy < myCandy.size(); candy++)
    {
        auto sc = std::to_string(candy);
        for (int kid = 0; kid < myKids.size(); kid++)
        {
            auto sk = std::to_string(kid);
            if (kid != 0)
                ss << " + ";
            ss << "f" << sc << sk;
        }
        ss << " <= " << std::to_string(myCandy[candy]);

        myMilpDesign.push_back(ss.str());
        ss.str("");
    }

    for (int kid = 0; kid < myKids.size(); kid++)
    {
        auto sk = std::to_string(kid);
        for (int candy = 0; candy < myCandy.size(); candy++)
        {
            if (candy != 0)
                ss << " + ";
            auto sc = std::to_string(candy);
            ss << "f" << sc << sk
               << " * " << std::to_string(myKids[kid].vSatis[candy]);
        }
        ss << " <= " << std::to_string(myKids[kid].satisLeft);

        myMilpDesign.push_back(ss.str());
        ss.str("");
    }

    for (auto &s : myMilpDesign)
        std::cout << s << "\n\n";
}

void sProblem::ssex_variables()
{
    std::vector<std::string> ret;

    for (int candy = 0; candy < myCandy.size(); candy++)
    {
        auto sc = std::to_string(candy);
        for (int kid = 0; kid < myKids.size(); kid++)
        {
            auto sk = std::to_string(kid);

            std::stringstream ss;
            ss << "f" << sc << sk;

            ret.push_back(ss.str());
        }
    }
    ssex.variables(ret, 10);
}
void sProblem::ssex_consts()
{
    for (int candy = 0; candy < myCandy.size(); candy++)
    {
        auto sc = std::to_string(candy);
        for (int kid = 0; kid < myKids.size(); kid++)
        {
            auto sk = std::to_string(kid);

            std::stringstream ss;
            ss << "s" << sc << sk;

            ssex.consts(ss.str(), myKids[kid].vSatis[candy]);
        }
    }
}

void sProblem::ssex_objective()
{
    std::stringstream ss;
    for (int candy = 0; candy < myCandy.size(); candy++)
    {
        auto sc = std::to_string(candy);
        for (int kid = 0; kid < myKids.size(); kid++)
        {
            auto sk = std::to_string(kid);
            if (!(candy == 0 && kid == 0))
                ss << " + ";
            ss << "f" << sc << sk
               << " * s" << sc << sk;
           
        }
    }

    ssex.objective(ss.str());
}

void sProblem::SolutionSpaceExplorerGenerator()
{
    try {
    ssex_variables();
    ssex_consts();
    ssex_objective();
    ssex_CandyConstraints();
    ssex_SatisfyConstraints();

        // parse the input
    ssex.parse();

    // find optimum assignment using exhaustive search
    ssex.search(1);

    // get optimal solution
    double ov = ssex.objective();

    }
    catch( std::runtime_error& e )
    {
        std::cout << "ssex threw: " << e.what() << "\n";
        exit(1);
    }
}

void sProblem::ssex_CandyConstraints()
{
    std::stringstream ss;
    for (int candy = 0; candy < myCandy.size(); candy++)
    {
        //ss << "(\"";
        auto sc = std::to_string(candy);
        for (int kid = 0; kid < myKids.size(); kid++)
        {
            auto sk = std::to_string(kid);

            if (kid != 0)
                ss << " + ";
            ss << "f" << sc << sk;
        }
        ss << " <= " << std::to_string(myCandy[candy]);
           //<< " \"";

        ssex.constraint(ss.str() );
    }
}

void sProblem::ssex_SatisfyConstraints()
{
    /* There is no need to gave any child more candy
    than will give it the maximum possible satisfaction.

    Sum the product of each candy type anount time the satisfaction per candy
    and limit to the total satisfaction obtainable by each kid
    */

   std::cout << "Satisfaction constraints\n";

    // loop over the kids
    for (int kid = 0; kid < myKids.size(); kid++)
    {
        // construct string to hold the constraint
        std::string ss;

        auto sk = std::to_string(kid);

        // loop over candy types
        for (int candy = 0; candy < myCandy.size(); candy++)
        {
            auto sc = std::to_string(candy);

            // add to sum of previous products
            if (candy != 0)
                ss += " + ";

            // amount of candy to child
            ss += "f" + sc + sk;

            // multiply amount by satisfaction
            ss += " * ";

            // satisfaction per candy
            ss += "s" + sc + sk;

        } // end loop over each candy type

        // maximum satisfaction obtainable
        ss += " <= " + std::to_string(myKids[kid].satisLeft);

        // add constraint to solution explorer
        ssex.constraint(ss);

        // display constraint
        std::cout << ss << "\n";

    }   // end loop over each child
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
    //theProblem.genTID1();

    theProblem.milp();
    theProblem.SolutionSpaceExplorerGenerator();

    theProblem.solve();
    theProblem.display();
    return 0;
}
