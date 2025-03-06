
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma warning(disable : 4996)

static bool is_uniform = false;
static int random_seed = 0;
static int coords_num = 0;

static int clusters_count = 5;
struct Cluster {
    float begin;
    float end;
};
std::vector<Cluster> clusters;

float GetRandomCoordinate(float base) {
    if (!clusters.empty()) {
        int cluster_idx = std::rand() % clusters_count;
        return (clusters[cluster_idx].begin + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.2f) * base;
    }
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * base * 2.0f - base;
}

static double Square(double A) {
    double Result = (A * A);
    return Result;
}

static double RadiansFromDegrees(double degrees) {
    double result = 0.01745329251994329577f * degrees;
    return result;
}

double GetHaversineDistance(double x0, double y0, double x1, double y1, double earth_radius) {
    double lat1 = y0;
    double lat2 = y1;
    double lon1 = x0;
    double lon2 = x1;

    double dLat = RadiansFromDegrees(lat2 - lat1);
    double dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);

    double a = Square(sin(dLat / 2.0)) + cos(lat1) * cos(lat2) * Square(sin(dLon / 2));
    double c = 2.0 * asin(sqrt(a));

    double Result = earth_radius * c;

    return Result;
}

static const double EarthRadius = 6372.8;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage " << argv[0] << " [uniform/cluster] [random seed] [number of coordinate pairs to generate]";
        return 1;
    }
    is_uniform = (strcmp(argv[1], "uniform") == 0);
    random_seed = std::stoi(argv[2]);
    coords_num = std::stoi(argv[3]);

    std::cout << "is_uniform: " << is_uniform << " random seed: " << random_seed << " coords num: " << coords_num << std::endl;
    std::srand(random_seed);
    if (!is_uniform) {
        for (int i = 0; i < clusters_count; i++) {
            Cluster cluster;
            cluster.begin = std::rand() / static_cast<float>(RAND_MAX) * 1.8f - 1.0f;
            cluster.end = cluster.begin + 0.2f;
            clusters.push_back(cluster);
        }
    }
    std::fstream file_stream("out.bin", std::ios::out | std::ios::binary | std::ios::trunc);
    double sum = 0.0f;
    char* buffer = new char[coords_num * 110];
    char* cur_pointer = buffer;
    auto json_start_str = "{\"pairs\":[\n";
    memcpy(cur_pointer, json_start_str, strlen(json_start_str));
    cur_pointer += strlen(json_start_str);
    struct CoordComponentData {
        const char* str;
        size_t size;
        float base;
        float coord = 0.0f;
    };
    std::vector<CoordComponentData> components_data;
    auto x0_str = "{\"x0\":";
    components_data.push_back({x0_str, strlen(x0_str), 180.0f});
    auto y0_str = ", \"y0\":";
    components_data.push_back({y0_str, strlen(y0_str), 90.0f});
    auto x1_str = ", \"x1\":";
    components_data.push_back({x1_str, strlen(x1_str), 180.0f});
    auto y1_str = ", \"y1\":";
    components_data.push_back({y1_str, strlen(y1_str), 90.0f});

    char temp_buf[20];
    for (int i = 0; i < coords_num; i++) {
        for (auto& data : components_data) {
            memcpy(cur_pointer, data.str, data.size);
            cur_pointer += data.size;
            data.coord = GetRandomCoordinate(data.base);
            cur_pointer += sprintf(cur_pointer, "%.15f", data.coord);
            *cur_pointer = ' ';
        }
        *cur_pointer++ = '}';

        double dist = GetHaversineDistance(components_data[0].coord, components_data[1].coord, components_data[2].coord, components_data[3].coord, EarthRadius);
        file_stream.write((char*)&dist, sizeof(dist));
        sum += dist;
        if (i < coords_num - 1) {
            *cur_pointer++ = ',';
        }
    }

    auto json_end_str = "]}";
    memcpy(cur_pointer, json_end_str, strlen(json_end_str));
    cur_pointer += strlen(json_end_str);
    *cur_pointer = '\0';
    auto file = fopen("point_pairs.json", "w");
    fputs(buffer, file);
    fclose(file);
    file_stream.close();
    std::cout << std::setprecision(15) << "Expected average: " << sum / coords_num << std::endl;
    delete[] buffer;

    return 0;
}
