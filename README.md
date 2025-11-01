# Library Management - C++ Web App

This is a minimal C++ web application (backend) + static frontend (HTML/JS/CSS)
for a Library Management System. The server is implemented using the single-header
libraries **cpp-httplib** and **nlohmann::json** (download links below).

## Files in this package
- main.cpp            : C++ server (uses httplib.h and json.hpp)
- books.json          : initial data store (JSON array)
- www/                : static frontend files (index.html, catalog.html, admin.html, styles.css, script.js)

## Required headers (download and place in project root)
- httplib.h  : https://github.com/yhirose/cpp-httplib (single_include/httplib/httplib.h)
- json.hpp   : https://github.com/nlohmann/json (single_include/nlohmann/json.hpp)

## Build & Run
1. Make sure you have g++ (supporting C++17) and pthreads.
2. Place `httplib.h` and `json.hpp` in the same folder as `main.cpp`.
3. Compile:
   ```bash
   g++ main.cpp -o server -pthread -std=c++17
   ```
4. Run:
   ```bash
   ./server
   ```
5. Open in your browser: http://localhost:8080/

## Notes
- This is an educational demo. For production, use robust frameworks, add authentication,
  input validation, and persistent DB like SQLite/MySQL.
