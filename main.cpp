#include <bits/stdc++.h>
#include "ThreadPool.h"

using namespace std;

namespace Const {
    double c1 = 2;
    double alpha = 2, beta = 50, N = 0;
    double Power = 1;
    int ClusterNumber = 6;
    double ClusterR = 10;
}


struct Node {
    double x, y, Power;

    Node() = default;

    Node(double x, double y) : x(x), y(y), Power(Const::Power) {}

    Node(double x, double y, double Power) : x(x), y(y), Power(Power) {}

    Node operator-(const Node &t) const {
        return {x - t.x, y - t.y};
    }

    Node operator+(const Node &t) const {
        return {x + t.x, y + t.y};
    }

    Node operator*(const double &v) const {
        return {x * v, y * v};
    }

    double dis() const {
        return sqrt(x * x + y * y);
    }

    Node Normalized() const {
        return {x / dis(), y / dis()};
    }

    bool inA(double X, double Y) {
        return X <= x * 2 && x * 2 <= Y && X <= y * 2 && y * 2 <= Y;
    }
};

struct Link {
    Node Sender, Receiver;
    bool ReceiverGetMessage, SenderGetAck;

    Link() = default;

    Link(Node s, Node r) : Sender(s), Receiver(r), ReceiverGetMessage(false), SenderGetAck(false) {}

    double getDis() const {
        return (Sender - Receiver).dis();
    }

    bool inA(double x, double y) {
        return Sender.inA(x, y) && Receiver.inA(x, y);
    }

};

struct Gen {
    default_random_engine e;

    Gen() {
        e.seed(time(nullptr));
    }

    void seed(unsigned long long x) {
        e.seed(x);
    }

    Node genNode(double minV, double maxV) {
        uniform_real_distribution<double> u(minV, maxV);
        return {u(e), u(e)};
    }

    Node genNode(Node nd, double minLen, double maxLen) {
        uniform_real_distribution<double> pu(-1, 1);
        uniform_real_distribution<double> lu(minLen, maxLen);
        return nd + Node(pu(e), pu(e)).Normalized() * lu(e);
    }

    Link genLink(Node Sender, double minLength, double maxLength, bool r = false) {
        uniform_real_distribution<double> lu(minLength, maxLength);
        uniform_real_distribution<double> pu(-1, 1);
        if (r) return {Sender + Node(pu(e), pu(e)).Normalized() * lu(e), Sender};
        return {Sender, Sender + Node(pu(e), pu(e)).Normalized() * lu(e)};
    }

    int getInt(int minNumber, int maxNumber) {
        uniform_int_distribution<int> rd(minNumber, maxNumber);
        return rd(e);
    }

    bool choice(double p) {
        uniform_real_distribution<double> u(0, 1);
        return u(e) <= p;
    }
};


struct SINR {
    double alpha, beta, N;

    SINR() : alpha(Const::alpha), beta(Const::beta), N(Const::N) {}

    SINR(double alpha) : alpha(alpha < 0 ? Const::alpha : alpha), beta(Const::beta), N(Const::N) {}

    int listen(Node &Receiver, vector<Node> &Senders) const {
        double sum = 0;
        for (auto &s: Senders) sum += s.Power / pow((s - Receiver).dis(), alpha);
        vector<int> res;
        for (int i = 0; i < Senders.size(); i++) {
            double d = Senders[i].Power / pow((Senders[i] - Receiver).dis(), alpha);
            if (d / (sum - d + N) >= beta) res.emplace_back(i);
        }
        if (res.size() != 1) return -1;
        return res.front();
    }

};

struct Network {

    vector<Link> d;
    vector<Node> Cluster;
    Link jammer{};

    int linkNumber, networkSize;
    double minLink, maxLink, ClusterR;
    bool useJammer, useClustered;
    double jammerMaxLength{}, jammerMinLength{};


    Network(int linkNumber,
            double minLink,
            double maxLink,
            int networkSize,
            double jammerMaxLength,
            double jammerMinLength,
            bool useClustered = false,
            bool useBoth = false,
            double ClusterR = Const::ClusterR) :
            linkNumber(linkNumber),
            minLink(minLink),
            maxLink(maxLink),
            networkSize(networkSize),
            jammerMaxLength(jammerMaxLength),
            jammerMinLength(jammerMinLength),
            useJammer(true),
            useClustered(useClustered),
            ClusterR(ClusterR) {
        if (useBoth) {
            initLinkData();
            initClusteredLinkData();
        } else {
            if (!useClustered)initLinkData();
            else initClusteredLinkData();
        }
        initJammer();
    }

