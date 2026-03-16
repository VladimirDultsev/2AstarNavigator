#include <iostream>
#include <unordered_set>
#include <map>
#include <tuple>
#include <vector>
#include <fstream>
#include <utility>
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include "rapidjson/include/rapidjson/istreamwrapper.h"

/// Структура хеширования пар
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

using namespace std;
map<double, unordered_set<unsigned long long int>> dict[2];
unordered_map<unsigned long long int, tuple<double, double, unsigned long long int, bool>>pars[2];//эвристика, minDist (чистое расстояние без эвристики), родитель
unordered_map<unsigned long long int, pair<double, double>> points;
unordered_map<pair<double, double>, int, pair_hash> Indices;
unordered_map<unsigned long long int, unordered_set<unsigned long long int>> adjList;
unsigned long long int start, finish;
unordered_map<pair<double, double>, unordered_set<pair<double, double>, pair_hash>, pair_hash> adjListCoords;
vector<pair<double, double>> pts;

#define PI 3.14159265358979323846
#define RADIO_TERRESTRE 6372797.56085
#define GRADOS_RADIANES PI / 180
#define RADIANES_GRADOS 180 / PI
/// Метод подсчёта расстояния между двумя точками
double calcGPSDistance(double longitude_new, double latitude_new, double longitude_old, double latitude_old){
    double  lat_new = latitude_old * GRADOS_RADIANES;
    double  lat_old = latitude_new * GRADOS_RADIANES;
    double  lat_diff = (latitude_new-latitude_old) * GRADOS_RADIANES;
    double  lng_diff = (longitude_new-longitude_old) * GRADOS_RADIANES;
    double  a = sin(lat_diff/2) * sin(lat_diff/2) +
                cos(lat_new) * cos(lat_old) *
                sin(lng_diff/2) * sin(lng_diff/2);
    double  c = 2 * atan2(sqrt(a), sqrt(1-a));
    double  distance = RADIO_TERRESTRE * c;
    return abs(distance);
}
/// Эвристика
double Heuristic(unsigned long long int point, int AStarIndex){
    if(AStarIndex){// Если нас вызывает обратный A*
        return calcGPSDistance(points[start].first, points[start].second, points[point].first, points[point].second);
    }else{
        return calcGPSDistance(points[finish].first, points[finish].second, points[point].first, points[point].second);
    }
}
/// 2A*
vector<unsigned long long int> DoubleAStar()
{
    int AStarIndex = 0;
    unsigned long long int point = 0;
    double finalDist, oldDist, heuristic, newNeighbourDist;
    unordered_set<unsigned long long int> EmptySet;
    pars[0].clear();
    pars[1].clear();
    dict[0].clear();
    dict[1].clear();
    pars[0][start] = make_tuple(Heuristic(start, 0), 0, start, false);
    dict[0][get<0>(pars[0][start])].insert(start);// Добавляем старт в словарь прямого A*-а под приоритетом в виде только эвристики, потому что расстояние от старта до старта 0
    pars[1][finish] = make_tuple(Heuristic(finish, 1), 0, finish, false);
    dict[1][get<0>(pars[1][finish]) + get<1>(pars[1][finish])].insert(finish);// Добавляем финиш в словарь обратного A*-а под приоритетом в виде только эвристики, потому что расстояние от финиша до финиша 0
    while (!dict[0].empty() || !dict[1].empty())// Алгоритм работает пока хоть в одном из словарей есть точки
    {
        if (!dict[0].empty() && !dict[1].empty()) {// Если в обоих словарях есть точки
            AStarIndex = (dict[0].begin()->first < dict[1].begin()->first) ? 0 : 1;// Берем точку из того словаря, где приоритет меньше
            point = *(dict[AStarIndex].begin()->second.begin());
        }
        else if (!dict[0].empty()) {// Если словарь обратного A*-а пустой
            AStarIndex = 0;// Берем точку из словаря прямого A*-а
            point = *(dict[0].begin()->second.begin());
        }
        else {// Если словарь прямого A*-а пустой
            AStarIndex = 1;// Берем точку из словаря обратного A*-а
            point = *(dict[1].begin()->second.begin());
        }
        if (pars[1 - AStarIndex].count(point) && get<3>(pars[1 - AStarIndex][point])) {// Если точка уже удалена из другого словаря, значит A*-ы соединились
            break;// Прекращаем работу
        }
        for (unsigned long long int neighbour : adjList[point])// Перебираем соседей точки
        {
            newNeighbourDist = get<1>(pars[AStarIndex][point]) + // Расстояние, за которое мы дошли до этого соседа (сумма расстояния за которое мы дошли до нас + расстояние до соседа)
                               calcGPSDistance(points[point].first, points[point].second, points[neighbour].first, points[neighbour].second);
            if (!pars[AStarIndex].count(neighbour))// Если до этого мы не встречали нашего соседа
            {
                heuristic = Heuristic(neighbour, AStarIndex);// Считаем эвристику до него
                pars[AStarIndex][neighbour] = make_tuple(heuristic, newNeighbourDist, point, false);
                finalDist = newNeighbourDist + heuristic;// В словарь кладем точку под приоритетом в виде суммы расстояния до соседа и эвристики
                dict[AStarIndex][finalDist].insert(neighbour);
            }
            else if (newNeighbourDist < get<1>(pars[AStarIndex][neighbour]))// Если найден более короткий путь до соседа
            {
                oldDist = get<0>(pars[AStarIndex][neighbour]) + get<1>(pars[AStarIndex][neighbour]);// Считаем приоритет, под которым сосед был положен в словарь раньше
                dict[AStarIndex][oldDist].erase(neighbour);// Удаляем соседа из словаря
                if (dict[AStarIndex][oldDist].empty()) {// Если в словаре больше нет точек с таким приоритетом
                    dict[AStarIndex].erase(oldDist);// Удаляем этот ключ из словаря
                }
                get<1>(pars[AStarIndex][neighbour]) = newNeighbourDist;// Обновляем расстояние до соседа
                get<2>(pars[AStarIndex][neighbour]) = point;// Теперь мы - родитель этого соседа
                finalDist = newNeighbourDist + get<0>(pars[AStarIndex][neighbour]);// В словарь кладем точку под приоритетом в виде суммы расстояния до соседа и эвристики
                dict[AStarIndex][finalDist].insert(neighbour);
            }
        }
        get<3>(pars[AStarIndex][point]) = true;// Отмечаем нашу точку удаленной
        oldDist = get<0>(pars[AStarIndex][point]) + get<1>(pars[AStarIndex][point]);// Считаем приоритет, под которым наша точка лежала в словаре
        dict[AStarIndex][oldDist].erase(point);// Удаляем точку из словаря
        if (dict[AStarIndex][oldDist].empty()) {// Если в словаре больше нет точек с таким приоритетом
            dict[AStarIndex].erase(oldDist);// Удаляем этот ключ из словаря
        }
    }
    vector<unsigned long long int> path;
    for(unsigned long long int pt = point; pt != start; pt = get<2>(pars[0][pt])){// Прыгаем по соседям из pars прямого A*-а пока не дойдем до старта
        path.push_back(pt);
    }
    path.push_back(start);// Добавляем старт в путь
    reverse(path.begin(), path.end());// Переворачиваем путь, потому что мы шли в обратном порядке
    for(unsigned long long int pt = get<2>(pars[1][point]); pt != finish; pt = get<2>(pars[1][pt])){// Прыгаем по соседям из pars обратного A*-а пока не дойдем до старта
        path.push_back(pt);
    }
    path.push_back(finish);// Добавляем финиш в путь
    return path;
}
using namespace rapidjson;
/// Строим список смежности по geojson файлу
void buildGraph(Document& data, unordered_map<pair<double, double>, unordered_set<pair<double, double>, pair_hash>, pair_hash>& adj) {
//    if (!data.HasMember("features") || !data["features"].IsArray()) {
//        std::cerr << "Invalid JSON structure: missing features array" << std::endl;
//        return;
//    }
    const Value& features = data["features"];
    for (unsigned long long int i = 0; i < features.Size(); ++i) {
        const Value& feature = features[i];
        if (!feature.IsObject() ||
            !feature.HasMember("geometry") ||
            !feature["geometry"].IsObject() ||
            !feature["geometry"].HasMember("coordinates") ||
            !feature["geometry"]["coordinates"].IsArray()) {
            continue;
        }
        const Value& coordinates = feature["geometry"]["coordinates"];
        for (SizeType y = 1; y < coordinates.Size(); ++y) {
            const Value& point = coordinates[y];
            const Value& lastPoint = coordinates[y - 1];
            if (!point.IsArray() || point.Size() < 2 ||
                !point[0].IsNumber() || !point[1].IsNumber()) {
                continue;
            }
            adj[make_pair(point[0].GetDouble(), point[1].GetDouble())].insert(make_pair(lastPoint[0].GetDouble(), lastPoint[1].GetDouble()));
            adj[make_pair(lastPoint[0].GetDouble(), lastPoint[1].GetDouble())].insert(make_pair(point[0].GetDouble(), point[1].GetDouble()));
        }
    }
}
/// Парсим массив номеров точек в geojson
string pathToJson(vector<unsigned long long int>& path) {
    Document jsonPath(kObjectType);
    auto& allocator = jsonPath.GetAllocator();
    jsonPath.AddMember("type", "FeatureCollection", allocator);
    Value features(kArrayType);
    Value feature(kObjectType);
    feature.AddMember("type", "Feature", allocator);
    Value properties(kObjectType);
    properties.AddMember("@id", "way/33803251", allocator);
    properties.AddMember("highway", "residential", allocator);
    properties.AddMember("name", "X Corps Boulevard", allocator);
    feature.AddMember("properties", properties, allocator);
    Value geometry(kObjectType);
    Value coordinates(kArrayType);
    for (auto& point : path) {
        Value pointCoords(kArrayType);
        pointCoords.PushBack(points[point].first, allocator);
        pointCoords.PushBack(points[point].second, allocator);
        coordinates.PushBack(pointCoords, allocator);
    }
    geometry.AddMember("type", "LineString", allocator);
    geometry.AddMember("coordinates", coordinates, allocator);
    feature.AddMember("geometry", geometry, allocator);
    features.PushBack(feature, allocator);
    jsonPath.AddMember("features", features, allocator);
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    jsonPath.Accept(writer);
    return buffer.GetString();
}
/// Задаём номера точкам
void codePoints(){
    unsigned long long int cnt = 0;
    for(auto pr: adjListCoords){
        points[cnt] = pr.first;
        pts.push_back(pr.first);
        Indices[pr.first] = cnt;
        ++cnt;
    }
    for(auto pr: adjListCoords){
        for(auto pr1: pr.second){
            adjList[Indices[pr.first]].insert(Indices[pr1]);
        }
    }
}
/// Метод поиска ближайшей точки графа
unsigned long long int findNearestPoint(double latitude, double longitude){
    double minDist = numeric_limits<double>::max();
    unsigned long long int nearestPoint;
    for(auto point: points){
        if(calcGPSDistance(latitude, longitude, point.second.second, point.second.first) < minDist){
            minDist = calcGPSDistance(latitude, longitude, point.second.second, point.second.first);
            nearestPoint = point.first;
        }
    }
    return nearestPoint;
}
int main(int argc, char* argv[])
{
    string outputPath = argv[1], graphPath = argv[2];
    ifstream file(graphPath);
    IStreamWrapper isw(file);
    Document doc;
    doc.ParseStream(isw);
    buildGraph(doc, adjListCoords);
    codePoints();
    double lat1, lon1, lat2, lon2;
    vector<unsigned long long int> path;

    while(true){
        cin >> lat1 >> lon1 >> lat2 >> lon2;
        start = findNearestPoint(lat1, lon1);
        finish = findNearestPoint(lat2, lon2);
        path = DoubleAStar();
        ofstream File(outputPath);
        File << pathToJson(path);
        File.close();
        cout << " \n";
    }
    return 0;
}
