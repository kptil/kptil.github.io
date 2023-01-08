#include <iostream>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm> //for c++ sort
#include <iomanip> //for setprecision
using namespace std;        

//FIXME: Expand documentation using previous written documentation doc and make into doxygen style
// The edge class allows storage and update of int, float pairs
struct Edge {
    int from;
    float weight;
    Edge(int _from, float _weight) : from(_from), weight(_weight) {}
};

class Graph {
private:
    unordered_map<string, int> vertices;
    unordered_map<int, vector<Edge>> adjList; //key is "to" vertex, value is vector of (from, wt) edges
    unordered_map<int, int> outdegrees; 
    
public:
    void PageRank(int p); 
    void insertEdge(string& from, string& to);
    void updateWeights(); 
};

void Graph::insertEdge(string& from, string& to) {
    // assign URLs to indices if not yet indexed
    vertices.insert({from, vertices.size()}); 
    vertices.insert({to, vertices.size()});
    
    // create and add edge with default weight, updating vertex outdegree
    Edge edge(vertices[from], 1.0);
    outdegrees[vertices[from]]++;
    adjList[vertices[to]].push_back(edge);
    adjList[vertices[from]];
}

// assigns weights based on "from" vertex outdegree
void Graph::updateWeights() {
    for (auto it = adjList.begin(); it != adjList.end(); it++) {
        for (int i = 0; i < (it->second).size(); i++) {
            (it->second)[i].weight = 1.0 / (float) outdegrees[(it->second)[i].from];
        }
    }
}

void Graph::PageRank(int p) {
    //set r_0 to a vector of length numVertices with all entries initialized 1/numVertices
    int numVertices = vertices.size();
    float init = 1.0 / (float) numVertices;
    vector<float> r0(numVertices, init);
    
    //Calculate r_p-1. Matrix multiplication is conducted p-1 times, and each time the result becomes the new r0 for the next iteration
    for (int i = 1; i < p; i++) {
        vector<float> r1(numVertices, 0);
        for (auto it = adjList.begin(); it != adjList.end(); it++) {
            float sum = 0;
            for (int i = 0; i < (it->second).size(); i++) {
                sum += r0[(it->second)[i].from]*(it->second)[i].weight;
            }
            r1[it->first] = sum;
        }
        r0 = r1;
    }
    
    //Sorts URL strings in alphanumeric order using C++ sort and prints.
    vector<string> links;
    for (auto it = vertices.begin(); it != vertices.end(); it++) {
        links.push_back(it->first);
    }
    sort(links.begin(), links.end());
    cout << fixed << showpoint;
    cout << setprecision(2);
    for (int i = 0; i < links.size(); i++) {
        cout << links[i] << " " << r0[vertices[links[i]]] << endl;
    }
} 
//FIXME: add test cases with cassert, not input; Use numbered tests as input
int main()
{
    Graph graph;
    int no_of_lines, power_iterations; 
    string from, to; 
    cin >> no_of_lines; 
    cin >> power_iterations; 
    for(int i=0; i< no_of_lines; i++) { 
      cin >> from; 
      cin >> to; 
      graph.insertEdge(from, to);
    } 
    graph.updateWeights();
    graph.PageRank(power_iterations); 
    return 0;
}