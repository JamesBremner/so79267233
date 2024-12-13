#include <stdexcept>
#include <iostream>
#include "GraphTheory.h"
#include "sProblem.h"

raven::graph::sGraphData gd;
std::vector<int> vEdgeFlow;

void sProblem::flow()
{
    flow_makeGraph();

    double total = flow_FFObjective(gd, vEdgeFlow);

    flow_report();
}

int kid_from_f_n_m(const std::string &label)
{
    int p = label.find("_");
    if (p < 0)
        return -1;
    return atoi(label.substr(p + 1).c_str());
}

void sProblem::flow_makeGraph()
{
    gd.g.directed();
    for (int candy = 0; candy < myCandy.size(); candy++)
    {
        auto sc = "c" + std::to_string(candy);
        gd.g.add("src", sc);
        gd.edgeWeight.push_back(myCandy[candy]);
        for (int kid = 0; kid < myKids.size(); kid++)
        {

            // candy of a type awarded to a kid  fn_m
            std::string sck = "f" + std::to_string(candy);
            sck += "_" + std::to_string(kid);
            gd.g.add(sc, sck);
            gd.edgeWeight.push_back(100);

            gd.g.add(sck, "snk");
            gd.edgeWeight.push_back(100);
        }
    }
    gd.startName = "src";
    gd.endName = "snk";
}

void sProblem::flow_report()
{
    std::vector<int> vkid(myKids.size(), 0);
    for (auto e : gd.g.edgeList())
    {
        std::cout << gd.g.userName(e.first)
                  << " " << gd.g.userName(e.second)
                  << " " << vEdgeFlow[gd.g.find(e.first, e.second)]
                  << "\n";
        int kid = kid_from_f_n_m(gd.g.userName(e.second));
        if (kid >= 0)
            vkid[kid] += vEdgeFlow[gd.g.find(e.first, e.second)];
    }
    for (int kid = 0; kid < vkid.size(); kid++)
        std::cout << "kid " << kid << " gets " << vkid[kid] << " candies\n";
}

double
sProblem::flow_FFObjective(
    raven::graph::sGraphData &gd,
    std::vector<int> &vEdgeFlow)
{
    if (!gd.g.isDirected())
        throw std::runtime_error(
            "Flow calculation needs directed graph ( 2nd input line must be 'g')");

    int totalFlow = 0;

    // construct residual network
    // for each link, add a reverse edge with zero weight
    auto residual = gd;
    for (auto ep : gd.g.edgeList())
    {
        int ei = residual.g.add(ep.second, ep.first);
        residual.edgeWeight.push_back(0);
    }

    while (1)
    {
        /* find shortest path with available capacity

        This is the Edmonds–Karp implementation of the Ford–Fulkerson method
        It uses breadth first searching so the paths are found in a defined order
        rather than a 'random' order depending on how the links are stored in the graph data structure

        https://en.wikipedia.org/wiki/Edmonds%E2%80%93Karp_algorithm

        https://theory.stanford.edu/~tim/w16/l/l1.pdf

        */

        auto p = bfsPath(residual);

        if (!p.size())
            break;

        // maximum flow through path
        int maxflow = INT_MAX;
        int u = -1;
        int v;
        for (int v : p)
        {
            if (u >= 0)
            {
                double cap = residual.edgeWeight[residual.g.find(u, v)];
                if (cap < maxflow)
                {
                    maxflow = cap;
                }
            }
            u = v;
        }

        // consume capacity of links in path
        u = -1;
        for (int v : p)
        {
            if (u >= 0)
            {
                // subtract flow from path link
                int ei = residual.g.find(u, v);
                residual.edgeWeight[ei] -= maxflow;

                // add flow to reverse edge
                ei = residual.g.find(v, u);
                residual.edgeWeight[ei] += maxflow;
            }
            u = v;
        }

        totalFlow += maxflow;
    }

    // calculate flow through each real edge
    vEdgeFlow.clear();

    // loop over real edges
    for (int ei = 0; ei < gd.g.edgeCount(); ei++)
    {
        // flow is capacity minus unused capacity
        vEdgeFlow.push_back(
            gd.edgeWeight[ei] - residual.edgeWeight[ei]);
    }

    return totalFlow;
}
