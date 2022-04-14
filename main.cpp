#include <bits/stdc++.h>
#include "ThreadPool.h"

using namespace std;

namespace Const {
    double c1 = 2;
    double alpha = 3, beta = 1.2, N = 0;
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
        return -X <= x * 2 && x * 2 <= X && -Y <= y * 2 && y * 2 <= Y;
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
        if (!useClustered)initLinkData();
        else initClusteredLinkData();
        initJammer();
    }

    Network(int linkNumber,
            double minLink,
            double maxLink,
            int networkSize,
            bool useClustered = false,
            double ClusterR = Const::ClusterR) :
            linkNumber(linkNumber),
            minLink(minLink),
            maxLink(maxLink),
            networkSize(networkSize),
            useJammer(false),
            useClustered(useClustered),
            ClusterR(ClusterR) {
        if (!useClustered)initLinkData();
        else initClusteredLinkData();

    }

    void initLinkData() {
        Gen g;
        for (int i = 0; i < linkNumber; i++) {
            auto nd = g.genNode(-networkSize / 2.0, networkSize / 2.0);
            auto lk = g.genLink(nd, minLink, maxLink);
            if (!lk.inA(networkSize, networkSize)) {
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

        // 获取 Cluster 的圈
        for (int i = 1; i <= Const::ClusterNumber; i++) {
            auto cen = g.genNode(-networkSize / 2.0, networkSize / 2.0);
            if (!cen.inA(-networkSize / 2.0 + ClusterR, networkSize / 2.0 - ClusterR)) {
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

        // 在圈内生成点
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
        auto nd = Node(0, 0);
        jammer = g.genLink(nd, 1, jammerMinLength, true);
    }


    void print(ofstream &o) {
        o << d.size() << endl;
        for (auto &i: d) {
            o << i.Sender.x << " " << i.Sender.y << " " << i.Receiver.x << " " << i.Receiver.y << "\n";
        }
    }


    auto run(double alpha = -1) {
        Gen g;
        SINR sinr(alpha);
        for (auto &i: d) {
            i.ReceiverGetMessage = false;
            i.SenderGetAck = false;
        }
        int unFinishNumber = (int) d.size();
        int Round = 0, lastRound = 0;

        double q = 1.0 / 8;
        for (int k = 1; unFinishNumber > 0; q /= 2, k++) {

            for (int it = 0; it < 8 / q * Const::c1 * log(d.size()) && unFinishNumber > 0; it++) {

                // Algorithm end if no link finished in 200 round
                if (Round - lastRound > 200) {
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

    ofstream out(R"(dataX.txt)");

    int Rept = 1, PoolSize = 56, RInT = 100;

    for (int n = 200; n <= 400; n += 10) {
        cerr << n << endl;
        double res_wj = 0, res = 0;
        double fail_wj = 0, fail = 0;

        vector<future<tuple<double, double>>> B;
        ThreadPool poolB(PoolSize);
        for (int rep = 0; rep < Rept; rep++) {
            B.emplace_back(poolB.enqueue([&] {
//                cerr << "*";
                cerr.flush();
                Network networkWithJammer(n, 1, 20, 200, 20, 2, false);
                pair<double, double> rs;
                for (int i = 1; i <= RInT; i++) {
                    auto r = networkWithJammer.run();
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
//        for (int i = 1; i <= Rept; i++) {
//            cerr << "*";
//            cerr.flush();
//            Network networkWithJammer(n, 1, 20, 200, 20, 2);
//            auto r = networkWithJammer.run();
//            fail_wj += get<1>(r) / 1.0 * n;
//            res_wj += get<0>(r);
//        }
//        res_wj /= Rept, fail_wj /= Rept;

        vector<future<tuple<double, double>>> C;
        ThreadPool poolC(PoolSize);
        for (int rep = 0; rep < Rept; rep++) {
            C.emplace_back(poolC.enqueue([&] {
//                cerr << "*";
                cerr.flush();
                Network network(n, 1, 20, 200, false);
                pair<double, double> rs;
                for (int i = 1; i <= RInT; i++) {
                    auto r = network.run();
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
//        for (int i = 1; i <= Rept; i++) {
//            cerr << "*";
//            cerr.flush();
//            Network network(n, 1, 20, 200);
//            auto r = network.run();
//            fail += get<1>(r) / 1.0 * n;
//            res += get<0>(r);
//        }
//        res /= Rept, fail /= Rept;

        cerr << endl;
        out << n << " " << res_wj << " " << res << " " << fail_wj << " " << fail << "\n";
    }
    return 0;
}