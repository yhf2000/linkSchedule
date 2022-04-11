#include <bits/stdc++.h>

using namespace std;

namespace Const {

}


struct Node {
    double x, y, Power;

    Node() = default;

    Node(double x, double y) : x(x), y(y) {}

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

    Link() = default;

    Link(Node s, Node r) : Sender(s), Receiver(r) {}

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
};

struct SINR {
    double alpha, beta, N;

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


    void run() {


    }

};

int main() {


    return 0;
}