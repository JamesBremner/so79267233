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

    // generate example problem posted to https://stackoverflow.com/q/79267233/16582
    void gen1();

    // generate TID1 problem
    // https://github.com/JamesBremner/so79267233/issues/1
    void genTID1();

    // read file
    void read(const std::string fname);

    void solve();
    void milp();
    void solutionSpaceExplorer();

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

    void ssex_Generator();
    void ssex_Solver();
    void ssex_Reporter();
    void ssex_CandyConstraints();
    void ssex_SatisfyConstraints();
    void ssex_variables();
    void ssex_consts();
    void ssex_Objective();
};
