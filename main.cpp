// main.cpp
// Simple Library Management Web Server using cpp-httplib + nlohmann::json
// Compile: g++ main.cpp -o server -pthread -std=c++17
// Place httplib.h and json.hpp in the same folder (download from their repos).

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <mutex>

#include "httplib.h"       // httplib single header (place in project folder)
#include "json.hpp"        // nlohmann::json single header (place in project folder)

using json = nlohmann::json;
namespace fs = std::filesystem;

const std::string DATA_FILE = "books.json";
const std::string WWW_DIR = "www";

std::mutex data_mutex;

json load_books() {
    std::lock_guard<std::mutex> lock(data_mutex);
    std::ifstream in(DATA_FILE);
    if (!in.is_open()) return json::array();
    json arr;
    try {
        in >> arr;
        if (!arr.is_array()) return json::array();
        return arr;
    } catch (...) {
        return json::array();
    }
}

void save_books(const json &arr) {
    std::lock_guard<std::mutex> lock(data_mutex);
    std::ofstream out(DATA_FILE);
    out << arr.dump(2);
    out.close();
}

std::string read_file_string(const fs::path &p) {
    std::ifstream in(p, std::ios::in | std::ios::binary);
    if (!in) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::string content_type_from_ext(const std::string &ext) {
    if (ext == "html") return "text/html; charset=utf-8";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "json") return "application/json";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "svg") return "image/svg+xml";
    return "text/plain";
}

int main() {
    // Ensure data file exists
    if (!fs::exists(DATA_FILE)) {
        json initial = json::array();
        initial.push_back({ {"id",101}, {"title","Data Structures in C++"}, {"author","John Smith"}, {"issued", false} });
        initial.push_back({ {"id",102}, {"title","Algorithms Unlocked"}, {"author","Jane Doe"}, {"issued", true} });
        save_books(initial);
    }

    httplib::Server svr;

    // REST: GET /api/books
    svr.Get("/api/books", [&](const httplib::Request&, httplib::Response& res){
        json arr = load_books();
        res.set_content(arr.dump(2), "application/json");
    });

    // REST: POST /api/books  (body: json with id,title,author)
    svr.Post("/api/books", [&](const httplib::Request& req, httplib::Response& res){
        try {
            json body = json::parse(req.body);
            if (!body.contains("id") || !body.contains("title") || !body.contains("author")) {
                res.status = 400;
                res.set_content(R"({"error":"id,title,author required"})", "application/json");
                return;
            }
            json arr = load_books();
            int id = body["id"].get<int>();
            // prevent duplicates
            for (auto &b : arr) if (b["id"].get<int>() == id) {
                res.status = 409;
                res.set_content(R"({"error":"book id already exists"})", "application/json");
                return;
            }
            json book = {
                {"id", id},
                {"title", body["title"].get<std::string>()},
                {"author", body["author"].get<std::string>()},
                {"issued", false}
            };
            arr.push_back(book);
            save_books(arr);
            res.status = 201;
            res.set_content(book.dump(2), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error":"invalid json"})", "application/json");
        }
    });

    // REST: PUT /api/books/{id}/issue and /api/books/{id}/return
    svr.Put(R"(^/api/books/(\d+)/(issue|return)$)", [&](const httplib::Request& req, httplib::Response& res){
        std::smatch m;
        std::regex r(R"(^/api/books/(\d+)/(issue|return)$)");
        std::string path = req.path;
        if (std::regex_match(path, m, r)) {
            int id = std::stoi(m[1].str());
            std::string action = m[2].str();
            json arr = load_books();
            bool found = false;
            for (auto &b : arr) {
                if (b["id"].get<int>() == id) {
                    found = true;
                    if (action == "issue") {
                        if (b["issued"].get<bool>()){
                            res.status = 409;
                            res.set_content(R"({"error":"book already issued"})","application/json");
                            return;
                        }
                        b["issued"] = true;
                    } else {
                        b["issued"] = false;
                    }
                    save_books(arr);
                    res.set_content(b.dump(2), "application/json");
                    return;
                }
            }
            if (!found) {
                res.status = 404;
                res.set_content(R"({"error":"book not found"})", "application/json");
                return;
            }
        } else {
            res.status = 400;
            res.set_content(R"({"error":"bad request"})", "application/json");
        }
    });

    // REST: DELETE /api/books/{id}
    svr.Delete(R"(^/api/books/(\d+)$)", [&](const httplib::Request& req, httplib::Response& res){
        std::smatch m;
        std::regex r(R"(^/api/books/(\d+)$)");
        if (std::regex_match(req.path, m, r)) {
            int id = std::stoi(m[1].str());
            json arr = load_books();
            json newarr = json::array();
            bool removed = false;
            for (auto &b : arr) {
                if (b["id"].get<int>() == id) {
                    removed = true;
                    continue;
                }
                newarr.push_back(b);
            }
            if (!removed) {
                res.status = 404;
                res.set_content(R"({"error":"book not found"})", "application/json");
                return;
            }
            save_books(newarr);
            res.set_content(R"({"ok":true})", "application/json");
        } else {
            res.status = 400;
            res.set_content(R"({"error":"bad request"})", "application/json");
        }
    });

    // Static file serving: everything else from WWW_DIR
    svr.Get(R"(/(.*))", [&](const httplib::Request& req, httplib::Response& res){
        std::string path = req.path;
        if (path == "/" || path.empty()) path = "/index.html";
        fs::path f = fs::path(WWW_DIR) / path.substr(1);
        if (!fs::exists(f) || fs::is_directory(f)) {
            res.status = 404;
            res.set_content("Not Found", "text/plain");
            return;
        }
        std::string ext = f.extension().string();
        if (!ext.empty() && ext[0]=='.') ext = ext.substr(1);
        std::string ct = content_type_from_ext(ext);
        auto content = read_file_string(f);
        res.set_content(content, ct);
    });

    std::cout << "Starting server at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}
