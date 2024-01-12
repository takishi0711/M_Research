#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;




double calc_dcg(const vector<int>& approx_ranking, const unordered_map<int, double>& exact_ppr) {
    int k = approx_ranking.size();
    double dcg = 0;
    for (int i = 0; i < k; i++) {
        double ppr_val;
        int approx_node_id = approx_ranking[i];
        auto itr = exact_ppr.find(approx_node_id);
        if (itr != exact_ppr.end()) ppr_val = itr -> second;
        else ppr_val = 0;
        dcg += (pow(2, ppr_val) - 1) / log2(i + 2);
    }
    return dcg;
}

double calc_perfect_dcg(const unordered_map<int, double>& exact_ppr, int k) {
    vector<pair<int, double>> ppr_vec;
    for(auto itr = exact_ppr.begin(); itr != exact_ppr.end(); ++itr) {
        ppr_vec.push_back(make_pair(itr->first, itr->second));
    }
    sort(ppr_vec.begin(), ppr_vec.end(),
        [](const pair<int, double> &l, const pair<int, double> &r)
        {
            if (l.second != r.second) {
                return l.second > r.second;
            }
            return l.first < r.first;
        });
    assert(k <= ppr_vec.size());
    ppr_vec.erase(ppr_vec.begin() + k, ppr_vec.end());
    
    double perfect_dcg = 0;
    for (int i = 0; i < ppr_vec.size(); i++) {
        perfect_dcg += (pow(2, ppr_vec[i].second) - 1) / log2(i + 2);
    }

    return perfect_dcg;
}

vector<pair<int, double>> get_ordered_ppr(unordered_map<int, double> ppr) {
    vector<pair<int, double>> ppr_vec;
    for(auto itr = ppr.begin(); itr != ppr.end(); ++itr) {
        ppr_vec.push_back({itr->first, itr->second});
    }
    sort(ppr_vec.begin(), ppr_vec.end(),
        [](const pair<int, double> &l, const pair<int, double> &r)
        {
            if (l.second != r.second) {
                return l.second > r.second;
            }
            return l.first < r.first;
        });
    
    return ppr_vec;
}


// exact_ppr, approx_ppr : key is node_id, value is ppr
double calc_ndcg(unordered_map<int, double> exact_ppr, unordered_map<int, double> approx_ppr, int k=128) {
    // node　−１ indicates the supernode to which the random walk that reached the dangling node will move next.
    if (exact_ppr.count(-1) == 1) exact_ppr.erase(-1);
    if (approx_ppr.count(-1) == 1) approx_ppr.erase(-1);
    vector<pair<int, double>> ordered_approx_ppr = get_ordered_ppr(approx_ppr);
    vector<int> approx_ranking;
    for (int i = 0; (i < k) && (i < ordered_approx_ppr.size()); i++) approx_ranking.push_back(ordered_approx_ppr[i].first);
    k = min({k, (int)exact_ppr.size()});
    // fill approx_ranking with meaningless nodes (-1) until the size becomes k.
    // note that exact_ppr[-1] == 0
    while (approx_ranking.size() < k) {
        approx_ranking.push_back(-1);
    }
    return calc_dcg(approx_ranking, exact_ppr) / calc_perfect_dcg(exact_ppr, k);
}