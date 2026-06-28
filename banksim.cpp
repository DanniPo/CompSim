// Single-server FCFS, uniform inter-arrival U(a1,b1)
// and uniform service U(a2,b2). All times in minutes.
//
// Compile : g++ -std=c++17 -O2 -o banksim banksim.cpp
// Run     : ./banksim     (then answer the prompts)

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <limits>
#include <string>
using namespace std;

// ─── Data structure for one customer ──────────────────
struct Customer {
    int    id;
    double interarrival;   // T_{A,i} ~ U(a1,b1)
    double arrival;        // A_i = A_{i-1} + T_{A,i}
    double service_time;   // T_{S,i} ~ U(a2,b2)
    double service_start;  // SS_i = max(A_i, F_{i-1})
    double wait_queue;     // W_{q,i} = SS_i - A_i
    double service_end;    // F_i = SS_i + T_{S,i}
    double sojourn;        // W_i = F_i - A_i
};

// ─── Sample X ~ U(a, b) using inverse-transform ───────
// Equation: X = a + U(0,1) * (b - a)
double uniform_sample(mt19937& rng, double a, double b) {
    uniform_real_distribution<double> dist(a, b);
    return dist(rng);
}

// ─── Core simulation loop ──────────────────────────────
vector<Customer> simulate(int n,
                           double a1, double b1,   // inter-arrival bounds
                           double a2, double b2,   // service time bounds
                           unsigned seed)
{
    mt19937 rng(seed);
    vector<Customer> customers(n);

    double arrival_clock = 0.0;   // running arrival time
    double server_free   = 0.0;   // F_{i-1}, starts at 0

    for (int i = 0; i < n; ++i) {

        // Step 1: sample inter-arrival time, T_{A,i} ~ U(a1, b1)
        customers[i].interarrival = uniform_sample(rng, a1, b1);

        // Step 2: arrival time, A_i = A_{i-1} + T_{A,i}
        arrival_clock += customers[i].interarrival;
        customers[i].arrival = arrival_clock;

        // Step 3: sample service time, T_{S,i} ~ U(a2, b2)
        customers[i].service_time = uniform_sample(rng, a2, b2);

        // Step 4: service start, SS_i = max(A_i, F_{i-1})
        customers[i].service_start = max(arrival_clock, server_free);

        // Step 5: queue wait, W_{q,i} = SS_i - A_i  (>= 0 always)
        customers[i].wait_queue = customers[i].service_start - arrival_clock;

        // Step 6: service end, F_i = SS_i + T_{S,i}
        customers[i].service_end = customers[i].service_start + customers[i].service_time;

        // Step 7: sojourn time, W_i = F_i - A_i  (= W_{q,i} + T_{S,i})
        customers[i].sojourn = customers[i].service_end - arrival_clock;

        // Advance server free time for the next customer
        server_free = customers[i].service_end;
        customers[i].id = i + 1;
    }
    return customers;
}

// ─── Compute and print aggregate statistics ───────────
void print_stats(const vector<Customer>& c) {
    int n = (int)c.size();

    double sum_wq = 0, sum_ts = 0, sum_w = 0;
    for (const auto& x : c) {
        sum_wq += x.wait_queue;
        sum_ts += x.service_time;
        sum_w  += x.sojourn;
    }

    double avg_wait    = sum_wq / n;                          // W_bar_q
    double avg_service = sum_ts / n;                          // S_bar
    double avg_sojourn = sum_w  / n;                          // W_bar
    double utilisation = sum_ts / c.back().service_end;       // rho

    cout << fixed << setprecision(4);
    cout << "\n=== Queue Statistics ===\n";
    cout << "Avg queue wait    (Wq) : " << avg_wait    << " min\n";
    cout << "Avg service time  (S)  : " << avg_service << " min\n";
    cout << "Avg sojourn time  (W)  : " << avg_sojourn << " min\n";
    cout << "Server utilisation(rho): " << utilisation * 100 << " %\n";
    cout << "Max queue wait observed: "
         << max_element(c.begin(), c.end(),
                [](const Customer& a, const Customer& b){ return a.wait_queue < b.wait_queue; })->wait_queue
         << " min (customer #"
         << max_element(c.begin(), c.end(),
                [](const Customer& a, const Customer& b){ return a.wait_queue < b.wait_queue; })->id
         << ")\n";
}

// ─── Main: interactive input / output window ──────────
int main() {
    double a1, b1, a2, b2;
    int    n;
    unsigned seed;

    cout << "=== Bank Queue Simulator ===\n";
    cout << "Inter-arrival  lower bound a1 (e.g. 1): "; cin >> a1;
    cout << "Inter-arrival  upper bound b1 (e.g. 8): "; cin >> b1;
    cout << "Service time   lower bound a2 (e.g. 1): "; cin >> a2;
    cout << "Service time   upper bound b2 (e.g. 6): "; cin >> b2;
    cout << "Number of customers (n)        : "; cin >> n;
    cout << "Random seed (0 = random)       : "; cin >> seed;

    if (a1 >= b1 || a2 >= b2 || n <= 0) {
        cerr << "Error: lower bound must be < upper bound, and n must be > 0.\n";
        return 1;
    }
    if (seed == 0) seed = random_device{}();

    auto customers = simulate(n, a1, b1, a2, b2, seed);

    // ─── Output table ────────────────────────────────
    cout << "\n" << setw(4)  << "#"
         << setw(10) << "Interarr"
         << setw(10) << "Arrival"
         << setw(10) << "SvcTime"
         << setw(10) << "SvcStart"
         << setw(10) << "WaitQ"
         << setw(10) << "SvcEnd"
         << setw(10) << "Sojourn"
         << "\n";
    cout << string(74, '-') << "\n";

    cout << fixed << setprecision(3);
    for (const auto& x : customers) {
        cout << setw(4)  << x.id
             << setw(10) << x.interarrival
             << setw(10) << x.arrival
             << setw(10) << x.service_time
             << setw(10) << x.service_start
             << setw(10) << x.wait_queue
             << setw(10) << x.service_end
             << setw(10) << x.sojourn
             << "\n";
    }

    print_stats(customers);

    cout << "\nPress Enter to exit...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
    return 0;
}