    Network(int linkNumber,
            double minLink,
            double maxLink,
            int networkSize,
            bool useClustered = false,
            bool useBoth = false,
            double ClusterR = Const::ClusterR) :
            linkNumber(linkNumber),
            minLink(minLink),
            maxLink(maxLink),
            networkSize(networkSize),
            useJammer(false),
            useClustered(useClustered),
            ClusterR(ClusterR) {
        if (useBoth) {
            initLinkData();
            initClusteredLinkData();
        } else {
            if (!useClustered)initLinkData();
            else initClusteredLinkData();
        }
    }

    void initLinkData() {
        Gen g;
        g.seed((unsigned long long) (void *) (&g));
        for (int i = 0; i < linkNumber; i++) {
            auto nd = g.genNode(-networkSize / 2.0, networkSize / 2.0);
            auto lk = g.genLink(nd, minLink, maxLink);
            if (!lk.inA(-networkSize, networkSize)) {
                i--;
                continue;
            }
            if (useJammer) {
                if (lk.Sender.dis() <= jammerMaxLength || lk.Receiver.dis() <= jammerMaxLength) {
                    i--;
                    continue;
                }
            }
            d.emplace_back(lk);
        }
    }

    void initClusteredLinkData() {
        Gen g;
        g.seed((unsigned long long) (void *) (&g));

        // ?????? Cluster ??????
        for (int i = 1; i <= Const::ClusterNumber; i++) {
            auto cen = g.genNode(-networkSize / 2.0, networkSize / 2.0);
            if (!cen.inA(-networkSize + ClusterR * 2, networkSize - ClusterR * 2)) {
                i--;
                continue;
            }
            bool flag = true;
            for (auto &x: Cluster) {
                if ((x - cen).dis() <= ClusterR * 2) {
                    flag = false;
                    break;
                }
            }
            if (!flag) {
                i--;
                continue;
            }
            if (useJammer) {
                if (cen.dis() <= jammerMaxLength + ClusterR) {
                    i--;
                    continue;
                }
            }
            Cluster.emplace_back(cen);
        }

        // ??????????????????
        int nEach = networkSize / Const::ClusterNumber;

        for (auto &ndc: Cluster) {
            for (int i = 1; i <= nEach; i++) {
                auto nd = g.genNode(ndc, minLink, min(maxLink, ClusterR));
                auto lk = g.genLink(nd, minLink, maxLink);

                if ((lk.Sender - ndc).dis() > ClusterR || (lk.Receiver - ndc).dis() > ClusterR) {
                    i--;
                    continue;
                }
                d.emplace_back(lk);
            }
        }
    }

    void initJammer() {
        Gen g;
        g.seed((unsigned long long) (void *) (&g));

        auto nd = Node(0, 0);
        jammer = g.genLink(nd, 1, jammerMinLength, true);
    }


    void print(ofstream &o) {
        o << d.size() << endl;
        for (auto &i: d) {
            o << i.Sender.x << " " << i.Sender.y << " " << i.Receiver.x << " " << i.Receiver.y << "\n";
        }
    }

