#include <bits/stdc++.h>

using namespace std;


struct SINR {
    double alpha, beta;

};


struct Node {
    double x, y, Power;

    Node(double x, double y) : x(x), y(y) {}

    Node(double x, double y, double Power) : x(x), y(y), Power(Power) {}

    Node operator-(const Node &t) const {
        return {x - t.x, y - t.y};
    }

    Node operator+(const Node &t) const {
        return {x + t.x, y + t.y};
    }

    Node operator * (const double &v) const{
        return {x * v, y * v};
    }

    double dis() const{
        return sqrt(x * x + y * y);
    }

    Node Normalized() const{
        return {x / dis(), y / dis()};
    }
};

struct Link {
    Node Sender, Receiver;

    Link(Node s, Node r) : Sender(s), Receiver(r) {}

    double getDis() const {
        return (Sender - Receiver).dis();
    }

};

struct Gen{
    default_random_engine e;

    Gen(){
        e.seed(time(nullptr));
    }

    Node genSender(double minV, double maxV){
        uniform_real_distribution<double> u(minV, maxV);
        return {u(e), u(e)};
    }

    Link genLink(Node Sender, double minLength, double maxLength){
        uniform_real_distribution<double> lu(minLength,maxLength);
        uniform_real_distribution<double> pu(-1,1);
        return {Sender, Sender + Node(pu(e), pu(e)).Normalized() * lu(e)};
    }
};


struct Network{

};

int main() {


    return 0;
}