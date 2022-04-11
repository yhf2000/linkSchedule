#include <bits/stdc++.h>

using namespace std;

namespace Const {
    double c1 = 0, q = 0;
    double alpha = 2, beta = 1, N = 1;
    double Power = 1;
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
};

struct Link {
    Node Sender, Receiver;
    bool ReceiverGetMessage, SenderGetAck;

    Link() = default;

    Link(Node s, Node r) : Sender(s), Receiver(r), ReceiverGetMessage(false), SenderGetAck(false) {}

    double getDis() const {
        return (Sender - Receiver).dis();
    }

};

struct Gen {
    default_random_engine e;

    Gen() {
        e.seed(time(nullptr));
    }

    Node genSender(double minV, double maxV) {
        uniform_real_distribution<double> u(minV, maxV);
        return {u(e), u(e)};
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
    Link jammer;

    Network(int linkNumber, double minLink, double maxLink, int networkSize) {
        Gen g;
        for (int i = 0; i < linkNumber; i++) {
            auto nd = g.genSender(-networkSize / 2.0, networkSize / 2.0);
            auto lk = g.genLink(nd, minLink, maxLink);
            d.emplace_back(lk);
        }
//        auto nd = Node(0, 0);
    }


    int run() {
        Gen g;
        SINR sinr;
        for (auto &i: d) {
            i.ReceiverGetMessage = false;
            i.SenderGetAck = false;
        }
        int unFinishNumber = (int) d.size();
        int Round = 0;

        double q = 1.0 / 8;
        for (int k = 1; unFinishNumber > 0; q /= 2, k++) {

            Round ++;

            vector<Node> Senders;
            vector<int> idx;

            for (int i = 0; i < d.size(); i++) {
                if (!d[i].ReceiverGetMessage && g.choice(q)) {
                    Senders.emplace_back(d[i].Sender);
                    idx.emplace_back(i);
                }
            }
            for (int i = 0; i < d.size(); i++) {
                if (!d[i].ReceiverGetMessage) {
                    int p = sinr.listen(d[i].Receiver, Senders);
                    if (p != -1 && idx[p] == i) {
                        d[i].ReceiverGetMessage = true;
                    }
                }
            }

            Senders.clear();
            idx.clear();

            for (int i = 0; i < d.size(); i++) {
                if (d[i].ReceiverGetMessage && g.choice(q)) {
                    Senders.emplace_back(d[i].Receiver);
                    idx.emplace_back(i);
                }
            }
            for (int i = 0; i < d.size(); i++) {
                if (!d[i].SenderGetAck) {
                    int p = sinr.listen(d[i].Sender, Senders);
                    if (p != -1 && idx[p] == i) {
                        d[i].SenderGetAck = true;
                        unFinishNumber--;
                    }
                }
            }
        }
    }

};

int main() {
    Network network(200, 1, 20, 200);
    cout << network.run();
    return 0;
}