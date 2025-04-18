
#include "JSONParser.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include "TimeCounter.h"

#pragma warning(disable : 4996)

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
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " [input_json_file] [input_control_bin_file]";
        return 1;
    }

    std::cout << "OS time frequency: " << GetOSTimerFreq() << "; Read OS timer: " << ReadOSTimer() << "; ReadCPUTimer: " << ReadCPUTimer()
              << "; Approximated CPU timer freq: " << ApproximateCPUTimerFreq() << std::endl;

    std::ifstream input(argv[2], std::ios::binary);
    input.seekg(0, std::ios::end);
    size_t filesize = input.tellg();
    input.seekg(0, std::ios::beg);
    std::vector<double> bin_file_buffer;
    bin_file_buffer.resize(filesize / sizeof(double));
    input.read(reinterpret_cast<char*>(bin_file_buffer.data()), filesize);
    FILE* json_file = fopen(argv[1], "r");
    if (json_file == nullptr) {
        std::cout << "File error: " << stderr;
        return 1;
    }

    fseek(json_file, 0, SEEK_END);
    int size = ftell(json_file);
    rewind(json_file);

    char* buffer = new char[size];
    if (buffer == nullptr) {
        std::cout << "Memory error";
        return 1;
    }

    int copy_result = fread(buffer, 1, size, json_file);
    if (copy_result + 1 != size) {
        std::cout << "Read error";
        return 1;
    }

    JSONParser parser;
    parser.Parse(buffer);
    JSONValue* root = parser.token_stack.top().parent_value;
    auto pairs_value = (*root->obj_)["pairs"];
    double average = 0;
    for (auto element : *pairs_value->array_) {
        auto x0 = (*element->obj_)["x0"]->number_;
        auto y0 = (*element->obj_)["y0"]->number_;
        auto x1 = (*element->obj_)["x1"]->number_;
        auto y1 = (*element->obj_)["y1"]->number_;
        auto haversine_distance = GetHaversineDistance(x0, y0, x1, y1, EarthRadius) / static_cast<double>(pairs_value->array_->size());
        average += haversine_distance;
    }

    std::cout << std::setprecision(15) << "Expected average: " << bin_file_buffer.back() << "; Calculated average: " << average;

    fclose(json_file);
    delete[] buffer;
    return 0;
}