    auto anotherLinkSchedulingAlgorithmRun(double alpha = -1) {
        Gen g;
        g.seed((unsigned long long) (void *) (&g));

        SINR sinr(alpha);
        double p_hat = 1.0 / 2 *
                       pow((pow(sinr.beta, 1 / sinr.alpha) - 2)
                           / (pow(sinr.beta, 1 / sinr.alpha) + 2),
                           sinr.alpha
                       );
        double sigma = 96 * (1 / p_hat), delta = 64 * exp(1) * (1 / p_hat);
        double Ie = sigma * log2(linkNumber), Pc = 1 / (2 * sigma * log2(linkNumber));

        int unFinishNumber = (int) d.size();
        int roundCount = 0;

        while (unFinishNumber >= 1) {
            for (int i = 1; i <= ceil(log2(maxLink)); i++) {
                vector<Link *> lk;
                map<int, vector<Link *>> mp;
                int mxRound = 0;
                // ????????????????????? Link
                for (auto &link: d) {
                    double len = (link.Sender - link.Receiver).dis();
                    if ((1 << (i - 1)) <= len && len < (1 << (i))) {
                        lk.emplace_back(&link);
                        double A = Ie;
                        mxRound = max(mxRound, (int) ceil(delta * A));
                        if (!link.SenderGetAck) {
                            while (A > sigma * log2(linkNumber)) {
                                int delay = g.getInt(1, ceil(delta * A));
                                mp[delay].emplace_back(&link);
                                A = (1 - p_hat / 8) * A;
                            }
                        }
                    }
                }
                roundCount += mxRound;

                for (auto &mpItem: mp) {
                    vector<Node> Senders;
                    // Slot 1: sv transmit
                    for (auto x: mpItem.second) {
                        if (!x->SenderGetAck) {
                            Senders.emplace_back(x->Sender);
                        }
                    }
                    int cnt = 0;
                    for (auto x: mpItem.second) {
                        if (!x->SenderGetAck) {
                            auto res = sinr.listen(x->Receiver, Senders);
                            if (res == cnt) x->ReceiverGetMessage = true;
                            cnt++;
                        }
                    }

                    vector<int> idx;
                    Senders.clear();
                    for (int j = 0; j < mpItem.second.size(); j++) {
                        auto x = mpItem.second[j];
                        if (x->ReceiverGetMessage && g.choice(p_hat)) {
                            Senders.emplace_back(x->Receiver);
                            idx.emplace_back(j);
                        }
                        x->ReceiverGetMessage = false;
                    }

                    for (int j = 0; j < mpItem.second.size(); j++) {
                        auto x = mpItem.second[j];
                        if (!x->SenderGetAck) {
                            int res = sinr.listen(x->Sender, Senders);
                            if (res != -1 && idx[res] == j) {
                                x->SenderGetAck = true;
                                unFinishNumber--;
                            }
                        }
                    }
                }

                int round = (int) (24.0 / p_hat * sigma * log2(linkNumber) * log2(linkNumber));
                roundCount += round;
                while (round--) {
                    vector<int> idx;
                    vector<Node> Sender;
                    for (int j = 0; j < lk.size(); j++) {
                        auto x = lk[j];
                        if (!x->SenderGetAck && g.choice(Pc)) {
                            Sender.emplace_back(x->Sender);
                            idx.emplace_back(j);
                        }
                    }

                    for (int j = 0; j < lk.size(); j++) {
                        auto x = lk[j];
                        if (!x->SenderGetAck) {
                            int res = sinr.listen(x->Receiver, Sender);
                            if (res != -1 && idx[res] == j) {
                                x->ReceiverGetMessage = true;
                            }
                        }
                    }

                    Sender.clear();
                    idx.clear();

                    for (int j = 0; j < lk.size(); j++) {
                        auto x = lk[j];
                        if (x->ReceiverGetMessage && g.choice(Pc)) {
                            Sender.emplace_back(x->Receiver);
                            idx.emplace_back(j);
                        }
                        x->ReceiverGetMessage = false;
                    }

                    for (int j = 0; j < lk.size(); j++) {
                        auto x = lk[j];
                        if (!x->SenderGetAck) {
                            int res = sinr.listen(x->Sender, Sender);
                            if (res != -1 && idx[res] == j) {
                                x->SenderGetAck = true;
                                unFinishNumber--;
                            }
                        }
                    }
                }
            }
            Ie = 2 * Ie;
        }
        return make_tuple(roundCount, unFinishNumber);
    }


    auto run(double alpha = -1) {
        Gen g;
        g.seed((unsigned long long) (void *) (&g));

        SINR sinr(alpha);
        for (auto &i: d) {
            i.ReceiverGetMessage = false;
            i.SenderGetAck = false;
        }
        int unFinishNumber = (int) d.size();
        int Round = 0, lastRound = 0;

        double q = 1.0 / 8;
        for (int k = 1; unFinishNumber > 0 && q >= 1.0 / (4 * linkNumber); q /= 2, k++) {

            for (int it = 0; it < 8 / q * Const::c1 * log(d.size()) && unFinishNumber > 0; it++) {

                // Algorithm end if no link finished in 200 round
                if (Round - lastRound > 1000 ||
                    (Round - lastRound > 600 && unFinishNumber / (double) linkNumber < 0.005) ||
                    (Round - lastRound > 800 && unFinishNumber / (double) linkNumber < 0.008)) {
                    return make_tuple(Round, unFinishNumber);
                }

                Round++;

                vector<Node> Senders;
                vector<int> idx;
                if (useJammer) {
                    Senders.emplace_back(jammer.Sender);
                    idx.emplace_back(-2);
                }

                for (int i = 0; i < d.size(); i++) {
                    if (!d[i].ReceiverGetMessage && g.choice(q)) {
                        Senders.emplace_back(d[i].Sender);
                        idx.emplace_back(i);
                    }
                }
                for (int i = 0; i < d.size(); i++) {
                    if (!d[i].ReceiverGetMessage) {
                        int p = sinr.listen(d[i].Receiver, Senders);
                        if (p == -2) assert(false);
                        if (p != -1 && idx[p] == i) {
                            d[i].ReceiverGetMessage = true;
                        }
                    }
                }

                Senders.clear();
                idx.clear();

                if (useJammer) {
                    Senders.emplace_back(jammer.Sender);
                    idx.emplace_back(-2);
                }

                for (int i = 0; i < d.size(); i++) {
                    if (d[i].ReceiverGetMessage && g.choice(q)) {
                        Senders.emplace_back(d[i].Receiver);
                        idx.emplace_back(i);
                    }
                    //// TODO
                    d[i].ReceiverGetMessage = false;
                }
                for (int i = 0; i < d.size(); i++) {
                    if (!d[i].SenderGetAck) {
                        int p = sinr.listen(d[i].Sender, Senders);
                        if (p == -2) assert(false);
                        if (p != -1 && idx[p] == i) {
                            d[i].SenderGetAck = true;
                            lastRound = Round;
                            unFinishNumber--;
                        }
                    }
                }
            }

        }
        return make_tuple(Round, unFinishNumber);
    }

};


