


#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

// https://github.com/JamesBremner/SolutionSpaceExplorer
#include "cSolutionSpaceExplorer.h"

#include "sProblem.h"

// the solution explorer
cSolutionSpaceExplorer ssex;

void sProblem::ssex_variables()
{
    // The variable values can be altered
    // to maximise the objective function
    // within the constrints

    std::vector<std::string> vv; // vector of variable names

    int max = 0; // maximum value of any variable

    for (int candy = 0; candy < myCandy.size(); candy++)
    {
        auto sc = std::to_string(candy);
        for (int kid = 0; kid < myKids.size(); kid++)
        {
            auto sk = std::to_string(kid);

            std::stringstream ss;
            ss << "f" << sc << sk;

            vv.push_back(ss.str());
        }

        if (myCandy[candy] > max)
            max = myCandy[candy];
    }

    // specify variable names
    ssex.variables(
        vv,   // vector of variable names
        max); // maximum amount of any candy
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

void sProblem::ssex_Objective()
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

void sProblem::solutionSpaceExplorer()
{
    try
    {
        ssex_Generator();
        ssex_Solver();
        ssex_Reporter();
    }
    catch (std::runtime_error &e)
    {
        std::cout << "ssex threw: " << e.what() << "\n";
        exit(1);
    }
}

void sProblem::ssex_Generator()
{
    ssex_variables();
    ssex_consts();
    ssex_Objective();
    ssex_CandyConstraints();
    ssex_SatisfyConstraints();
}

void sProblem::ssex_Solver()
{
    // parse the input
    ssex.parse();

    // find optimum assignment using exhaustive search
    // TODO: this is sufficient for small problems,
    // large problems will need the integer linear programming algorithm

    ssex.search(1);
}

void sProblem::ssex_Reporter()
{
    std::vector<int> vs(myKids.size(), 0);
    std::vector<int> vu(myKids.size(), 0);
    std::vector<int> vc(myCandy.size(), 0);
    int utotal = 0;
    for (auto &ov : ssex.optVarNameValue())
    {
        // std::cout << ov.first << " " << ov.second << "\n";
        int candy, kid;
        candy = atoi(ov.first.substr(1, 1).c_str());
        kid = atoi(ov.first.substr(2, 1).c_str());

        std::cout
            << "Kid " << kid
            << " gets " << ov.second
            << " of candy " << candy
            << "\n";

        vs[kid] += myKids[kid].vSatis[candy] * ov.second;
        vc[candy] += ov.second;
    }
    std::cout << "kid's satifaction\n";
    for (int s : vs)
        std::cout << s << " ";
    std::cout << "\nCandy amounts\n";
    for (int c : vc)
        std::cout << c << " ";
    std::cout << "\n";

    // complete every child's satisfaction by awarding universal candies
    for (int kid = 0; kid < myKids.size(); kid++)
    {
        if (vs[kid] < myKids[kid].satisLeft)
        {
            vu[kid] = myKids[kid].satisLeft - vs[kid];
            utotal += vu[kid];
        }
    }
    std::cout << "Total universal " << utotal << "\n";
}

void sProblem::ssex_CandyConstraints()
{
    for (int candy = 0; candy < myCandy.size(); candy++)
    {
        std::stringstream ss;
        auto sc = std::to_string(candy);
        for (int kid = 0; kid < myKids.size(); kid++)
        {
            auto sk = std::to_string(kid);

            if (kid != 0)
                ss << " + ";
            ss << "f" << sc << sk;
        }
        ss << " <= " << std::to_string(myCandy[candy]);

        ssex.constraint(ss.str());
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

    } // end loop over each child
}