int main() {

    const int Rept = 1, PoolSize = 56, RInT = 1;
    // ??????????????? jammer ????????????????????????
    const double jammerMaxLength = 20, jammerMinLength = 2;

    auto fun = [&]
            (ofstream &out,
             int n,
             double alpha,
             double minLink,
             double maxLink,
             int networkSize,
             bool useClustered,
             double ClusterR,
             bool useBoth
            ) -> void {
        double res_wj = 0, res = 0;
        double fail_wj = 0, fail = 0;

        vector<future<tuple<double, double>>> B;
        ThreadPool poolB(PoolSize);
        for (int rep = 0; rep < Rept; rep++) {
            B.emplace_back(poolB.enqueue([&] {
                Network networkWithJammer(n,
                                          minLink,
                                          maxLink,
                                          networkSize,
                                          jammerMaxLength,
                                          jammerMinLength,
                                          useClustered,
                                          useBoth,
                                          ClusterR);
                pair<double, double> rs;
                for (int i = 1; i <= RInT; i++) {
                    auto r = networkWithJammer.run(alpha);
                    rs.first += get<0>(r);
                    rs.second += get<1>(r);
                }
                rs.first /= RInT, rs.second /= RInT;

                return make_tuple(rs.first * 1.0 / Rept, rs.second / (1.0 * n) / Rept);
            }));
        }
        for (auto &&it: B) {
            auto itt = it.get();
            res_wj += get<0>(itt);
            fail_wj += get<1>(itt);
        }
        vector<future<tuple<double, double>>> C;
        ThreadPool poolC(PoolSize);
        for (int rep = 0; rep < Rept; rep++) {
            C.emplace_back(poolC.enqueue([&] {
                Network network(n,
                                minLink,
                                maxLink,
                                networkSize,
                                useClustered,
                                useBoth,
                                ClusterR);
                pair<double, double> rs;
                for (int i = 1; i <= RInT; i++) {
                    auto r = network.run(alpha);
                    rs.first += get<0>(r);
                    rs.second += get<1>(r);
                }
                rs.first /= RInT, rs.second /= RInT;

                return make_tuple(rs.first * 1.0 / Rept, rs.second / (1.0 * n) / Rept);
            }));
        }
        for (auto &&it: C) {
            auto itt = it.get();
            res += get<0>(itt);
            fail += get<1>(itt);
        }
        cerr << endl;
        out << n << " " << res_wj << " " << res << " " << fail_wj << " " << fail << "\n";
    };

    auto fun2 = [&]
            (ofstream &out,
             int n,
             double alpha,
             double minLink,
             double maxLink,
             int networkSize,
             bool useClustered,
             double ClusterR,
             bool useBoth
            ) -> void {
        double res_wj = 0, res = 0;
        double fail_wj = 0, fail = 0;

//        vector<future<tuple<double, double>>> C;
//        ThreadPool poolC(PoolSize);
//        for (int rep = 0; rep < Rept; rep++) {
//            C.emplace_back(poolC.enqueue([&] {
                Network network(n,
                                minLink,
                                maxLink,
                                networkSize,
                                useClustered,
                                useBoth,
                                ClusterR);
                pair<double, double> rs;
                for (int i = 1; i <= RInT; i++) {
                    auto r = network.anotherLinkSchedulingAlgorithmRun(alpha);
                    rs.first += get<0>(r);
                    rs.second += get<1>(r);
                }
                rs.first /= RInT, rs.second /= RInT;

//                return make_tuple(rs.first * 1.0 / Rept, rs.second / (1.0 * n) / Rept);
//            }));
//        }
//        for (auto &&it: C) {
//            auto itt = it.get();
//            res += get<0>(itt);
//            fail += get<1>(itt);
//        }
        cerr << endl;
        out << n << " " << res_wj << " " << res << " " << fail_wj << " " << fail << "\n";
    };

//    ofstream out1(R"(data1.txt)");
//    for (int n = 200; n <= 400; n += 50) {
//        cerr << n << endl;
//        fun(out1,
//            n,
//            3,
//            1,
//            20,
//            200,
//            false,
//            10,
//            false);
//    }

    ofstream out(R"(data.txt)");
    for (int n = 200; n <= 400; n += 50) {
        cerr << n << endl;
        fun2(out,
            n,
            Const::alpha,
            1,
            20,
            200,
            false,
            Const::ClusterR,
            false);
    }
    return 0;
}